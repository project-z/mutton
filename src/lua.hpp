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

#ifndef __MUTTON_LUA_HPP_INCLUDED__
#define __MUTTON_LUA_HPP_INCLUDED__

#include <vector>
#include <boost/shared_ptr.hpp>
#include "libmutton/mutton.h"
#include "status.hpp"
#include "base_types.hpp"

class lua_State;

namespace mtn {

    class context_t;

    typedef boost::shared_ptr<lua_State> lua_state_t;

    mtn::status_t
    lua_register_script(
        mtn::context_t& context,
        const char*     event_name,
        size_t          event_name_size,
        const char*     buffer,
        size_t          buffer_size);

    mtn::status_t
    lua_register_script(
        mtn::context_t&    context,
        const std::string& event_name,
        const std::string& buffer);

    mtn::status_t
    lua_register_script_path(
        mtn::context_t& context,
        const char*     event_name,
        size_t          event_name_size,
        const char*     path,
        size_t          path_size);

    mtn::status_t
    lua_register_script_path(
        mtn::context_t&    context,
        const std::string& event_name,
        const std::string& path);

    mtn::status_t
    lua_process_event(
        mtn::context_t&       context,
        mtn_index_partition_t partition,
        const char*           event_name,
        size_t                event_name_size,
        const char*           buffer,
        size_t                buffer_size);

    mtn::status_t
    lua_process_event(
        mtn::context_t&       context,
        mtn_index_partition_t partition,
        const char*           bucket,
        size_t                bucket_size,
        const char*           event_name,
        size_t                event_name_size,
        const char*           buffer,
        size_t                buffer_size);

    mtn::status_t
    lua_process_event(
        mtn::context_t&       context,
        mtn_index_partition_t partition,
        const std::string&    event_name,
        const std::string&    buffer);

    mtn::status_t
    lua_process_event(
        mtn::context_t&           context,
        mtn_index_partition_t     partition,
        std::vector<mtn::byte_t>& bucket,
        const std::string&        event_name,
        const std::string&        buffer);

    void
    lua_free_state(lua_State* lua);



} // namespace mtn

#endif // __MUTTON_LUA_HPP_INCLUDED__
