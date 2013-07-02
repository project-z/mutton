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

#include "context.hpp"
#include "lua.hpp"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

static int
lua_mutton_index_value(lua_State* lua_state)
{
    size_t size = 0;
    mtn::context_t* context = static_cast<mtn::context_t*>(lua_touserdata(lua_state, 1));

	mtn_index_partition_t partition = luaL_checkint(lua_state, 2);

    size_t bucket_size = 0;
    const char* bucket = lua_tolstring(lua_state, 3, &bucket_size);

    size_t field_size = 0;
    const char* field = lua_tolstring(lua_state, 4, &field_size);

    size = lua_objlen(lua_state, 5);
    if (size != sizeof(mtn_index_address_t)) {
        return -1;
    }
    mtn_index_address_t value = *static_cast<mtn_index_address_t*>(lua_touserdata(lua_state, 5));

    size = lua_objlen(lua_state, 6);
    if (size != sizeof(mtn_index_address_t)) {
        return -1;
    }
    mtn_index_address_t who_or_what = *static_cast<mtn_index_address_t*>(lua_touserdata(lua_state, 6));

    bool state = lua_toboolean(lua_state, 7);

    context->index_value(partition,
                         bucket,
                         bucket + bucket_size,
                         field,
                         field + field_size,
                         value,
                         who_or_what,
                         state);
    return 0;
}

static int
lua_mutton_index_value_trigram(lua_State* lua_state)
{
    size_t size = 0;
    mtn::context_t* context = static_cast<mtn::context_t*>(lua_touserdata(lua_state, 1));

	mtn_index_partition_t partition = luaL_checkint(lua_state, 2);

    size_t bucket_size = 0;
    const char* bucket = lua_tolstring(lua_state, 3, &bucket_size);

    size_t field_size = 0;
    const char* field = lua_tolstring(lua_state, 4, &field_size);

    size_t value_size = 0;
    const char* value = lua_tolstring(lua_state, 4, &value_size);

    size = lua_objlen(lua_state, 6);
    if (size != sizeof(mtn_index_address_t)) {
        return -1;
    }
    mtn_index_address_t who_or_what = *static_cast<mtn_index_address_t*>(lua_touserdata(lua_state, 6));

    bool state = lua_toboolean(lua_state, 7);

    context->index_value_trigram(partition,
                                 bucket,
                                 bucket + bucket_size,
                                 field,
                                 field + field_size,
                                 value,
                                 value + value_size,
                                 who_or_what,
                                 state);
    return 0;
}

inline void
register_functions(
    mtn::lua_state_t lua_state)
{
    lua_register(lua_state.get(), "mutton_index_value", lua_mutton_index_value);
    lua_register(lua_state.get(), "mutton_index_value_trigram", lua_mutton_index_value_trigram);
}

inline mtn::status_t
process_event(
    mtn::lua_state_t      lua_state,
    mtn::context_t&       context,
    mtn_index_partition_t partition,
    const char*           bucket,
    size_t                bucket_size,
    const char*           buffer,
    size_t                buffer_size)
{
    assert(lua_state.get());

    // create a new table
    lua_newtable(lua_state.get());

    lua_pushstring(lua_state.get(), "context");
    lua_pushlightuserdata(lua_state.get(), &context);
    lua_rawset(lua_state.get(), -3);

    lua_pushstring(lua_state.get(), "partition");
    lua_pushnumber(lua_state.get(), partition);
    lua_rawset(lua_state.get(), -3);

    lua_pushstring(lua_state.get(), "bucket");
    lua_pushlstring(lua_state.get(), bucket, bucket_size);
    lua_rawset(lua_state.get(), -3);

    lua_pushstring(lua_state.get(), "event_data");
    lua_pushlstring(lua_state.get(), buffer, buffer_size);
    lua_rawset(lua_state.get(), -3);

    lua_setglobal(lua_state.get(), "mutton");

    // Run the fucker
    if (lua_pcall(lua_state.get(), 0, LUA_MULTRET, 0)) {
        std::cout << "Failed to run script: " << lua_tostring(lua_state.get(), -1) << std::endl;
    }

    return mtn::status_t();
}

mtn::status_t
mtn::lua_register_script(
    mtn::context_t& context,
    const char*     event_name,
    size_t          event_name_size,
    const char*     buffer,
    size_t          buffer_size)
{
    lua_state_t lua_state(luaL_newstate(), lua_free_state);
    assert(lua_state.get());

    luaL_openlibs(lua_state.get());
    int load_status = luaL_loadbuffer(lua_state.get(), buffer, buffer_size, event_name);

    if (load_status) {
        mtn::status_t output(MTN_ERROR_SCRIPT, "Couldn't load lua buffer: ");
        output.message.append(lua_tostring(lua_state.get(), -1));
        return output;
    }

    register_functions(lua_state);
    context.register_lua_script(event_name, event_name_size, lua_state);
    return mtn::status_t();
}

mtn::status_t
mtn::lua_register_script_path(
    mtn::context_t& context,
    const char*     event_name,
    size_t          event_name_size,
    const char*     path,
    size_t)
{
    lua_state_t lua_state(luaL_newstate(), lua_free_state);
    luaL_openlibs(lua_state.get());
    int load_status = luaL_loadfile(lua_state.get(), path);

    if (load_status) {
        mtn::status_t output(MTN_ERROR_SCRIPT, "Couldn't load lua file: ");
        output.message.append(lua_tostring(lua_state.get(), -1));
        return output;
    }

    register_functions(lua_state);
    context.register_lua_script(event_name, event_name_size, lua_state);
    return mtn::status_t();
}


mtn::status_t
mtn::lua_register_script(
    mtn::context_t&    context,
    const std::string& event_name,
    const std::string& buffer)
{
    return mtn::lua_register_script(context, event_name.c_str(), event_name.size(), buffer.c_str(), buffer.size());
}


mtn::status_t
mtn::lua_register_script_path(
    mtn::context_t&    context,
    const std::string& event_name,
    const std::string& path)
{
    return mtn::lua_register_script(context, event_name.c_str(), event_name.size(), path.c_str(), path.size());
}

mtn::status_t
mtn::lua_process_event(
    mtn::context_t&       context,
    mtn_index_partition_t partition,
    const char*           event_name,
    size_t                event_name_size,
    const char*           buffer,
    size_t                buffer_size)
{
    lua_state_t lua_state;
    if (context.get_lua_script(event_name, event_name_size, lua_state)) {
        return process_event(lua_state, context, partition, "", 0, buffer, buffer_size);
    }
    else {
        std::string message("no script registered for event type: ");
        message.append(event_name, event_name_size);
        return mtn::status_t(MTN_ERROR_UNKOWN_EVENT_TYPE, message);
    }
}

mtn::status_t
mtn::lua_process_event(
    mtn::context_t&       context,
    mtn_index_partition_t partition,
    const char*           bucket,
    size_t                bucket_size,
    const char*           event_name,
    size_t                event_name_size,
    const char*           buffer,
    size_t                buffer_size)
{
    lua_state_t lua_state;
    if (context.get_lua_script(event_name, event_name_size, lua_state)) {
        return process_event(lua_state, context, partition, bucket, bucket_size, buffer, buffer_size);
    }
    else {
        std::string message("no script registered for event type: ");
        message.append(event_name, event_name_size);
        return mtn::status_t(MTN_ERROR_UNKOWN_EVENT_TYPE, message);
    }
}

mtn::status_t
mtn::lua_process_event(
    mtn::context_t&       context,
    mtn_index_partition_t partition,
    const std::string&    event_name,
    const std::string&    buffer)
{
    lua_state_t lua_state;
    if (context.get_lua_script(event_name, lua_state)) {
        assert(lua_state.get());
        return process_event(lua_state, context, partition, "", 0, buffer.c_str(), buffer.size());
    }
    else {
        return mtn::status_t(MTN_ERROR_UNKOWN_EVENT_TYPE, std::string("no script registered for event type: ") + event_name);
    }
}

mtn::status_t
mtn::lua_process_event(
    mtn::context_t&           context,
    mtn_index_partition_t     partition,
    std::vector<mtn::byte_t>& bucket,
    const std::string&        event_name,
    const std::string&        buffer)
{
    return mtn::lua_process_event(context, partition, (const char*)&bucket[0], bucket.size(), event_name.c_str(), event_name.size(), buffer.c_str(), buffer.size());
}


void
mtn::lua_free_state(
    lua_State* lua)
{
    lua_close(lua);
}
