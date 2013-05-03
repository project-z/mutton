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

#ifndef __X_INDEX_SLICE_HPP_INCLUDED__
#define __X_INDEX_SLICE_HPP_INCLUDED__

#include <vector>
#include <boost/ptr_container/ptr_list.hpp>

#include "base_types.hpp"
#include "status.hpp"

namespace prz {

    class index_reader_t;
    class index_writer_t;

    class index_slice_t
    {
    public:

        struct index_node_t {
            index_address_t offset;
            index_segment_t segment;

            index_node_t(const index_node_t& node);

            index_node_t(index_address_t offset);

            index_node_t(index_address_t         offset,
                         const index_segment_ptr data);

            void
            zero();
        };

        typedef prz::index_slice_t::index_node_t type;
        typedef boost::ptr_list<prz::index_slice_t::index_node_t> slice_container;
        typedef slice_container::iterator iterator;
        typedef slice_container::const_iterator const_iterator;

        index_slice_t(index_partition_t partition,
                      const char*       field,
                      size_t            field_size,
                      index_address_t   value);

        index_slice_t(index_partition_t partition,
                      const byte_t*     field,
                      size_t            field_size,
                      index_address_t   value);

        index_slice_t(const index_slice_t& other);

        static prz::status_t
        execute(index_operation_enum operation,
                index_slice_t&       a_index,
                index_slice_t&       b_index,
                index_slice_t&       output);

        prz::status_t
        execute(prz::index_operation_enum operation,
                prz::index_reader_t*      reader,
                index_partition_t         partition,
                const byte_t*             field,
                size_t                    field_size,
                index_address_t           value,
                index_slice_t&            output);

        prz::status_t
        execute(prz::index_operation_enum operation,
                prz::index_reader_t*      reader,
                index_partition_t         partition,
                const byte_t*             field,
                size_t                    field_size,
                index_address_t           value);

        prz::status_t
        bit(prz::index_reader_t* reader,
            prz::index_writer_t* writer,
            index_address_t      bit,
            bool                 state);

        bool
        bit(index_address_t      bit);

        index_partition_t
        partition() const;

        const byte_t*
        field() const;

        size_t
        field_size() const;

        index_address_t
        value() const;

        prz::index_slice_t&
        operator=(const index_slice_t& other);

        inline iterator
        begin()
        {
            return _index_slice.begin();
        }

        inline iterator
        end()
        {
            return _index_slice.end();
        }

        inline const_iterator
        cbegin() const
        {
            return _index_slice.cbegin();
        }

        inline const_iterator
        cend() const
        {
            return _index_slice.cend();
        }

        inline iterator
        insert(iterator      pos,
               index_node_t* value)
        {
            return _index_slice.insert(pos, value);
        }

        inline void
        clear()
        {
            _index_slice.clear();
        }

        inline iterator
        erase(iterator first,
              iterator last)
        {
            return _index_slice.erase(first, last);
        }

        inline iterator
        erase(iterator position)
        {
            return _index_slice.erase(position);
        }

        inline size_t
        size()
        {
            return _index_slice.size();
        }

    private:
        slice_container     _index_slice;
        index_partition_t   _partition;
        std::vector<byte_t> _field;
        index_address_t     _value;
    };

} // namespace prz

namespace boost
{
    // specialize range_mutable_iterator and range_const_iterator in namespace boost
    template<>
    struct range_mutable_iterator< prz::index_slice_t >
    {
        typedef prz::index_slice_t::iterator type;
    };

    template<>
    struct range_const_iterator< prz::index_slice_t >
    {
        typedef prz::index_slice_t::iterator type;
    };
} // namespace boost


#endif // __X_INDEX_SLICE_HPP_INCLUDED__
