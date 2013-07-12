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

#include <boost/format.hpp>

#include "context.hpp"
#include "lua.hpp"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Lua wrappers for the context methods
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

int
lua_mutton_index_value(
    lua_State* L)
{
    mtn::context_t* context = static_cast<mtn::context_t*>(lua_touserdata(L, 1));

	mtn_index_partition_t partition = luaL_checkint(L, 2);

    luaL_checkany(L, 3);
    size_t bucket_size = 0;
    const char* bucket = luaL_checklstring(L, 3, &bucket_size);

    luaL_checkany(L, 4);
    size_t field_size = 0;
    const char* field = luaL_checklstring(L, 4, &field_size);

    luaL_checkany(L, 5);
    size_t value_size = 0;
    const char* value = lua_tolstring(L, 5, &value_size);
    if (value_size > sizeof(mtn_index_address_t)) {
        std::string error_msg = (boost::format("max of 16 bytes expected, got %1%") % value_size).str();
        luaL_argerror(L, 5, error_msg.c_str());
        return 0;
    }

    luaL_checkany(L, 6);
    size_t who_or_what_size = 0;
    const char* who_or_what = lua_tolstring(L, 6, &who_or_what_size);
    if (who_or_what_size > sizeof(mtn_index_address_t)) {
        std::string error_msg = (boost::format("max of 16 bytes expected, got %1%") % who_or_what_size).str();
        luaL_argerror(L, 6, error_msg.c_str());
        return 0;
    }

    luaL_checkany(L, 7);
    bool state = lua_toboolean(L, 7);

    context->index_value(partition,
                         bucket,
                         bucket + bucket_size,
                         field,
                         field + field_size,
                         *reinterpret_cast<const mtn_index_address_t*>(value),
                         *reinterpret_cast<const mtn_index_address_t*>(who_or_what),
                         state);
    return 0;
}

int
lua_mutton_index_value_trigram(
    lua_State* L)
{
    if (!lua_islightuserdata(L, 1)) {
        luaL_argerror(L, 1, "expected a mutton context");
        return 0;
    }
    mtn::context_t* context = static_cast<mtn::context_t*>(lua_touserdata(L, 1));

	mtn_index_partition_t partition = luaL_checkint(L, 2);

    luaL_checkany(L, 3);
    size_t bucket_size = 0;
    const char* bucket = luaL_checklstring(L, 3, &bucket_size);

    luaL_checkany(L, 4);
    size_t field_size = 0;
    const char* field = luaL_checklstring(L, 4, &field_size);

    luaL_checkany(L, 5);
    size_t value_size = 0;
    const char* value = lua_tolstring(L, 5, &value_size);
    if (!value) {
        luaL_argerror(L, 6, "nil value");
        return 0;
    }

    luaL_checkany(L, 6);
    size_t who_or_what_size = 0;
    const char* who_or_what = lua_tolstring(L, 6, &who_or_what_size);
    if (who_or_what_size > sizeof(mtn_index_address_t)) {
        std::string error_msg = (boost::format("max of 16 bytes expected, got %1%") % who_or_what_size).str();
        luaL_argerror(L, 6, error_msg.c_str());
        return 0;
    }

    luaL_checkany(L, 7);
    bool state = lua_toboolean(L, 7);

    mtn::status_t status
        = context->index_value_trigram(partition,
                                       bucket,
                                       bucket + bucket_size,
                                       field,
                                       field + field_size,
                                       value,
                                       value + value_size,
                                       *reinterpret_cast<const mtn_index_address_t*>(who_or_what),
                                       state);

    if (!status) {
        luaL_error(L, status.message.c_str());
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Lua helper functions
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline int
register_functions(
    lua_State       *L)
{
    lua_register(L, "mutton_index_value", lua_mutton_index_value);
    lua_register(L, "mutton_index_value_trigram", lua_mutton_index_value_trigram);

    return 0; // figure out how to check for errors
}

inline int
set_package_path(
    lua_State *L,
    const char *path)
{
    lua_getglobal(L, "package");
    lua_pushstring(L, path);
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);

    return 0; // figure out how to check for errors
}

inline int
set_package_cpath(
    lua_State *L,
    const char *path)
{
    lua_getglobal(L, "package");
    lua_pushstring(L, path);
    lua_setfield(L, -2, "cpath");
    lua_pop(L, 1);

    return 0; // figure out how to check for errors
}

inline mtn::status_t
format_lua_error(
    lua_State       *L,
    const std::string& message)

{
    return mtn::status_t(MTN_ERROR_SCRIPT, (boost::format(message) % lua_tostring(L, -1)).str());
}

inline mtn::status_t
setup_lua_env(
    mtn::context_t&  context,
    lua_State       *L)
{
    std::string path;
    if (context.get_opt(MTN_OPT_LUA_PATH, path)) {
        if (set_package_path(L, path.c_str())) {
            return format_lua_error(L, "error setting Lua package path '%1%'");
        }
    }

    std::string cpath;
    if (context.get_opt(MTN_OPT_LUA_CPATH, cpath)) {
        if (set_package_cpath(L, cpath.c_str())) {
            return format_lua_error(L, "error setting Lua library path '%1%'");
        }
    }

    if (register_functions(L)) {
        return format_lua_error(L, "error registering Lua functions '%1%'");
    }

    return mtn::status_t();
}

inline mtn::status_t
process_event(
    lua_State             *L,
    mtn::context_t&        context,
    mtn_index_partition_t  partition,
    const char*            bucket,
    size_t                 bucket_size,
    const char*            buffer,
    size_t                 buffer_size)
{
    assert(L);

    // remember the index of the top of the stack
    int stack_top = lua_gettop(L);

    // copy the script
    lua_pushvalue(L, -1);

    // create a new table
    lua_newtable(L);

    lua_pushstring(L, "context");
    lua_pushlightuserdata(L, &context);
    lua_rawset(L, -3);

    lua_pushstring(L, "partition");
    lua_pushnumber(L, partition);
    lua_rawset(L, -3);

    lua_pushstring(L, "bucket");
    lua_pushlstring(L, bucket, bucket_size);
    lua_rawset(L, -3);

    lua_pushstring(L, "event_data");
    lua_pushlstring(L, buffer, buffer_size);
    lua_rawset(L, -3);

    lua_setglobal(L, "mutton");

    // Run the fucker
    mtn::status_t status;
    if (lua_pcall(L, 0, LUA_MULTRET, 0)) {
        status = format_lua_error(L, "error in execution of Lua script '%1%'");
    }

    // set the stack index back to the remembered value to trigger GC
    lua_settop(L, stack_top);
    return status;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The public stuff
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////


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
    if (luaL_loadbuffer(lua_state.get(), buffer, buffer_size, event_name)) {
        return format_lua_error(lua_state.get(), "error loading Lua buffer '%1%'");
    }

    mtn::status_t setup_status = setup_lua_env(context, lua_state.get());
    if (!setup_status) {
        return setup_status;
    }

    context.register_lua_script(event_name, event_name_size, lua_state);
    return setup_status;
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
    assert(lua_state.get());

    luaL_openlibs(lua_state.get());
    if (luaL_loadfile(lua_state.get(), path)) {
        return format_lua_error(lua_state.get(), "error loading Lua script '%1%'");
    }

    mtn::status_t setup_status = setup_lua_env(context, lua_state.get());
    if (!setup_status) {
        return setup_status;
    }

    context.register_lua_script(event_name, event_name_size, lua_state);
    return setup_status;
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
        return process_event(lua_state.get(), context, partition, "", 0, buffer, buffer_size);
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
        return process_event(lua_state.get(), context, partition, bucket, bucket_size, buffer, buffer_size);
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
        return process_event(lua_state.get(), context, partition, "", 0, buffer.c_str(), buffer.size());
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
