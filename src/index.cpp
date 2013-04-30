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
#include <stdint.h>
#include <tmmintrin.h>
#include <immintrin.h>

#include "encode.hpp"
#include "index_reader.hpp"
#include "index_writer.hpp"

#include "index.hpp"

// Required to use stdint.h
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

inline void
get_address(prz::index_address_t    position,
            prz::index_address_t*   bucket,
            prz::index_partition_t* bucket_index,
            prz::index_partition_t* bit_offset)
{
    *bucket = position >> 8; // div by 256
    prz::index_partition_t bucket_offset = position & 0xFF; // mod by 256
    *bucket_index = bucket_offset >> 6; // div by 64
    *bit_offset = bucket_offset & 0x3F; // mod by 64
}

inline void
set_bit(prz::index_segment_ptr input,
        prz::index_partition_t bucket_index,
        prz::index_partition_t bit_offset,
        bool                   val)
{
    input[bucket_index] |= (val ? 1 : 0) << bit_offset;
}

inline void
segment_union(prz::index_segment_ptr a,
              prz::index_segment_ptr b,
              prz::index_segment_ptr o)
{
    for (int i = 0; i < INDEX_SEGMENT_LENGTH; ++i) {
        o[i] = a[i] | b[i];
    }
}

inline void
segment_intersection(prz::index_segment_ptr a,
                     prz::index_segment_ptr b,
                     prz::index_segment_ptr o)
{
    for (int i = 0; i < INDEX_SEGMENT_LENGTH; ++i) {
        o[i] = a[i] & b[i];
    }
}

inline prz::index_t::iterator
find_insertion_point(prz::index_t::iterator begin,
                     prz::index_t::iterator end,
                     prz::index_address_t bucket)
{
    return std::find_if(begin,
                        end,
                        boost::bind(&prz::index_t::index_node_t::offset, _1 ) >= bucket);
}

inline prz::index_t::iterator
get_output_node(prz::index_t&          output,
                prz::index_t::iterator output_iter,
                prz::index_address_t        offset)
{
    output_iter = find_insertion_point(output_iter, output.end(), offset);
    if (output_iter == output.end()) {
        return output.insert(output_iter, new prz::index_t::index_node_t(offset));
    }
    return output_iter;
}

inline bool
union_behavior(prz::index_t& a_index,
               prz::index_t& b_index,
               prz::index_t& output)
{

    prz::index_t::iterator a_iter = a_index.begin();
    prz::index_t::iterator b_iter = b_index.begin();
    prz::index_t::iterator output_iter = output.begin();

    for (;;) {
        if (a_iter == a_index.end() && b_iter == b_index.end()) {
            break;
        }
        else if (a_iter == a_index.end()) {
            if (&output == &b_index) {
                break;
            }
            output_iter = get_output_node(output, output_iter, b_iter->offset);
            memcpy(output_iter->segment, b_iter->segment, INDEX_SEGMENT_SIZE);
            ++b_iter;
        }
        else if (a_iter->offset < b_iter->offset) {
            output_iter = get_output_node(output, output_iter, a_iter->offset);
            memcpy(output_iter->segment, a_iter->segment, INDEX_SEGMENT_SIZE);
            ++a_iter;
        }
        else if (a_iter->offset > b_iter->offset) {
            output_iter = get_output_node(output, output_iter, b_iter->offset);
            memcpy(output_iter->segment, b_iter->segment, INDEX_SEGMENT_SIZE);
            ++b_iter;
        }
        else if (a_iter->offset == b_iter->offset) {
            output_iter = get_output_node(output, output_iter, b_iter->offset);
            segment_union(a_iter->segment, b_iter->segment, output_iter->segment);
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
intersection_behavior(prz::index_t& a_index,
                      prz::index_t& b_index,
                      prz::index_t& output)
{
    prz::index_t::iterator a_iter = a_index.begin();
    prz::index_t::iterator b_iter = b_index.begin();
    prz::index_t::iterator output_iter = output.begin();

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
            prz::index_t::iterator new_output_iter = get_output_node(output, output_iter, b_iter->offset);
            if (output_iter != output.end() && new_output_iter != output_iter) {
                output.erase(output_iter, new_output_iter);
            }
            output_iter = new_output_iter;
            segment_intersection(a_iter->segment, b_iter->segment, output_iter->segment);
            ++a_iter;
            ++b_iter;
            ++output_iter;
        }
        else {
            // XXX shit's gone crazy log and abort
            return false;
        }
    }
    return true;
}

prz::index_t::index_node_t::index_node_t(const index_node_t& node) :
    offset(node.offset)
{
    memcpy(segment, node.segment, INDEX_SEGMENT_SIZE);
}

prz::index_t::index_node_t::index_node_t(prz::index_address_t offset) :
    offset(offset)
{}

prz::index_t::index_node_t::index_node_t(prz::index_address_t offset,
                                         const index_segment_ptr data) :
    offset(offset)
{
    memcpy(segment, data, INDEX_SEGMENT_SIZE);
}

void
prz::index_t::index_node_t::zero()
{
    memset(segment, 0, INDEX_SEGMENT_SIZE);
}

prz::index_t::index_t(prz::index_partition_t partition,
                      const char*            field,
                      size_t                 field_size,
                      prz::index_address_t   value) :
    _partition(partition),
    _field(field, field + field_size),
    _value(value)
{}

prz::index_t::index_t(prz::index_partition_t partition,
                      const prz::byte_t*     field,
                      size_t                 field_size,
                      prz::index_address_t   value) :
    _partition(partition),
    _field(field, field + field_size),
    _value(value)
{}

bool
prz::index_t::execute(index_operation_enum operation,
                      prz::index_t&        a_index,
                      prz::index_t&        b_index,
                      prz::index_t&        output)
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
prz::index_t::execute(prz::index_operation_enum operation,
                      prz::index_reader_t*      reader,
                      prz::index_partition_t    partition,
                      const prz::byte_t*        field,
                      size_t                    field_size,
                      prz::index_address_t      value,
                      prz::index_t&             output)
{

}

void
prz::index_t::execute(prz::index_operation_enum operation,
                      prz::index_reader_t*      reader,
                      prz::index_partition_t    partition,
                      const prz::byte_t*        field,
                      size_t                    field_size,
                      prz::index_address_t      value)
{

}

void
prz::index_t::bit(prz::index_reader_t* reader,
                  prz::index_writer_t* writer,
                  prz::index_address_t bit,
                  bool                 state)
{
    prz::index_address_t   offset       = 0;
    prz::index_partition_t offset_index = 0;
    prz::index_partition_t bit_offset   = 0;
    get_address(bit, &offset, &offset_index, &bit_offset);

    prz::index_t::iterator it = find_insertion_point(begin(), end(), offset);

    if (it->offset != offset) {
        it = prz::index_t::iterator(_index.insert(it.base(), new index_node_t(offset)));
        reader->read_segment(_partition, &_field[0], _field.size(), _value, offset, it->segment);
    }
    set_bit(it->segment, offset_index, bit_offset, state);
    writer->write_segment(_partition, &_field[0], _field.size(), _value, offset, it->segment);
}

bool
prz::index_t::bit(index_address_t      bit)
{
    prz::index_address_t        bucket       = 0;
    prz::index_partition_t      bucket_index = 0;
    prz::index_partition_t      bit_offset   = 0;
    get_address(bit, &bucket, &bucket_index, &bit_offset);

    prz::index_t::iterator it = find_insertion_point(begin(), end(), bucket);

    if (it->offset != bucket) {
        return false;
    }
    return (it->segment[bucket_index] & 1 << bit_offset);
}

prz::index_partition_t
prz::index_t::partition() const
{
    return _partition;
}

const prz::byte_t*
prz::index_t::field() const
{
    return &_field[0];
}

prz::index_address_t
prz::index_t::value() const
{
    return _value;
}
