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
#include "index_reader_writer_leveldb.hpp"
#include "libmutton/mutton.h"

#define CHECK_NULL(__param__, __outstatus__) if (!__param__) { *__outstatus__ = new mtn::status_t(MTN_ERROR_BAD_PARAM, "null parameter"); return false; }
#define CHECK_SIZE(__size__, __outstatus__) if (__size__ > MTN_MAX_STRING_SIZE) { *__outstatus__ = new mtn::status_t(MTN_ERROR_BAD_PARAM, "string size greater than the allowed maximum"); return false; }
#define CHECK_STRING(__param__, __size__, __outstatus__) CHECK_NULL(__param__, __outstatus__); CHECK_SIZE(__size__, __outstatus__);


inline static bool
set_error(void** statusptr,
          const mtn::status_t& status)
{
    assert(statusptr != NULL);
    if (status) {
        return true;
    }
    else if (*statusptr == NULL) {
        *statusptr = new mtn::status_t(status);
    }
    else {
        delete static_cast<mtn::status_t*>(*statusptr);
        *statusptr = new mtn::status_t(status);
    }
    return false;
}

void*
mutton_new_context()
{
    return new mtn::context_t(new mtn::index_reader_writer_leveldb_t());
}

void
mutton_free_context(
    void* context)
{
    delete static_cast<mtn::context_t*>(context);
}

bool
mutton_init_context(
    void*  context,
    void** status)
{
    CHECK_NULL(context, status);
    return set_error(status, static_cast<mtn::context_t*>(context)->init());
}

int
mutton_status_get_code(
    void*,
    void* status)
{
    if (!status) {
        return 0;
    }
    return static_cast<mtn::status_t*>(status)->code;
}

void
mutton_status_get_message(
    void*,
    void*  status,
    char** message)
{
    if (status) {
        *message = strdup(static_cast<mtn::status_t*>(status)->message.c_str());
    }
}

void
mutton_free_status(
    void* status)
{
    delete static_cast<mtn::status_t*>(status);
}

bool
mutton_set_opt(
    void*  context,
    int    option,
    void*  value,
    size_t value_size,
    void** status)
{
    CHECK_STRING(value, value_size, status);
    return set_error(status, static_cast<mtn::context_t*>(context)->set_opt(option, value, value_size));
}

bool
mutton_index_value(
    void*                 context,
    mtn_index_partition_t partition,
    void*                 bucket,
    size_t                bucket_size,
    void*                 field,
    size_t                field_size,
    mtn_index_address_t   value,
    mtn_index_address_t   who_or_what,
    bool                  state,
    void**                status)
{
    CHECK_NULL(context, status);
    CHECK_STRING(bucket, bucket_size, status);
    CHECK_STRING(field, field_size, status);

    return set_error(status,
                     static_cast<mtn::context_t*>(context)
                     ->index_value(partition,
                                   static_cast<unsigned char*>(bucket),
                                   static_cast<unsigned char*>(bucket) + bucket_size,
                                   static_cast<unsigned char*>(field),
                                   static_cast<unsigned char*>(field) + field_size,
                                   value,
                                   who_or_what,
                                   state));
}

bool
mutton_index_value_trigram(
    void*                 context,
    mtn_index_partition_t partition,
    void*                 bucket,
    size_t                bucket_size,
    void*                 field,
    size_t                field_size,
    void*                 value,
    size_t                value_size,
    mtn_index_address_t   who_or_what,
    bool                  state,
    void**                status)
{
    CHECK_NULL(context, status);
    CHECK_STRING(bucket, bucket_size, status);
    CHECK_STRING(field, field_size, status);
    CHECK_STRING(value, value_size, status);

    return set_error(status,
                     static_cast<mtn::context_t*>(context)
                     ->index_value_trigram(partition,
                                           static_cast<unsigned char*>(bucket),
                                           static_cast<unsigned char*>(bucket) + bucket_size,
                                           static_cast<unsigned char*>(field),
                                           static_cast<unsigned char*>(field) + field_size,
                                           static_cast<unsigned char*>(value),
                                           static_cast<unsigned char*>(value) + value_size,
                                           who_or_what,
                                           state));
}

bool
mutton_query(
    void*                 context,
    mtn_index_partition_t partition,
    void*                 bucket,
    size_t                bucket_size,
    void*                 query,
    size_t                query_size,
    void**                status)
{
    CHECK_NULL(context, status);
    CHECK_STRING(bucket, bucket_size, status);
    CHECK_STRING(query, query_size, status);

    (void)context;
    (void)partition;
    (void)bucket;
    (void)bucket_size;
    (void)query;
    (void)query_size;
    (void)status;

    // XXX TODO TROLOLOL haven't implemented.
    return false;
}

bool
mutton_register_script(
    void*  context,
    int    ,
    void*  event_name,
    size_t event_name_size,
    void*  buffer,
    size_t buffer_size,
    void** status)
{
    CHECK_NULL(context, status);
    CHECK_STRING(event_name, event_name_size, status);
    CHECK_STRING(buffer, buffer_size, status);

    return set_error(status,
                     lua_register_script(
                         *static_cast<mtn::context_t*>(context),
                         static_cast<char*>(event_name),
                         event_name_size,
                         static_cast<char*>(buffer),
                         buffer_size));
}

bool
mutton_register_script_path(
    void*  context,
    int    ,
    void*  event_name,
    size_t event_name_size,
    void*  path,
    size_t path_size,
    void** status)
{
    CHECK_NULL(context, status);
    CHECK_STRING(event_name, event_name_size, status);
    CHECK_STRING(path, path_size, status);

    return set_error(status,
                     lua_register_script_path(
                         *static_cast<mtn::context_t*>(context),
                         static_cast<char*>(event_name),
                         event_name_size,
                         static_cast<char*>(path),
                         path_size));
}

bool
mutton_process_event(
    void*                 context,
    mtn_index_partition_t partition,
    void*                 event_name,
    size_t                event_name_size,
    void*                 buffer,
    size_t                buffer_size,
    void**                status)
{
    CHECK_NULL(context, status);
    CHECK_STRING(event_name, event_name_size, status);
    CHECK_STRING(buffer, buffer_size, status);

    return set_error(status,
                     lua_process_event(
                         *static_cast<mtn::context_t*>(context),
                         partition,
                         static_cast<char*>(event_name),
                         event_name_size,
                         static_cast<char*>(buffer),
                         buffer_size));
}

bool
mutton_process_event_bucketed(
    void*                 context,
    mtn_index_partition_t partition,
    void*                 bucket,
    size_t                bucket_size,
    void*                 event_name,
    size_t                event_name_size,
    void*                 buffer,
    size_t                buffer_size,
    void**                status)
{
    CHECK_NULL(context, status);
    CHECK_STRING(bucket, bucket_size, status);
    CHECK_STRING(event_name, event_name_size, status);
    CHECK_STRING(buffer, buffer_size, status);

    return set_error(status,
                     lua_process_event(
                         *static_cast<mtn::context_t*>(context),
                         partition,
                         static_cast<char*>(bucket),
                         bucket_size,
                         static_cast<char*>(event_name),
                         event_name_size,
                         static_cast<char*>(buffer),
                         buffer_size));
}


bool
mutton_persistence_query(
    void*                 context,
    void*                 query,
    size_t                query_size,
    void**                future,
    void**                status)

{
    CHECK_NULL(context, status);
    CHECK_STRING(query, query_size, status);

    (void) context;
    (void) query;
    (void) query_size;
    (void) future;
    (void) status;
    return true;



    // cql::cql_client_pool_t* pool;
    // if (static_cast<mtn::context_t*>(context)->cql_pool(&pool)) {
    //     std::string cppstring(static_cast<char*>(query), query_size);
    //     *future = new boost::shared_future<cql::cql_future_connection_t>(pool->query(cppstring, cql::CQL_CONSISTENCY_ONE));
    // }

    // *statusptr = new mtn::status_t(MTN_ERROR_BAD_OPTION, "client pool is uninitialized");
    // return false;
}
