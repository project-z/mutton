/*
  Copyright (c) 2013 Matthew Stump

  This file is part of libmutton.

  libmutton is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  libmutton is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <vector>
#include "context.hpp"
#include "encode.hpp"
#include "index.hpp"
#include "index_slice.hpp"
#include "index_reader_writer_leveldb.hpp"

mtn::index_reader_writer_leveldb_t::index_reader_writer_leveldb_t() :
    _db(NULL),
    _read_options(),
    _write_options()
{}

mtn::index_reader_writer_leveldb_t::~index_reader_writer_leveldb_t()
{
    if (_db) {
        delete _db;
    }
}

mtn::status_t
mtn::index_reader_writer_leveldb_t::init(mtn::context_t& context)
{
    std::string path;
    if (!context.get_opt(MTN_OPT_DB_PATH, path)) {
        return mtn::status_t(MTN_ERROR_BAD_CONFIGURATION, "no database path was specified");
    }

    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, path, &_db);

    if (!status.ok()) {
        return mtn::status_t(MTN_ERROR_UNKOWN, status.ToString(), false, true);
    }

    return mtn::status_t();
}

mtn::status_t
mtn::index_reader_writer_leveldb_t::read_index(mtn_index_partition_t           partition,
                                               const std::vector<mtn::byte_t>& bucket,
                                               const std::vector<mtn::byte_t>& field,
                                               mtn::index_t**                  output)
{
    mtn::index_reader_writer_t::index_container container;
    mtn::status_t status = read_indexes(partition, bucket, field, std::vector<mtn::byte_t>(), std::vector<mtn::byte_t>(), container);

    if (status) {
        if (!container.empty()) {
            *output = container.release(container.begin()).release();
        }
        else {
            *output = new mtn::index_t(partition, bucket, field);
        }
    }

    return status;
}

mtn::status_t
mtn::index_reader_writer_leveldb_t::read_indexes(mtn_index_partition_t                        partition,
                                                 const std::vector<mtn::byte_t>&              start_bucket,
                                                 const std::vector<mtn::byte_t>&              start_field,
                                                 const std::vector<mtn::byte_t>&              end_bucket,
                                                 const std::vector<mtn::byte_t>&              end_field,
                                                 mtn::index_reader_writer_t::index_container& output)
{
    std::vector<mtn::byte_t> start_key;
    std::vector<mtn::byte_t> stop_key;
    encode_index_key(partition, &start_bucket[0], start_bucket.size(), &start_field[0], start_field.size(), 0, 0, start_key);

    if (!end_bucket.empty() && !end_field.empty()) {
        encode_index_key(partition, &end_bucket[0], end_bucket.size(), &end_field[0], end_field.size(), INDEX_ADDRESS_MAX, INDEX_ADDRESS_MAX, stop_key);
    }
    else {
        encode_index_key(partition + 1, &end_bucket[0], end_bucket.size(), &end_field[0], end_field.size(), 0, 0, stop_key);
    }

    leveldb::Slice start_slice(reinterpret_cast<char*>(&start_key[0]), start_key.size());
    leveldb::Slice stop_slice(reinterpret_cast<char*>(&stop_key[0]), stop_key.size());

    std::vector<mtn::byte_t> current_bucket;
    std::vector<mtn::byte_t> current_field;
    mtn::index_t* current_index = NULL;
    mtn::index_slice_t* current_slice = NULL;
    mtn_index_address_t current_slice_value = INDEX_ADDRESS_MAX;

    std::auto_ptr<leveldb::Iterator> iter(_db->NewIterator(_read_options));
    for (iter->Seek(start_slice);
         iter->Valid() && iter->key().compare(stop_slice) < 0;
         iter->Next())
    {
        uint16_t             temp_partition   = 0;
        byte_t*              temp_bucket      = NULL;
        uint16_t             temp_bucket_size = 0;
        byte_t*              temp_field       = NULL;
        uint16_t             temp_field_size  = 0;
        mtn_index_address_t temp_value       = 0;
        mtn_index_address_t offset           = 0;

        assert(iter->value().size() == MTN_INDEX_SEGMENT_SIZE);
        mtn::decode_index_key(
            reinterpret_cast<const mtn::byte_t*>(iter->key().data()),
            &temp_partition,
            &temp_bucket,
            &temp_bucket_size,
            &temp_field,
            &temp_field_size,
            &temp_value,
            &offset);

        if ((current_bucket.size() != temp_bucket_size
             || memcmp(&current_bucket[0], temp_bucket, temp_bucket_size) != 0)
            && (current_field.size() != temp_field_size
                || memcmp(&current_field[0], temp_field, temp_field_size) != 0))
        {
            std::vector<mtn::byte_t> key;
            key.assign(temp_field, temp_field + temp_field_size);
            current_field.assign(temp_field, temp_field + temp_field_size);
            current_index = output.insert(key,
                                          new mtn::index_t(temp_partition,
                                                           temp_bucket,
                                                           temp_bucket_size,
                                                           temp_field,
                                                           temp_field_size)
                ).first->second;

            current_slice_value = INDEX_ADDRESS_MAX;
        }

        if (current_slice_value != temp_value) {
            mtn::index_t::iterator insert_iter = current_index->find(temp_value);
            if (insert_iter != current_index->end()) {
                current_slice = insert_iter->second;
            }
            else {
                current_slice = current_index->insert(temp_value,
                                                      new mtn::index_slice_t(temp_partition,
                                                                             temp_bucket,
                                                                             temp_bucket_size,
                                                                             temp_field,
                                                                             temp_field_size,
                                                                             temp_value)).first->second;
                current_slice_value = temp_value;
            }
        }
        current_slice->insert(current_slice->end(), new mtn::index_slice_t::index_node_t(offset, (const index_segment_ptr) iter->value().data()));
    }
    return mtn::status_t(); // XXX TODO better error handling
}

mtn::status_t
mtn::index_reader_writer_leveldb_t::read_index_slice(mtn_index_partition_t           partition,
                                                     const std::vector<mtn::byte_t>& bucket,
                                                     const std::vector<mtn::byte_t>& field,
                                                     mtn_index_address_t             value,
                                                     mtn::index_slice_t&             output)
{
    std::vector<mtn::byte_t> start_key;
    std::vector<mtn::byte_t> stop_key;
    encode_index_key(partition, &bucket[0], bucket.size(), &field[0], field.size(), value, 0, start_key);
    encode_index_key(partition, &bucket[0], bucket.size(), &field[0], field.size(), value, INDEX_ADDRESS_MAX, stop_key);
    leveldb::Slice start_slice(reinterpret_cast<char*>(&start_key[0]), start_key.size());

    mtn::index_slice_t::iterator insert_iter = output.begin();

    std::auto_ptr<leveldb::Iterator> iter(_db->NewIterator(_read_options));
    for (iter->Seek(start_slice);
         iter->Valid() && memcmp(iter->key().data(), &stop_key[0], MTN_INDEX_SEGMENT_SIZE) < 0;
         iter->Next())
    {
        uint16_t             temp_partition   = 0;
        mtn::byte_t*         temp_bucket      = NULL;
        uint16_t             temp_bucket_size = 0;
        mtn::byte_t*         temp_field       = NULL;
        uint16_t             temp_field_size  = 0;
        mtn_index_address_t temp_value       = 0;
        mtn_index_address_t offset           = 0;

        assert(iter->value().size() == MTN_INDEX_SEGMENT_SIZE);
        mtn::decode_index_key(reinterpret_cast<const mtn::byte_t*>(iter->key().data()), &temp_partition, &temp_bucket, &temp_bucket_size, &temp_field, &temp_field_size, &temp_value, &offset);
        insert_iter = output.insert(insert_iter, new mtn::index_slice_t::index_node_t(offset, (const index_segment_ptr) iter->value().data()));
    }
    return mtn::status_t(); // XXX TODO better error handling
}

mtn::status_t
mtn::index_reader_writer_leveldb_t::read_segment(mtn_index_partition_t           partition,
                                                 const std::vector<mtn::byte_t>& bucket,
                                                 const std::vector<mtn::byte_t>& field,
                                                 mtn_index_address_t             value,
                                                 mtn_index_address_t             offset,
                                                 mtn::index_segment_ptr          output)
{
    std::vector<mtn::byte_t> key;
    encode_index_key(partition, &bucket[0], bucket.size(), &field[0], field.size(), value, offset, key);
    leveldb::Slice key_slice(reinterpret_cast<char*>(&key[0]), key.size());

    std::auto_ptr<leveldb::Iterator> iter(_db->NewIterator(_read_options));
    iter->Seek(key_slice);
    if (iter->Valid() && iter->key() == key_slice) {
        assert(iter->value().size() == MTN_INDEX_SEGMENT_SIZE);
        memcpy(output, iter->value().data(), MTN_INDEX_SEGMENT_SIZE);
    }
    else {
        memset(output, 0, MTN_INDEX_SEGMENT_SIZE);
    }
    return mtn::status_t();
}

mtn::status_t
mtn::index_reader_writer_leveldb_t::write_segment(mtn_index_partition_t           partition,
                                                  const std::vector<mtn::byte_t>& bucket,
                                                  const std::vector<mtn::byte_t>& field,
                                                  mtn_index_address_t             value,
                                                  mtn_index_address_t             offset,
                                                  mtn::index_segment_ptr          input)
{
    std::vector<mtn::byte_t> key;
    encode_index_key(partition, &bucket[0], bucket.size(), &field[0], field.size(), value, offset, key);
    leveldb::Status db_status = _db->Put(_write_options,
                                         leveldb::Slice(reinterpret_cast<char*>(&key[0]), key.size()),
                                         leveldb::Slice(reinterpret_cast<char*>(input), MTN_INDEX_SEGMENT_SIZE));

    mtn::status_t status;
    if (!db_status.ok()) {
        status.local_storage = true;
        status.code = -1;
        status.message = db_status.ToString();
    }
    return status;
}

mtn::status_t
mtn::index_reader_writer_leveldb_t::estimateSize(mtn_index_partition_t           partition,
                                                 const std::vector<mtn::byte_t>& bucket,
                                                 const std::vector<mtn::byte_t>& field,
                                                 mtn_index_address_t             value,
                                                 uint64_t*                       output)
{
    std::vector<mtn::byte_t> start_key;
    std::vector<mtn::byte_t> stop_key;
    encode_index_key(partition, &bucket[0], bucket.size(), &field[0], field.size(), value, 0, start_key);
    encode_index_key(partition, &bucket[0], bucket.size(), &field[0], field.size(), value, INDEX_ADDRESS_MAX, stop_key);
    leveldb::Range range(leveldb::Slice(reinterpret_cast<char*>(&start_key[0]), start_key.size()),
                         leveldb::Slice(reinterpret_cast<char*>(&stop_key[0]), stop_key.size()));

    _db->GetApproximateSizes(&range, 1, output);
    return mtn::status_t();
}
