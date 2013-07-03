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

#ifndef __MUTTON_CONTEXT_HPP_INCLUDED__
#define __MUTTON_CONTEXT_HPP_INCLUDED__

#include <boost/ptr_container/ptr_map.hpp>
#include <boost/unordered_map.hpp>

#include "base_types.hpp"
#include "index.hpp"
#include "index_reader_writer.hpp"
#include "status.hpp"
#include "lua.hpp"

namespace mtn {

    class context_t {
    public:

        typedef std::vector<mtn::byte_t>                       index_key_t;
        typedef boost::ptr_map<index_key_t, mtn::index_t>      index_container_t;
        typedef std::map<int, std::vector<mtn::byte_t> >       options_container_t;
        typedef boost::unordered_map<std::string, lua_state_t> lua_state_container_t;

        context_t(mtn::index_reader_writer_t* rw) :
            _rw(rw)
        {}

        inline mtn::status_t
        set_opt(int         option,
                const void* value,
                size_t      value_size)
        {
            const mtn::byte_t* byte_value = static_cast<const mtn::byte_t*>(value);
            _options[option] = std::vector<mtn::byte_t>(byte_value, byte_value + value_size);
            return mtn::status_t();
        }

        bool
        get_opt(int          option,
                std::string& output)
        {
            mtn::context_t::options_container_t::const_iterator iter = _options.find(option);
            if (iter == _options.end()) {
                return false;
            }

            output.assign(iter->second.begin(), iter->second.end());
            return true;
        }

        inline mtn::status_t
        init()
        {
            return _rw->init(*this);
        }

        inline mtn::index_reader_writer_t&
        index_reader_writer()
        {
            return *_rw;
        }

        inline mtn::status_t
        get_index(mtn_index_partition_t,
                  const std::vector<mtn::byte_t>& bucket,
                  const std::vector<mtn::byte_t>& field,
                  mtn::index_t**                  output)
        {
            index_key_t key;
            key.reserve(bucket.size() + field.size());
            key.insert(key.end(), bucket.begin(), bucket.end());
            key.insert(key.end(), field.begin(), field.end());

            mtn::status_t status;
            index_container_t::iterator iter = _indexes.find(key);
            if (iter != _indexes.end()) {
                *output = iter->second;
                return status;
            }

            status.library = true;
            status.code = MTN_ERROR_NOT_FOUND;
            status.message = "index not found";
            return status;
        }

        inline mtn::status_t
        create_index(mtn_index_partition_t           partition,
                     const std::vector<mtn::byte_t>& bucket,
                     const std::vector<mtn::byte_t>& field,
                     mtn::index_t**                  output)
        {
            return create_index(partition, bucket.begin(), bucket.end(), field.begin(), field.end(), output);
        }

        template<class BucketIterator, class FieldIterator>
        inline mtn::status_t
        create_index(mtn_index_partition_t partition,
                     BucketIterator        bucket_begin,
                     BucketIterator        bucket_end,
                     FieldIterator         field_begin,
                     FieldIterator         field_end,
                     mtn::index_t**        output)
        {
            index_key_t key;
            key.reserve(std::distance(bucket_begin, bucket_end) + std::distance(field_begin, field_end));
            key.insert(key.end(), bucket_begin, bucket_end);
            key.insert(key.end(), field_begin, field_end);

            mtn::status_t status;
            index_container_t::iterator iter = _indexes.find(key);
            if (iter != _indexes.end()) {
                if (output) {
                    *output = iter->second;
                }
                return status;
            }

            std::pair<index_container_t::iterator, bool> insert_result
                = _indexes.insert(key, new mtn::index_t(partition, bucket_begin, bucket_end, field_begin, field_end));

            if (insert_result.second) {
                if (output) {
                    *output = insert_result.first->second;
                }
                return status;
            }

            status.library = true;
            status.code = MTN_ERROR_UNKOWN;
            status.message = "could not create new index";
            return status;
        }

        inline mtn::status_t
        index_value(mtn_index_partition_t           partition,
                    const std::vector<mtn::byte_t>& bucket,
                    const std::vector<mtn::byte_t>& field,
                    mtn_index_address_t             value,
                    mtn_index_address_t             who_or_what,
                    bool                            state)
        {
            return index_value(partition, bucket.begin(), bucket.end(), field.begin(), field.end(), value, who_or_what, state);
        }

        template<class BucketIterator, class FieldIterator>
        inline mtn::status_t
        index_value(mtn_index_partition_t partition,
                    BucketIterator        bucket_begin,
                    BucketIterator        bucket_end,
                    FieldIterator         field_begin,
                    FieldIterator         field_end,
                    mtn_index_address_t   value,
                    mtn_index_address_t   who_or_what,
                    bool                  state)
        {
            mtn::index_t* index = NULL;
            mtn::status_t create_status = create_index(partition, bucket_begin, bucket_end, field_begin, field_end, &index);
            if (create_status && index) {
                return index->index_value(*_rw, value, who_or_what, state);
            }
            return create_status;
        }

        template<class ValueIterator>
        inline mtn::status_t
        index_value_trigram(mtn_index_partition_t           partition,
                            const std::vector<mtn::byte_t>& bucket,
                            const std::vector<mtn::byte_t>& field,
                            ValueIterator                   first,
                            ValueIterator                   last,
                            mtn_index_address_t             who_or_what,
                            bool                            state)
        {
            return index_value_trigram(partition, bucket.begin(), bucket.end(), field.begin(), field.end(), first, last, who_or_what, state);
        }

        template<class BucketIterator, class FieldIterator, class ValueIterator>
        inline mtn::status_t
        index_value_trigram(mtn_index_partition_t partition,
                            BucketIterator        bucket_begin,
                            BucketIterator        bucket_end,
                            FieldIterator         field_begin,
                            FieldIterator         field_end,
                            ValueIterator         first,
                            ValueIterator         last,
                            mtn_index_address_t   who_or_what,
                            bool                  state)
        {
            mtn::index_t* index = NULL;
            mtn::status_t create_status = create_index(partition, bucket_begin, bucket_end, field_begin, field_end, &index);
            if (create_status && index) {
                return index->index_value_trigram(*_rw, first, last, who_or_what, state);
            }
            return create_status;
        }

        inline void
        register_lua_script(const char* event_name,
                            size_t      event_name_size,
                            lua_state_t lua_state)
        {
            assert(lua_state.get());
            _lua_state.insert(lua_state_container_t::value_type(std::string(event_name, event_name_size), lua_state));
        }

        inline void
        register_lua_script(const std::string& event_name,
                            lua_state_t        lua_state)
        {
            assert(lua_state.get());
            _lua_state.insert(lua_state_container_t::value_type(event_name, lua_state));
        }

        inline bool
        get_lua_script(const std::string& event_name,
                       lua_state_t&       output)
        {
            lua_state_container_t::iterator iter = _lua_state.find(event_name);
            if (iter != _lua_state.end()) {
                assert(iter->second.get());
                output = iter->second;
                assert(output.get());
                return true;
            }
            return false;
        }

        inline bool
        get_lua_script(const char*  event_name,
                       size_t       event_name_size,
                       lua_state_t& output)
        {
            return get_lua_script(std::string(event_name, event_name_size), output);
        }

    private:
        std::auto_ptr<mtn::index_reader_writer_t> _rw;
        lua_state_container_t                     _lua_state;
        index_container_t                         _indexes;
        options_container_t                       _options;
    };

} // namespace mtn

#endif // __MUTTON_CONTEXT_HPP_INCLUDED__
