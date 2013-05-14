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

#ifndef __MUTTON_INDEX_HPP_INCLUDED__
#define __MUTTON_INDEX_HPP_INCLUDED__

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>

#include "base_types.hpp"
#include "index_slice.hpp"
#include "status.hpp"

namespace mtn {

    class index_reader_t;
    class index_writer_t;
    struct range_t;

    class index_t
        : boost::noncopyable
    {
    public:
        typedef mtn::index_slice_t type;
        typedef boost::ptr_map<mtn::index_address_t, mtn::index_slice_t> index_container;
        typedef index_container::iterator iterator;

        index_t(mtn::index_partition_t partition,
                const char*            field,
                size_t                 field_size);

        mtn::status_t
        slice(mtn::index_slice_t&       output);

        mtn::status_t
        slice(mtn::range_t*             ranges,
              size_t                    range_count,
              mtn::index_slice_t&       output);

        mtn::status_t
        slice(mtn::range_t*             ranges,
              size_t                    range_count,
              mtn::index_operation_enum operation,
              mtn::index_slice_t&       output);

        mtn::status_t
        slice(mtn::index_address_t*     trigrams,
              size_t                    trigrams_count,
              mtn::index_operation_enum operation,
              mtn::index_slice_t&       output);

        mtn::status_t
        index_value(mtn::index_reader_t* reader,
                    mtn::index_writer_t* writer,
                    mtn::index_address_t value,
                    mtn::index_address_t who_or_what,
                    bool                 state);

        mtn::status_t
        index_value_trigram(mtn::index_reader_t* reader,
                            mtn::index_writer_t* writer,
                            const char*          value,
                            const char*          end,
                            mtn::index_address_t who_or_what,
                            bool                 state);

        mtn::status_t
        index_value_hash(mtn::index_reader_t* reader,
                         mtn::index_writer_t* writer,
                         const char*          value,
                         size_t               len,
                         mtn::index_address_t who_or_what,
                         bool                 state);

        mtn::status_t
        indexed_value(mtn::index_reader_t* reader,
                      mtn::index_writer_t* writer,
                      mtn::index_address_t value,
                      mtn::index_address_t who_or_what,
                      bool*                state);

        mtn::status_t
        indexed_value(mtn::index_reader_t* reader,
                      mtn::index_writer_t* writer,
                      mtn::index_address_t value,
                      mtn::index_slice_t** who_or_what);

        index_partition_t
        partition() const;

        const byte_t*
        field() const;

        size_t
        field_size() const;

        inline iterator
        find(mtn::index_address_t a)
        {
            return _index.find(a);
        }

        inline iterator
        begin()
        {
            return _index.begin();
        }

        inline iterator
        end()
        {
            return _index.end();
        }

        std::pair<iterator, bool>
        insert(mtn::index_address_t value,
               index_slice_t* slice)
        {
            return _index.insert(value, slice);
        }

        inline void
        clear()
        {
            _index.clear();
        }

        inline void
        erase(iterator first,
              iterator last)
        {
            _index.erase(first, last);
        }

        inline void
        erase(iterator position)
        {
            _index.erase(position);
        }

        inline size_t
        size()
        {
            return _index.size();
        }

    private:
        index_container     _index;
        index_partition_t   _partition;
        std::vector<byte_t> _field;
    };


} // namespace mtn

namespace boost
{
    // specialize range_mutable_iterator and range_const_iterator in namespace boost
    template<>
    struct range_mutable_iterator< mtn::index_t >
    {
        typedef mtn::index_t::iterator type;
    };

    template<>
    struct range_const_iterator< mtn::index_t >
    {
        typedef mtn::index_t::iterator type;
    };
} // namespace boost


#endif // __MUTTON_INDEX_HPP_INCLUDED__
