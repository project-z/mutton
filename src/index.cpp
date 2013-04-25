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

inline void
segment_union(uint64_t* a,
              uint64_t* b,
              uint64_t* o)
{

}

inline void
segment_intersection(uint64_t* a,
                     uint64_t* b,
                     uint64_t* o)
{

}

prz::index::index_node_t::index_node_t(const index_node_t& node) :
    offset(node.offset)
{
    memcpy(segment, node.segment, SEGMENT_SIZE);
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

bool
prz::index::execute(index_operation_enum  operation,
                    index&                a_index,
                    index&                b_index,
                    index&                output)
{
    if (operation != INDEX_INTERSECTION
        && operation != INDEX_UNION) {
        return false;
    }

    output.clear();
    prz::index::iterator a_iter = a_index.begin();
    prz::index::iterator b_iter = b_index.begin();
    prz::index::iterator output_iter = output.begin();

    for (;;) {
        if (a_iter == a_index.end() && b_iter == b_index.end()) {
            break;
        }
        else if (operation == INDEX_INTERSECTION && (b_iter == b_index.end() || a_iter == a_index.end())) {
            break;
        }
        else if (operation == INDEX_UNION && a_iter == a_index.end()) {
            output_iter = output.insert(output_iter, new index_node_t(*b_iter));
            ++b_iter;
            ++output_iter;
        }
        else if (operation == INDEX_UNION && b_iter == b_index.end()) {
            output_iter = output.insert(output_iter, new index_node_t(*a_iter));
            ++a_iter;
            ++output_iter;
        }
        else if (operation == INDEX_UNION && a_iter->offset < b_iter->offset) {
            output_iter = output.insert(output_iter, new index_node_t(*a_iter));
            ++a_iter;
            ++output_iter;
        }
        else if (operation == INDEX_UNION && a_iter->offset > b_iter->offset) {
            output_iter = output.insert(output_iter, new index_node_t(*b_iter));
            ++b_iter;
            ++output_iter;
        }
        else if (operation == INDEX_INTERSECTION && a_iter->offset < b_iter->offset) {
            ++a_iter;
        }
        else if (operation == INDEX_INTERSECTION && a_iter->offset > b_iter->offset) {
            ++b_iter;
        }
        else if (a_iter->offset == b_iter->offset) {
            std::auto_ptr<index_node_t> out_node(new index_node_t(a_iter->offset));
            if (operation == INDEX_INTERSECTION) {
                segment_intersection(a_iter->segment, b_iter->segment, out_node->segment);
            }
            else {
                segment_union(a_iter->segment, b_iter->segment, out_node->segment);
            }
            output_iter = output.insert(output_iter, out_node.release());
            ++a_iter;
            ++b_iter;
            ++output_iter;
        }
        else {
            // XXX shit's gone crazy log and abort
            break;
        }
    }
    return true;
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
