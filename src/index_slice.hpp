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

#ifndef __MUTTON_INDEX_SLICE_HPP_INCLUDED__
#define __MUTTON_INDEX_SLICE_HPP_INCLUDED__

#include <string>
#include <vector>
#include <boost/ptr_container/ptr_list.hpp>

#include "base_types.hpp"
#include "status.hpp"

namespace mtn {

    class index_reader_writer_t;

    class index_slice_t
    {
    public:

        struct index_node_t {
            mtn_index_address_t offset;
            index_segment_t     segment;

            index_node_t(const index_node_t& node);

            index_node_t(mtn_index_address_t offset);

            index_node_t(mtn_index_address_t     offset,
                         const index_segment_ptr data);

            void
            zero();
        };

        typedef mtn::index_slice_t::index_node_t type;
        typedef boost::ptr_list<mtn::index_slice_t::index_node_t> slice_container;
        typedef slice_container::iterator iterator;
        typedef slice_container::const_iterator const_iterator;

        index_slice_t();

        index_slice_t(mtn_index_partition_t           partition,
                      const std::vector<mtn::byte_t>& bucket,
                      const std::vector<mtn::byte_t>& field,
                      mtn_index_address_t             value);

        index_slice_t(mtn_index_partition_t partition,
                      const mtn::byte_t*    bucket,
                      size_t                bucket_size,
                      const mtn::byte_t*    field,
                      size_t                field_size,
                      mtn_index_address_t   value);

        index_slice_t(const index_slice_t& other);

        void
        invert();

        static mtn::status_t
        execute(index_operation_enum operation,
                index_slice_t&       a_index,
                index_slice_t&       b_index,
                index_slice_t&       output);

        mtn::status_t
        bit(mtn::index_reader_writer_t& rw,
            mtn_index_address_t         bit,
            bool                        state);

        bool
        bit(mtn_index_address_t bit);

        mtn::index_slice_t&
        operator=(const index_slice_t& other);

        inline mtn_index_partition_t
        partition() const
        {
            return _partition;
        }

        inline const std::vector<mtn::byte_t>&
        bucket() const
        {
            return _bucket;
        }

        inline const std::vector<mtn::byte_t>&
        field() const
        {
            return _field;
        }

        inline mtn_index_address_t
        value() const
        {
            return _value;
        }

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
        slice_container          _index_slice;
        mtn_index_partition_t    _partition;
        std::vector<mtn::byte_t> _bucket;
        std::vector<mtn::byte_t> _field;
        mtn_index_address_t      _value;
    };

} // namespace mtn

namespace boost
{
    // specialize range_mutable_iterator and range_const_iterator in namespace boost
    template<>
    struct range_mutable_iterator< mtn::index_slice_t >
    {
        typedef mtn::index_slice_t::iterator type;
    };

    template<>
    struct range_const_iterator< mtn::index_slice_t >
    {
        typedef mtn::index_slice_t::iterator type;
    };
} // namespace boost


#endif // __MUTTON_INDEX_SLICE_HPP_INCLUDED__
