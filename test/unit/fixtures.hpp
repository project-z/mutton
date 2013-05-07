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

#ifndef __X_TEST_FIXTURES_HPP_INCLUDED__
#define __X_TEST_FIXTURES_HPP_INCLUDED__

#include <stdint.h>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>

#include "base_types.hpp"
#include "index_reader.hpp"
#include "index_writer.hpp"
#include "index.hpp"
#include "index_slice.hpp"

// Required to use stdint.h
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

static prz::index_segment_t SEGMENT_NONE = {0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0};

static prz::index_segment_t SEGMENT_ONE = {1, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0};

static prz::index_segment_t SEGMENT_EVERY_OTHER_ODD = {0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX,
                                                       0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX,
                                                       0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX,
                                                       0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX};

static prz::index_segment_t SEGMENT_EVERY_OTHER_EVEN = {UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0,
                                                        UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0,
                                                        UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0,
                                                        UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0};

static prz::index_segment_t SEGMENT_EVERY = {UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                                             UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                                             UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                                             UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX};

typedef std::basic_string<unsigned char> ustring_t;

class index_reader_writer_memory_t :
    public prz::index_reader_t,
    public prz::index_writer_t,
    private boost::noncopyable
{

public:

    struct index_key_t {

        prz::index_partition_t partition;
        ustring_t              field;
        prz::index_address_t   value;

        index_key_t(prz::index_partition_t partition,
                    const prz::byte_t*     field,
                    size_t                 field_size,
                    prz::index_address_t   value) :
            partition(partition),
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
            if (a.field != b.field) {
                return a.field <= b.field;
            }
            return a.value <= b.value;
        }
    };

    typedef boost::ptr_map<index_key_t, prz::index_slice_t> index_container_t;

    index_reader_writer_memory_t()
    {}

    prz::status_t
    write_segment(prz::index_partition_t partition,
                  const prz::byte_t*     field,
                  size_t                 field_size,
                  prz::index_address_t   value,
                  prz::index_address_t   offset,
                  prz::index_segment_ptr input)
    {
        index_key_t key(partition, field, field_size, value);
        index_container_t::iterator iter = _index.find(key);

        if (iter == _index.end()) {
            iter = _index.insert(key, new prz::index_slice_t(partition, field, field_size, value)).first;
        }

        prz::index_slice_t::iterator slice_insert_iter \
            = std::find_if(iter->second->begin(),
                           iter->second->end(),
                           boost::bind(&prz::index_slice_t::index_node_t::offset, _1 ) >= offset);

        if (slice_insert_iter != iter->second->end() && slice_insert_iter->offset == offset) {
            slice_insert_iter = iter->second->erase(slice_insert_iter);
        }

        iter->second->insert(slice_insert_iter, new prz::index_slice_t::index_node_t(offset, input));
        return prz::status_t();
    }

    prz::status_t
    read_index(prz::index_partition_t partition,
               const prz::byte_t*     field,
               size_t                 field_size,
               prz::index_t*          output)
    {
        index_key_t start_key(partition, field, field_size, 0);
        index_key_t end_key(partition, field, field_size, UINT64_MAX);

        index_container_t::iterator iter = _index.lower_bound(start_key);
        for (; iter != _index.end() && iter->first <= end_key; ++iter) {
            output->insert(iter->second->value(), new prz::index_slice_t(*iter->second));
        }
        return prz::status_t();
    }

    prz::status_t
    read_index_slice(prz::index_partition_t partition,
                     const prz::byte_t*     field,
                     size_t                 field_size,
                     prz::index_address_t   value,
                     prz::index_slice_t*    output)
    {
        index_key_t key(partition, field, field_size, value);
        index_container_t::iterator iter = _index.find(key);
        if (iter != _index.end()) {
            *output = *iter->second;
        }
        return prz::status_t();
    }

    prz::status_t
    read_segment(prz::index_partition_t partition,
                 const prz::byte_t*     field,
                 size_t                 field_size,
                 prz::index_address_t   value,
                 prz::index_address_t   offset,
                 prz::index_segment_ptr output)
    {
        index_key_t key(partition, field, field_size, value);
        index_container_t::iterator iter = _index.find(key);
        if (iter != _index.end()) {
            prz::index_slice_t::iterator slice_iter     \
                = std::find_if(iter->second->begin(),
                               iter->second->end(),
                               boost::bind(&prz::index_slice_t::index_node_t::offset, _1 ) == offset);

            if (slice_iter != iter->second->end()) {
                memcpy(output, slice_iter->segment, PRZ_INDEX_SEGMENT_SIZE);
                return prz::status_t();
            }
        }
        memset(output, 0, PRZ_INDEX_SEGMENT_SIZE);
        return prz::status_t();
    }

    prz::status_t
    estimateSize(prz::index_partition_t partition,
                 const prz::byte_t*     field,
                 size_t                 field_size,
                 prz::index_address_t   value,
                 uint64_t*              output)
    {
        *output = 0;
        return prz::status_t();
    }

private:
    index_container_t _index;
};


#endif // __X_TEST_FIXTURES_HPP_INCLUDED__
