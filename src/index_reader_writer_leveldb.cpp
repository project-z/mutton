/*
  Copyright (c) 2013 Matthew Stump

  This file is part of libmtn.

  libmtn is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  libmtn is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <vector>
#include "encode.hpp"
#include "index.hpp"
#include "index_slice.hpp"
#include "index_reader_writer_leveldb.hpp"

mtn::index_reader_writer_leveldb_t::index_reader_writer_leveldb_t(leveldb::DB*          db,
                                                                  leveldb::ReadOptions  read_options,
                                                                  leveldb::WriteOptions write_options) :
    _db(db),
    _read_options(read_options),
    _write_options(write_options)
{}

mtn::status_t
mtn::index_reader_writer_leveldb_t::read_index(mtn::index_partition_t partition,
                                               const mtn::byte_t*     field,
                                               size_t                 field_size,
                                               mtn::index_t*          output)
{
    std::vector<char> start_key;
    std::vector<char> stop_key;
    encode_index_key(partition, reinterpret_cast<const char*>(field), field_size, 0, 0, start_key);
    encode_index_key(partition, reinterpret_cast<const char*>(field), field_size, UINT64_MAX, UINT64_MAX, stop_key);
    leveldb::Slice start_slice(reinterpret_cast<char*>(&start_key[0]), start_key.size());

    mtn::index_slice_t* current_slice = NULL;
    mtn::index_address_t current_slice_value = UINT64_MAX;

    std::auto_ptr<leveldb::Iterator> iter(_db->NewIterator(_read_options));
    for (iter->Seek(start_slice);
         iter->Valid() && memcmp(iter->key().data(), &stop_key[0], MTN_INDEX_SEGMENT_SIZE) < 0;
         iter->Next())
    {
        uint16_t temp_partition = 0;
        char*    temp_field = NULL;
        uint16_t temp_field_size = 0;
        uint64_t temp_value = 0;
        uint64_t offset = 0;
        assert(iter->value().size() == MTN_INDEX_SEGMENT_SIZE);
        mtn::decode_index_key(iter->key().data(), &temp_partition, &temp_field, &temp_field_size, &temp_value, &offset);

        if (current_slice_value != temp_value) {
            mtn::index_t::iterator insert_iter = output->find(temp_value);
            if (insert_iter != output->end()) {
                current_slice = insert_iter->second;
            }
            else {
                current_slice = output->insert(temp_value, new mtn::index_slice_t(temp_partition, temp_field, temp_field_size, temp_value)).first->second;
                current_slice_value = temp_value;
            }
        }
        current_slice->insert(current_slice->end(), new mtn::index_slice_t::index_node_t(offset, (const index_segment_ptr) iter->value().data()));
    }
    return mtn::status_t(); // XXX TODO better error handling
}

mtn::status_t
mtn::index_reader_writer_leveldb_t::read_index_slice(mtn::index_partition_t partition,
                                                     const mtn::byte_t*     field,
                                                     size_t                 field_size,
                                                     mtn::index_address_t   value,
                                                     mtn::index_slice_t*    output)
{
    std::vector<char> start_key;
    std::vector<char> stop_key;
    encode_index_key(partition, reinterpret_cast<const char*>(field), field_size, value, 0, start_key);
    encode_index_key(partition, reinterpret_cast<const char*>(field), field_size, value, UINT64_MAX, stop_key);
    leveldb::Slice start_slice(reinterpret_cast<char*>(&start_key[0]), start_key.size());

    mtn::index_slice_t::iterator insert_iter = output->begin();

    std::auto_ptr<leveldb::Iterator> iter(_db->NewIterator(_read_options));
    for (iter->Seek(start_slice);
         iter->Valid() && memcmp(iter->key().data(), &stop_key[0], MTN_INDEX_SEGMENT_SIZE) < 0;
         iter->Next())
    {
        uint16_t temp_partition = 0;
        char*    temp_field = NULL;
        uint16_t temp_field_size = 0;
        uint64_t temp_value = 0;
        uint64_t offset = 0;
        assert(iter->value().size() == MTN_INDEX_SEGMENT_SIZE);
        mtn::decode_index_key(iter->key().data(), &temp_partition, &temp_field, &temp_field_size, &temp_value, &offset);
        insert_iter = output->insert(insert_iter, new mtn::index_slice_t::index_node_t(offset, (const index_segment_ptr) iter->value().data()));
    }
    return mtn::status_t(); // XXX TODO better error handling
}

mtn::status_t
mtn::index_reader_writer_leveldb_t::read_segment(mtn::index_partition_t partition,
                                                 const mtn::byte_t*     field,
                                                 size_t                 field_size,
                                                 mtn::index_address_t   value,
                                                 mtn::index_address_t   offset,
                                                 mtn::index_segment_ptr output)
{
    std::vector<char> key;
    leveldb::Slice key_slice(reinterpret_cast<char*>(&key[0]), key.size());
    encode_index_key(partition, reinterpret_cast<const char*>(field), field_size, value, offset, key);

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
mtn::index_reader_writer_leveldb_t::write_segment(mtn::index_partition_t partition,
                                                  const mtn::byte_t*     field,
                                                  size_t                 field_size,
                                                  mtn::index_address_t   value,
                                                  mtn::index_address_t   offset,
                                                  mtn::index_segment_ptr input)
{
    std::vector<char> key;
    encode_index_key(partition, reinterpret_cast<const char*>(field), field_size, value, offset, key);
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
mtn::index_reader_writer_leveldb_t::estimateSize(mtn::index_partition_t partition,
                                                 const mtn::byte_t*     field,
                                                 size_t                 field_size,
                                                 mtn::index_address_t   value,
                                                 uint64_t*              output)
{
    std::vector<char> start_key;
    std::vector<char> stop_key;
    encode_index_key(partition, reinterpret_cast<const char*>(field), field_size, value, 0, start_key);
    encode_index_key(partition, reinterpret_cast<const char*>(field), field_size, value, UINT64_MAX, stop_key);
    leveldb::Range range(leveldb::Slice(reinterpret_cast<char*>(&start_key[0]), start_key.size()),
                         leveldb::Slice(reinterpret_cast<char*>(&stop_key[0]), stop_key.size()));

    _db->GetApproximateSizes(&range, 1, output);
    return mtn::status_t();
}
