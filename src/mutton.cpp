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
#include "index_reader_writer_leveldb.hpp"
#include "libmutton/mutton.h"

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
mutton_init(
    void*  context,
    void** status)
{
    return set_error(status, static_cast<mtn::context_t*>(context)->init());
}

int
mutton_status_get_code(
    void*,
    void* status)
{
    return static_cast<mtn::status_t*>(status)->code;
}

void
mutton_status_get_message(
    void*,
    void*  status,
    char** message)
{
    *message = strdup(static_cast<mtn::status_t*>(status)->message.c_str());
}

void
mutton_free_status(
    void* status)
{
    delete static_cast<mtn::status_t*>(status);
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
    void*  event_name,
    size_t event_name_size,
    void*  buffer,
    size_t buffer_size,
    void** status)
{
    (void)context;
    (void)event_name;
    (void)event_name_size;
    (void)buffer;
    (void)buffer_size;
    (void)status;

    // XXX TODO TROLOLOL haven't implemented.
    return false;
}

bool
mutton_register_script_path(
    void*  context,
    void*  event_name,
    size_t event_name_size,
    void*  path,
    size_t path_size,
    void** status)
{
    (void)context;
    (void)event_name;
    (void)event_name_size;
    (void)path;
    (void)path_size;
    (void)status;

    // XXX TODO TROLOLOL haven't implemented.
    return false;
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
    (void)context;
    (void)partition;
    (void)event_name;
    (void)event_name_size;
    (void)buffer;
    (void)buffer_size;
    (void)status;

    // XXX TODO TROLOLOL haven't implemented.
    return false;
}

bool
mutton_process_event_bucketed(
    void*                 context,
    mtn_index_partition_t partition,
    void*                 event_name,
    size_t                event_name_size,
    void*                 bucket,
    size_t                bucket_size,
    void*                 buffer,
    size_t                buffer_size,
    void**                status)
{
    (void)context;
    (void)partition;
    (void)event_name;
    (void)event_name_size;
    (void)bucket;
    (void)bucket_size;
    (void)buffer;
    (void)buffer_size;
    (void)status;

    // XXX TODO TROLOLOL haven't implemented.
    return false;
}
