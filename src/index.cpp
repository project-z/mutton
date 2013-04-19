/*
  Copyright (c) 2013 Matthew Stump

  This file is part of libprz.

  libprz is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  libprz is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <boost/bind.hpp>
#include <leveldb/db.h>
#include <stdint.h>

#include "index.hpp"
#include "encode.hpp"

// Required to use stdint.h
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

inline void
get_address(uint64_t  position,
            uint64_t* bucket,
            uint8_t*  bucket_index,
            uint8_t*  bit_offset)
{
    *bucket = position >> 8; // div by 256
    uint8_t bucket_offset = position & 0xFF; // mod by 256
    *bucket_index = bucket_offset >> 6; // div by 64
    *bit_offset = bucket_offset & 0x3F; // mod by 64
}

inline void
set_bit(uint64_t* input,
        uint8_t   bucket_index,
        uint8_t   bit_offset,
        bool      val)
{
    input[bucket_index] |= (val ? 1 : 0) << bit_offset;
}

prz::index::index_node_t::index_node_t(uint64_t    offset) :
    offset(offset)
{
    memset(segment, 0, SEGMENT_SIZE);
}

prz::index::index_node_t::index_node_t(uint64_t    offset,
                                       const char* data) :
    offset(offset)
{
    memcpy(segment, data, SEGMENT_SIZE);
}

prz::index::index(leveldb::DB*          db,
                  leveldb::ReadOptions* options,
                  uint8_t               partition,
                  const char*           field,
                  size_t                field_size,
                  uint64_t              value) :
    _partition(partition),
    _field(field, field + field_size),
    _value(value)
{}

void
prz::index::execute(index_operation_enum  operation,
                    index&                other_index,
                    index&                output)
{
    prz::index::iterator this_iter = begin();
    for (; this_iter != end(); ++ this_iter) {

    }
}

void
prz::index::execute(leveldb::DB*          db,
                    leveldb::ReadOptions* options,
                    index_operation_enum  operation,
                    uint8_t               partition,
                    const char*           field,
                    size_t                field_size,
                    uint64_t              value,
                    index&                output)
{

}

void
prz::index::execute(leveldb::DB*          db,
                    leveldb::ReadOptions* options,
                    index_operation_enum  operation,
                    uint8_t               partition,
                    const char*           field,
                    size_t                field_size,
                    uint64_t              value)
{

}

void
prz::index::bit(leveldb::DB*           db,
                leveldb::WriteOptions* options,
                uint64_t               bit,
                bool                   state)
{
    uint64_t bucket = 0;
    uint8_t  bucket_index = 0;
    uint8_t  bit_offset = 0;
    get_address(bit, &bucket, &bucket_index, &bit_offset);
    prz::index::index_container::iterator it = find_insertion_point(bucket);

    if (it->offset != bucket) {
        it = _index.insert(it, new index_node_t(bucket));
    }
    set_bit(it->segment, bucket_index, bit_offset, state);

    std::vector<char> key;
    encode_index_key(_partition, &_field[0], _field.size(), _value, bucket, key);
    leveldb::Status status = db->Put(*options,
                                     leveldb::Slice(&key[0], key.size()),
                                     leveldb::Slice((char*)it->segment, SEGMENT_SIZE));
}

bool
prz::index::bit(uint64_t bit)
{
    uint64_t bucket = 0;
    uint8_t  bucket_index = 0;
    uint8_t  bit_offset = 0;
    get_address(bit, &bucket, &bucket_index, &bit_offset);
    prz::index::index_container::iterator it = find_insertion_point(bucket);

    if (it->offset != bucket) {
        return false;
    }
    return (it->segment[bucket_index] & 1 << bit_offset);
}

uint8_t
prz::index::partition() const
{
    return _partition;
}

const char*
prz::index::field() const
{
    return &_field[0];
}

uint64_t
prz::index::value() const
{
    return _value;
}

uint64_t
prz::index::estimateSize(leveldb::DB* db,
                         uint8_t      partition,
                         const char*  field,
                         size_t       field_size,
                         uint64_t     value)
{
    std::vector<char> start_key;
    std::vector<char> stop_key;
    encode_index_key(partition, field, field_size, value, 0, start_key);
    encode_index_key(partition, field, field_size, value, UINT64_MAX, stop_key);
    leveldb::Range range(leveldb::Slice(&start_key[0], start_key.size()),
                         leveldb::Slice(&stop_key[0], stop_key.size()));
    uint64_t size;
    db->GetApproximateSizes(&range, 1, &size);
    return size;
}

prz::index::index_container::iterator
prz::index::find_insertion_point(uint64_t bucket)
{
    return std::find_if(_index.begin(),
                        _index.end(),
                        boost::bind(&prz::index::index_node_t::offset, _1 ) >= bucket);
}
