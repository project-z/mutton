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

#ifndef __MUTTON_TEST_FIXTURES_HPP_INCLUDED__
#define __MUTTON_TEST_FIXTURES_HPP_INCLUDED__

#include <stdint.h>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>

#include "base_types.hpp"
#include "index.hpp"
#include "index_reader_writer.hpp"
#include "index_slice.hpp"

// Required to use stdint.h
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

typedef std::basic_string<unsigned char> ustring_t;

class index_reader_writer_memory_t :
    public mtn::index_reader_writer_t,
    private boost::noncopyable
{

public:

    struct index_key_t {

        mtn_index_partition_t partition;
        ustring_t             bucket;
        ustring_t             field;
        mtn_index_address_t   value;

        index_key_t(mtn_index_partition_t           partition,
                    const std::vector<mtn::byte_t>& bucket,
                    const std::vector<mtn::byte_t>& field,
                    mtn_index_address_t             value) :
            partition(partition),
            bucket(&bucket[0], bucket.size()),
            field(&field[0], bucket.size()),
            value(value)
        {}

        index_key_t(mtn_index_partition_t partition,
                    const mtn::byte_t*    bucket,
                    size_t                bucket_size,
                    const mtn::byte_t*    field,
                    size_t                field_size,
                    mtn_index_address_t   value) :
            partition(partition),
            bucket(bucket, bucket + bucket_size),
            field(field, field + field_size),
            value(value)
        {}

        friend bool
        operator<(const index_key_t& a,
                  const index_key_t& b)
        {
            if (a.partition != b.partition) {
                return a.partition < b.partition;
            }
            if (a.bucket != b.bucket) {
                return a.bucket < b.bucket;
            }
            if (a.field != b.field) {
                return a.field < b.field;
            }
            return a.value < b.value;
        }

        friend bool
        operator<=(const index_key_t& a,
                   const index_key_t& b)
        {
            if (a.partition != b.partition) {
                return a.partition <= b.partition;
            }
            if (a.bucket != b.bucket) {
                return a.bucket <= b.bucket;
            }
            if (a.field != b.field) {
                return a.field <= b.field;
            }
            return a.value <= b.value;
        }
    };

    typedef boost::ptr_map<index_key_t, mtn::index_slice_t> index_container_t;

    index_reader_writer_memory_t()
    {}

    mtn::status_t
    write_segment(mtn_index_partition_t           partition,
                  const std::vector<mtn::byte_t>& bucket,
                  const std::vector<mtn::byte_t>& field,
                  mtn_index_address_t             value,
                  mtn_index_address_t             offset,
                  mtn::index_segment_ptr          input)
    {
        index_key_t key(partition, bucket, field, value);
        index_container_t::iterator iter = _index.find(key);

        if (iter == _index.end()) {
            iter = _index.insert(key, new mtn::index_slice_t(partition, bucket, field, value)).first;
        }

        mtn::index_slice_t::iterator slice_insert_iter \
            = std::find_if(iter->second->begin(),
                           iter->second->end(),
                           boost::bind(&mtn::index_slice_t::index_node_t::offset, _1 ) >= offset);

        if (slice_insert_iter != iter->second->end() && slice_insert_iter->offset == offset) {
            slice_insert_iter = iter->second->erase(slice_insert_iter);
        }

        iter->second->insert(slice_insert_iter, new mtn::index_slice_t::index_node_t(offset, input));
        return mtn::status_t();
    }

    mtn::status_t
    read_indexes(mtn_index_partition_t                        partition,
                 const std::vector<mtn::byte_t>&              start_bucket,
                 const std::vector<mtn::byte_t>&              start_field,
                 const std::vector<mtn::byte_t>&              end_bucket,
                 const std::vector<mtn::byte_t>&              end_field,
                 mtn::index_reader_writer_t::index_container& output)
    {
        index_key_t start_key(partition, start_bucket, start_field, 0);
        index_key_t end_key(partition, end_bucket, end_field, 0);

        ustring_t current_field;
        mtn::index_t* current_index = NULL;

        index_container_t::iterator iter = _index.lower_bound(start_key);
        for (; iter != _index.end() && iter->first <= end_key; ++iter) {

            if (iter->first.field != current_field) {
                mtn::index_reader_writer_t::index_container::key_type key(iter->first.field.begin(), iter->first.field.end());
                current_index = output.insert(
                    key,
                    new mtn::index_t(partition,
                                     iter->first.bucket.c_str(),
                                     iter->first.bucket.size(),
                                     iter->first.field.c_str(),
                                     iter->first.field.size())
                    ).first->second;

                current_field = iter->first.field;
            }
            current_index->insert(iter->second->value(), new mtn::index_slice_t(*iter->second));
        }
        return mtn::status_t();
    }

    mtn::status_t
    read_index(mtn_index_partition_t           partition,
               const std::vector<mtn::byte_t>& bucket,
               const std::vector<mtn::byte_t>& field,
               mtn::index_t**                  output)
    {
        index_key_t start_key(partition, bucket, field, 0);
        index_key_t end_key(partition, bucket, field, INDEX_ADDRESS_MAX);

        *output = new mtn::index_t(partition, bucket, field);
        index_container_t::iterator iter = _index.lower_bound(start_key);
        for (; iter != _index.end() && iter->first <= end_key; ++iter) {
            (*output)->insert(iter->second->value(), new mtn::index_slice_t(*iter->second));
        }
        return mtn::status_t();
    }

    mtn::status_t
    read_index_slice(mtn_index_partition_t           partition,
                     const std::vector<mtn::byte_t>& bucket,
                     const std::vector<mtn::byte_t>& field,
                     mtn_index_address_t             value,
                     mtn::index_slice_t&             output)
    {
        index_key_t key(partition, bucket, field, value);
        index_container_t::iterator iter = _index.find(key);
        if (iter != _index.end()) {
            output = *iter->second;
        }
        return mtn::status_t();
    }

    mtn::status_t
    read_segment(mtn_index_partition_t           partition,
                 const std::vector<mtn::byte_t>& bucket,
                 const std::vector<mtn::byte_t>& field,
                 mtn_index_address_t             value,
                 mtn_index_address_t             offset,
                 mtn::index_segment_ptr          output)
    {
        index_key_t key(partition, bucket, field, value);
        index_container_t::iterator iter = _index.find(key);
        if (iter != _index.end()) {
            mtn::index_slice_t::iterator slice_iter     \
                = std::find_if(iter->second->begin(),
                               iter->second->end(),
                               boost::bind(&mtn::index_slice_t::index_node_t::offset, _1 ) == offset);

            if (slice_iter != iter->second->end()) {
                memcpy(output, slice_iter->segment, MTN_INDEX_SEGMENT_SIZE);
                return mtn::status_t();
            }
        }
        memset(output, 0, MTN_INDEX_SEGMENT_SIZE);
        return mtn::status_t();
    }

    mtn::status_t
    estimateSize(mtn_index_partition_t,
                 const std::vector<mtn::byte_t>&,
                 const std::vector<mtn::byte_t>&,
                 mtn_index_address_t,
                 uint64_t* output)
    {
        *output = 0;
        return mtn::status_t();
    }

private:
    index_container_t _index;
};


#endif // __MUTTON_TEST_FIXTURES_HPP_INCLUDED__
