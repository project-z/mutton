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

#ifndef __MUTTON_ENCODE_HPP_INCLUDED__
#define __MUTTON_ENCODE_HPP_INCLUDED__

#include <vector>
#include <machine/endian.h>
#include <stdint.h>

#include "base_types.hpp"

// Required to use stdint.h
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif // __STDC_LIMIT_MACROS

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ntohlll(x) ((((uint128_t) ntohll(x)) << 64) | ntohll(x >> 64))
#define htonlll(x) ntohlll(x)
#define ntohll(x) (uint64_t) __builtin_bswap64(x)
#define htonll(x) ntohll(x)
#else
#define ntohlll(x) (x)
#define htonlll(x) (x)
#define ntohll(x) (x)
#define htonll(x) (x)
#endif // __BYTE_ORDER == __LITTLE_ENDIAN

namespace mtn {

////////////////////////////////////////////////////////////////////////
// Decode
////////////////////////////////////////////////////////////////////////

    inline mtn::byte_t*
    decode_uint16(const mtn::byte_t* input,
                  uint16_t*          output)
    {
        *output = ntohs(*(reinterpret_cast<const uint16_t*>(input)));
        return (mtn::byte_t*) input + sizeof(uint16_t);
    }

    inline mtn::byte_t*
    decode_uint32(const mtn::byte_t* input,
                  uint32_t*          output)
    {
        *output = ntohl(*(reinterpret_cast<const uint32_t*>(input)));
        return (mtn::byte_t*) input + sizeof(uint32_t);
    }

    inline mtn::byte_t*
    decode_uint64(const mtn::byte_t* input,
                  uint64_t*          output)
    {
        *output = ntohll(*(reinterpret_cast<const uint64_t*>(input)));
        return (mtn::byte_t*) input + sizeof(uint64_t);
    }

    inline mtn::byte_t*
    decode_uint128(const mtn::byte_t* input,
                   uint128_t*         output)
    {
        *output = ntohlll(*(reinterpret_cast<const uint128_t*>(input)));
        return (mtn::byte_t*) input + sizeof(uint128_t);
    }

    inline mtn::byte_t*
    decode_parition(const mtn::byte_t* input,
                    uint16_t*          output)
    {
        return decode_uint16(input, output);
    }

    inline mtn::byte_t*
    decode_bytes(const mtn::byte_t* input,
                 mtn::byte_t**      output,
                 uint16_t*          size)
    {
        *output = decode_uint16(input, size);
        return *output + *size;
    }

    inline mtn::byte_t*
    decode_index_key(const mtn::byte_t*    input,
                     uint16_t*             partition,
                     mtn::byte_t**         bucket,
                     uint16_t*             bucket_size,
                     mtn::byte_t**         field,
                     uint16_t*             field_size,
                     mtn::index_address_t* value,
                     mtn::index_address_t* offset)
    {
        mtn::byte_t* output = decode_parition(input, partition);
        output = decode_bytes(output, bucket, bucket_size);
        output = decode_bytes(output, field, field_size);
        output = decode_uint128(output, value);
        return decode_uint128(output, offset);
    }

////////////////////////////////////////////////////////////////////////
// Encode
////////////////////////////////////////////////////////////////////////

    inline mtn::byte_t*
    encode_uint16(uint16_t     input,
                  mtn::byte_t* output)
    {
        *((uint16_t*) output) = htons(input);
        return output + sizeof(input);
    }

    inline mtn::byte_t*
    encode_uint32(uint32_t     input,
                  mtn::byte_t* output)
    {
        *((uint32_t*) output) = htonl(input);
        return output + sizeof(input);
    }

    inline mtn::byte_t*
    encode_uint64(uint64_t     input,
                  mtn::byte_t* output)
    {
        *((uint64_t*) output) = htonll(input);
        return output + sizeof(input);
    }

    inline mtn::byte_t*
    encode_uint128(uint128_t   input,
                  mtn::byte_t* output)
    {
        *((uint128_t*) output) = htonlll(input);
        return output + sizeof(input);
    }

    inline mtn::byte_t*
    encode_parition(uint16_t     input,
                    mtn::byte_t* output)
    {
        return encode_uint16(input, output);
    }

    inline mtn::byte_t*
    encode_bytes(const mtn::byte_t* input,
                 uint16_t           size,
                 mtn::byte_t*       output)
    {
        memcpy(encode_uint16(size, output), input, size);
        return output + size + sizeof(uint16_t);
    }

    inline size_t
    get_index_key_size(uint16_t             partition,
                       uint16_t             bucket_size,
                       uint16_t             field_size,
                       mtn::index_address_t value,
                       mtn::index_address_t offset)
    {
        return sizeof(partition) + sizeof(uint16_t) + bucket_size + field_size + sizeof(value) + sizeof(offset);
    }

    inline mtn::byte_t*
    encode_index_key(uint16_t             partition,
                     const mtn::byte_t*   bucket,
                     uint16_t             bucket_size,
                     const mtn::byte_t*   field,
                     uint16_t             field_size,
                     mtn::index_address_t value,
                     mtn::index_address_t offset,
                     mtn::byte_t*         output)
    {
        mtn::byte_t* pos = encode_parition(partition, &output[0]);
        pos = encode_bytes(bucket, bucket_size, pos);
        pos = encode_bytes(field, field_size, pos);
        pos = encode_uint128(value, pos);
        return encode_uint128(offset, pos);
    }

    inline void
    encode_index_key(uint16_t                  partition,
                     const mtn::byte_t*        bucket,
                     uint16_t                  bucket_size,
                     const mtn::byte_t*        field,
                     uint16_t                  field_size,
                     mtn::index_address_t      value,
                     mtn::index_address_t      offset,
                     std::vector<mtn::byte_t>& output)
    {
        output.resize(get_index_key_size(partition, bucket_size, field_size, value, offset));
        encode_index_key(partition, bucket, bucket_size, field, field_size, value, offset, &output[0]);
    }

} // namespace mtn

#endif // __MUTTON_ENCODE_HPP_INCLUDED__
