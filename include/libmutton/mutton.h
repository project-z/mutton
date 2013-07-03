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

#ifndef __MUTTON_H_INCLUDED__
#define __MUTTON_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#if !defined _WIN32_WCE
#include <errno.h>
#endif
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/*  Handle DSO symbol visibility                                             */
#if defined _WIN32
#   if defined MUTTON_STATIC
#       define MUTTON_EXPORT
#   elif defined DLL_EXPORT
#       define MUTTON_EXPORT __declspec(dllexport)
#   else
#       define MUTTON_EXPORT __declspec(dllimport)
#   endif
#else
#   if defined __SUNPRO_C  || defined __SUNPRO_CC
#       define MUTTON_EXPORT __global
#   elif (defined __GNUC__ && __GNUC__ >= 4) || defined __INTEL_COMPILER
#       define MUTTON_EXPORT __attribute__ ((visibility("default")))
#   else
#       define MUTTON_EXPORT
#   endif
#endif

typedef __uint128_t uint128_t;
#define INDEX_ADDRESS_MAX ((uint128_t) 0xFFFFFFFFFFFFFFFF) << 64 | 0xFFFFFFFFFFFFFFFF
#define INDEX_ADDRESS_MIN ((uint128_t) 0x0000000000000000) << 64 | 0x0000000000000000

typedef uint16_t      mtn_index_partition_t;
typedef uint128_t     mtn_index_address_t;

/* The configuration options */
#define MTN_OPT_DB_PATH 1 /* the path to store the DB files */
#define MTN_OPT_LUA_PATH 2 /* the search path for lua packages */
#define MTN_OPT_LUA_CPATH 3 /* the search path for shared libraries utilized by lua  */

/* Event Processing script types */
#define MTN_SCRIPT_LUA 1

/* libmutton error codes */
#define MTN_ERROR_UNKOWN 1
#define MTN_ERROR_INDEX_OPERATION 2
#define MTN_ERROR_NOT_FOUND 3
#define MTN_ERROR_BAD_REGEX 4
#define MTN_ERROR_BAD_OPTION 4
#define MTN_ERROR_SCRIPT 5
#define MTN_ERROR_UNKOWN_EVENT_TYPE 6

/**
 * Allocate a new libmutton context.
 *
 * Note: Contexts must be freed using the supplied mutton_free_context function.
 *
 * @return allocated mutton context
 */
MUTTON_EXPORT void*
mutton_new_context();

/**
 * Free the supplied context and all of the associated resources
 *
 * @param context context to be feed
 */
MUTTON_EXPORT void
mutton_free_context(
    void* context);

/**
 * Initialize the mutton context
 *
 * @param context allocated mutton context
 * @param status output pointer to status if error is encountered, NULL otherwise. If input value of status is not NULL it will be freed prior to being set.
 *
 * @return true if successfull
 */
MUTTON_EXPORT bool
mutton_init_context(
    void*  context,
    void** status);

/**
 * Get the error code from the mutton status pointer
 *
 * @param context allocated mutton context
 * @param status opaque pointer to the mutton status
 *
 * @return underlying error code
 */
MUTTON_EXPORT int
mutton_status_get_code(
    void* context,
    void* status);

/**
 * Get the error message from the mutton status pointer
 *
 * Note: resulting error message string must be freed by the caller
 *
 * @param context allocated mutton context
 * @param status opaque pointer to the mutton status
 * @param message output pointer for the error message
 *
 */
MUTTON_EXPORT void
mutton_status_get_message(
    void*  context,
    void*  status,
    char** message);

/**
 * Free the mutton status
 *
 * @param status allocated mutton status
 */
MUTTON_EXPORT void
mutton_free_status(
    void* status);

/**
 * Set a configuration for the mutton context prior to initialization
 *
 * @param context allocated mutton context
 * @param option option to set
 * @param value option value
 * @param value_size size of option value
 * @param status output pointer to status if error is encountered, NULL otherwise. If input value of status is not NULL it will be freed prior to being set.
 *
 * @return true if successfull
 */
MUTTON_EXPORT bool
mutton_set_opt(
    void*  context,
    int    option,
    void*  value,
    size_t value_size,
    void** status);

/**
 * Index a value for the given field and row
 *
 * @param context allocated mutton context
 * @param partition partition, used to create logical seperation between indexes and other data
 * @param bucket bucket namespace for the indexed field
 * @param bucket_size size of the bucket array
 * @param field indexed field
 * @param field_size size of the field array
 * @param value the value being indexed
 * @param who_or_what ID of the row which contains the indexed value
 * @param state set the index value to true or false
 * @param status output pointer to status if error is encountered, NULL otherwise. If input value of status is not NULL it will be freed prior to being set.
 *
 * @return true if successfull
 */
MUTTON_EXPORT bool
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
    void**                status);

/**
 * Index a utf8 byte array using trigrams for the given field and row
 *
 * @param context allocated mutton context
 * @param partition partition, used to create logical seperation between indexes and other data
 * @param bucket bucket namespace for the indexed field
 * @param bucket_size size of the bucket array
 * @param field indexed field
 * @param field_size size of the field array
 * @param value the utf8 byte array being indexed, assumes rfc3629 format
 * @param value_size the size of the value byte array
 * @param who_or_what ID of the row which contains the indexed value
 * @param state set the index value to true or false
 * @param status output pointer to status if error is encountered, NULL otherwise. If input value of status is not NULL it will be freed prior to being set.
 *
 * @return true if successfull
 */
MUTTON_EXPORT bool
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
    void**                status);

/**
 * Execute a query for the supplied bucket.
 *
 * The query language is lispy in style. The most basic operation is a slice which takes a field name and an optional range/regex.
 *
 *      Supports utf8 field names: (slice "ταБЬℓσ")
 *
 *      Return all rows that have a value for field "a": (slice "a")
 *
 *      Return all rows with a field "a" value (1, 2]: (slice "a" (range 1 2))
 *
 *      Return all rows with a field "a" value (1, 2] OR (3, 4]: (slice "a" (range 1 2) (range 3 4))
 *
 *      Return all rows with a field "a" with trigram index values required of the supplied regex: (slice "a" (regex "foo.*bar"))
 *
 *      Ranges accept unsigned 128 bit values (0, 340282366920938463463374607431768211455)
 *
 *      The boolean operators 'or', 'and', 'xor', and 'not' can be used to contruct complex queries: (or (slice "a") (not (slice "b")))
 *
 * @param context allocated mutton context
 * @param partition partition, used to create logical seperation between indexes and other data
 * @param bucket bucket namespace for the indexed field
 * @param bucket_size size of the bucket array
 * @param query query string
 * @param query_size query string size
 * @param status output pointer to status if error is encountered, NULL otherwise. If input value of status is not NULL it will be freed prior to being set.
 *
 * @return true if successfull
 */
MUTTON_EXPORT bool
mutton_query(
    void*                 context,
    mtn_index_partition_t partition,
    void*                 bucket,
    size_t                bucket_size,
    void*                 query,
    size_t                query_size,
    void**                status);

/**
 * Register a script with the event proccessing system
 *
 * @param context allocated mutton context
 * @param script_type type of script being registered
 * @param event_name name of the event to associate with this script, must be unique
 * @param event_name_size event name size
 * @param buffer string buffer containing the script
 * @param buffer_size buffer size
 * @param status output pointer to status if error is encountered, NULL otherwise. If input value of status is not NULL it will be freed prior to being set.
 *
 * @return true if successfull
 */
MUTTON_EXPORT bool
mutton_register_script(
    void*  context,
    int    script_type,
    void*  event_name,
    size_t event_name_size,
    void*  buffer,
    size_t buffer_size,
    void** status);

/**
 * Register a script with the event proccessing system
 *
 * @param context allocated mutton context
 * @param script_type type of script being registered
 * @param event_name name of the event to associate with this script, must be unique
 * @param event_name_size event name size
 * @param path path to the Lua script
 * @param path_size path size
 * @param status output pointer to status if error is encountered, NULL otherwise. If input value of status is not NULL it will be freed prior to being set.
 *
 * @return true if successfull
 */
MUTTON_EXPORT bool
mutton_register_script_path(
    void*  context,
    int    script_type,
    void*  event_name,
    size_t event_name_size,
    void*  path,
    size_t path_size,
    void** status);

/**
 * Proccess an event
 *
 * Note: Should only be used for event scripts which extract the bucket ID
 *
 * @param context allocated mutton context
 * @param partition partition, used to create logical seperation between indexes and other data
 * @param event_name name of the event to associate with this script, must be unique
 * @param event_name_size event name size
 * @param buffer event data
 * @param buffer_size event data size
 * @param status output pointer to status if error is encountered, NULL otherwise. If input value of status is not NULL it will be freed prior to being set.
 *
 * @return true if successfull
 */
MUTTON_EXPORT bool
mutton_process_event(
    void*                 context,
    mtn_index_partition_t partition,
    void*                 event_name,
    size_t                event_name_size,
    void*                 buffer,
    size_t                buffer_size,
    void**                status);

/**
 * Proccess an event
 *
 * @param context allocated mutton context
 * @param partition partition, used to create logical seperation between indexes and other data
 * @param bucket bucket namespace for the event
 * @param bucket_size size of the bucket array
 * @param event_name name of the event to associate with this script, must be unique
 * @param event_name_size event name size
 * @param buffer event data
 * @param buffer_size event data size
 * @param status output pointer to status if error is encountered, NULL otherwise. If input value of status is not NULL it will be freed prior to being set.
 *
 * @return true if successfull
 */
MUTTON_EXPORT bool
mutton_process_event_bucketed(
    void*                 context,
    mtn_index_partition_t partition,
    void*                 bucket,
    size_t                bucket_size,
    void*                 event_name,
    size_t                event_name_size,
    void*                 buffer,
    size_t                buffer_size,
    void**                status);


#undef MUTTON_EXPORT

#ifdef __cplusplus
}
#endif

#endif
