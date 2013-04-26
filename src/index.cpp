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

inline prz::index::iterator
find_insertion_point(prz::index::iterator begin,
                     prz::index::iterator end,
                     uint64_t bucket)
{
    return std::find_if(begin,
                        end,
                        boost::bind(&prz::index::index_node_t::offset, _1 ) >= bucket);
}

inline prz::index::iterator
insert_node(prz::index&           output,
            prz::index::iterator& output_iter,
            uint64_t*             data,
            uint64_t              offset)
{
    output_iter = find_insertion_point(output_iter, output.end(), offset);
    if (output_iter == output.end()) {
        return output.insert(output_iter, new prz::index::index_node_t(offset, data));
    }
    else {
        memcpy(output_iter->segment, data, SEGMENT_SIZE);
        return output_iter;
    }
}

inline bool
union_behavior(prz::index::index& a_index,
               prz::index::index& b_index,
               prz::index::index& output)
{

    prz::index::iterator a_iter = a_index.begin();
    prz::index::iterator b_iter = b_index.begin();
    prz::index::iterator output_iter = output.begin();

    for (;;) {
        if (a_iter == a_index.end() && b_iter == b_index.end()) {
            break;
        }
        else if (a_iter == a_index.end()) {
            if (&output == &b_index) {
                break;
            }
            output_iter = insert_node(output, output_iter, b_iter->segment, b_iter->offset);
            ++b_iter;
        }
        else if (a_iter->offset < b_iter->offset) {
            output_iter = insert_node(output, output_iter, a_iter->segment, a_iter->offset);
            ++a_iter;
        }
        else if (a_iter->offset > b_iter->offset) {
            output_iter = insert_node(output, output_iter, b_iter->segment, b_iter->offset);
            ++b_iter;
        }
        else if (a_iter->offset == b_iter->offset) {
            prz::index::segment_t out_segment;
            segment_union(a_iter->segment, b_iter->segment, out_segment);
            output_iter = insert_node(output, output_iter, out_segment, b_iter->offset);
            ++a_iter;
            ++b_iter;
        }
        else {
            // XXX shit's gone crazy log and abort
            return false;
        }
    }
    return true;
}

inline bool
intersection_behavior(prz::index::index& a_index,
                      prz::index::index& b_index,
                      prz::index::index& output)
{
    prz::index::iterator a_iter = a_index.begin();
    prz::index::iterator b_iter = b_index.begin();
    prz::index::iterator output_iter = output.begin();

    for (;;) {
        if (b_iter == b_index.end() || a_iter == a_index.end()) {
            output.erase(output_iter, output.end());
            break;
        }
        else if (a_iter->offset < b_iter->offset) {
            ++a_iter;
        }
        else if (a_iter->offset > b_iter->offset) {
            ++b_iter;
        }
        else if (a_iter->offset == b_iter->offset) {
            prz::index::segment_t out_segment;
            segment_intersection(a_iter->segment, b_iter->segment, out_segment);
            output_iter = insert_node(output, output_iter, out_segment, b_iter->offset);
            ++a_iter;
            ++b_iter;
        }
        else {
            // XXX shit's gone crazy log and abort
            return false;
        }
    }
    return true;
}

prz::index::index_node_t::index_node_t(const index_node_t& node) :
    offset(node.offset)
{
    memcpy(segment, node.segment, SEGMENT_SIZE);
}

prz::index::index_node_t::index_node_t(uint64_t offset) :
    offset(offset)
{
    memset(segment, 0, SEGMENT_SIZE);
}

prz::index::index_node_t::index_node_t(uint64_t offset,
                                       const uint64_t* data) :
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
prz::index::execute(index_operation_enum operation,
                    prz::index::index&   a_index,
                    prz::index::index&   b_index,
                    prz::index::index&   output)
{
    if (operation == INDEX_INTERSECTION) {
        return intersection_behavior(a_index, b_index, output);
    }
    else if (operation == INDEX_UNION) {
        return union_behavior(a_index, b_index, output);
    }
    return false;
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
    prz::index::iterator it = find_insertion_point(begin(), end(), bucket);

    if (it->offset != bucket) {
        it = prz::index::iterator(_index.insert(it.base(), new index_node_t(bucket)));
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
    prz::index::iterator it = find_insertion_point(begin(), end(), bucket);

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
