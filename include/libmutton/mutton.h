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

//  Socket options
#define MTN_DB_PATH 1

MUTTON_EXPORT void*
mutton_context();

MUTTON_EXPORT void*
mutton_set_opt(
    void*  context,
    int    option,
    void*  value,
    size_t value_size);

MUTTON_EXPORT void*
mutton_init(
    void* context);

MUTTON_EXPORT void*
mutton_index_value(
    void*                 context,
    mtn_index_partition_t partition,
    void*                 bucket,
    size_t                bucket_size,
    void*                 field,
    size_t                field_size,
    mtn_index_address_t   value,
    mtn_index_address_t   who_or_what,
    bool                  state);

MUTTON_EXPORT void*
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
    bool                  state);

MUTTON_EXPORT void*
mutton_query(
    void*                 context,
    mtn_index_partition_t partition,
    void*                 bucket,
    size_t                bucket_size,
    void*                 query,
    size_t                query_size);

MUTTON_EXPORT void*
mutton_register_script(
    void*                 context,
    void*                 event_name,
    size_t                event_name_size,
    void*                 buffer,
    size_t                buffer_size);

MUTTON_EXPORT void*
mutton_register_script_path(
    void*                 context,
    void*                 event_name,
    size_t                event_name_size,
    void*                 path,
    size_t                path_size);

MUTTON_EXPORT void*
mutton_process_event(
    void*                 context,
    void*                 event_name,
    size_t                event_name_size,
    void*                 buffer,
    size_t                buffer_size);


#undef MUTTON_EXPORT

#ifdef __cplusplus
}
#endif

#endif
