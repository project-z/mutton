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

#ifndef __X_INDEX_HPP_INCLUDED__
#define __X_INDEX_HPP_INCLUDED__

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_list.hpp>

#define SEGMENT_LENGTH 32
#define SEGMENT_SIZE SEGMENT_LENGTH * sizeof(uint64_t)

class leveldb::Snapshot;

namespace prz {

    enum index_operation_enum {
        INDEX_INTERSECTION = 0,
        INDEX_UNION = 1,
    };

    class index : boost::noncopyable
    {
    public:
        typedef uint64_t segment_t[SEGMENT_LENGTH];

        struct index_node_t {
            uint64_t offset;
            segment_t segment;

            index_node_t(const index_node_t& node);

            index_node_t(uint64_t    offset);

            index_node_t(uint64_t    offset,
                         const char* data);
        };

        typedef boost::ptr_list<index_node_t> index_container;

        class iterator :
            public boost::iterator_adaptor<iterator,
                                           index_container::iterator,
                                           index_node_t&,
                                           boost::bidirectional_traversal_tag,
                                           index_node_t>
        {
        public:
            iterator()
                : iterator::iterator_adaptor_() {}

            explicit
            iterator(const iterator::iterator_adaptor_::base_type& p)
                : iterator::iterator_adaptor_(p) {}

        private:
            friend class boost::iterator_core_access;
            struct enabler {};  // a private type avoids misuse
        };

        index(leveldb::DB*          snapshot,
              leveldb::ReadOptions* options,
              uint8_t               partition,
              const char*           field,
              size_t                field_size,
              uint64_t              value);

        inline iterator
        begin()
        {
            return iterator(_index.begin());
        }

        inline iterator
        end()
        {
            return iterator(_index.end());
        }

        inline iterator
        insert(iterator      pos,
               index_node_t* value)
        {
            return iterator(_index.insert(pos.base(), value));
        }

        inline void
        clear()
        {
            _index.clear();
        }

        static bool
        execute(index_operation_enum  operation,
                index&                a_index,
                index&                b_index,
                index&                output);

        void
        execute(leveldb::DB*          db,
                leveldb::ReadOptions* options,
                index_operation_enum  operation,
                uint8_t               partition,
                const char*           field,
                size_t                field_size,
                uint64_t              value,
                index&                output);

        void
        execute(leveldb::DB*          db,
                leveldb::ReadOptions* options,
                index_operation_enum  operation,
                uint8_t               partition,
                const char*           field,
                size_t                field_size,
                uint64_t              value);

        void
        bit(leveldb::DB*           db,
            leveldb::WriteOptions* options,
            uint64_t               bit,
            bool                   state);

        bool
        bit(uint64_t bit);

        uint8_t
        partition() const;

        const char*
        field() const;

        size_t
        field_size() const;

        uint64_t
        value() const;

        static uint64_t
        estimateSize(leveldb::DB* db,
                     uint8_t      partition,
                     const char*  field,
                     size_t       field_size,
                     uint64_t     value);

    private:
        index_container::iterator
        find_insertion_point(uint64_t bucket);

        index_container   _index;
        uint8_t           _partition;
        std::vector<char> _field;
        uint64_t          _value;
    };

} // namespace prz


#endif // __X_INDEX_HPP_INCLUDED__
