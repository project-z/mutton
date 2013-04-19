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

#ifndef __X_ENCODE_HPP_INCLUDED__
#define __X_ENCODE_HPP_INCLUDED__

#include <machine/endian.h>
#include <stdint.h>
#include <tmmintrin.h>

// Required to use stdint.h
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ntohll(x) (uint64_t) __builtin_bswap64(x)
#define htonll(x) ntohll(x)
#else
#define ntohll(x) (x)
#define htonll(x) (x)
#endif

namespace prz {

////////////////////////////////////////////////////////////////////////
// Decode
////////////////////////////////////////////////////////////////////////

    inline char*
    decode_uint16(char*     input,
                  uint16_t* output)
    {
        *output = ntohs(*(reinterpret_cast<const uint16_t*>(input)));
        return input + sizeof(uint16_t);
    }

    inline char*
    decode_uint32(char*     input,
                  uint32_t* output)
    {
        *output = ntohl(*(reinterpret_cast<const uint32_t*>(input)));
        return input + sizeof(uint32_t);
    }

    inline char*
    decode_uint64(char*     input,
                  uint64_t* output)
    {
        *output = ntohll(*(reinterpret_cast<const uint64_t*>(input)));
        return input + sizeof(uint64_t);
    }

    inline char*
    decode_parition(char*     input,
                    uint16_t* output)
    {
        return decode_uint16(input, output);
    }

    inline char*
    decode_bytes(char*     input,
                 char**    output,
                 uint16_t* size)
    {
        *output = decode_uint16(input, size);
        return *output + *size;
    }

    inline char*
    decode_index_key(char*     input,
                     uint16_t* partition,
                     char**    field,
                     uint16_t* field_size,
                     uint64_t* value,
                     uint64_t* offset)
    {
        char* output = decode_parition(input, partition);
        output = decode_bytes(output, field, field_size);
        output = decode_uint64(output, value);
        return decode_uint64(output, offset);
    }

////////////////////////////////////////////////////////////////////////
// Encode
////////////////////////////////////////////////////////////////////////

    inline char*
    encode_uint16(uint16_t  input,
                  char*     output)
    {
        *((uint16_t*) output) = htons(input);
        return output + sizeof(input);
    }

    inline char*
    encode_uint32(uint32_t  input,
                  char*     output)
    {
        *((uint32_t*) output) = htonl(input);
        return output + sizeof(input);
    }

    inline char*
    encode_uint64(uint64_t  input,
                  char*     output)
    {
        *((uint64_t*) output) = htonll(input);
        return output + sizeof(input);
    }

    inline char*
    encode_parition(uint16_t  input,
                    char*     output)
    {
        return encode_uint16(input, output);
    }

    inline char*
    encode_bytes(const char* input,
                 uint16_t    size,
                 char*       output)
    {
        memcpy(encode_uint16(size, output), input, size);
        return output + size + sizeof(uint16_t);
    }

    inline size_t
    get_index_key_size(uint16_t    partition,
                       const char* field,
                       uint16_t    field_size,
                       uint64_t    value,
                       uint64_t    offset)
    {
        return sizeof(partition) + sizeof(uint16_t) + field_size + sizeof(value) + sizeof(offset);
    }

    inline char*
    encode_index_key(uint16_t    partition,
                     const char* field,
                     uint16_t    field_size,
                     uint64_t    value,
                     uint64_t    offset,
                     char*       output)
    {
        char* pos = encode_parition(partition, &output[0]);
        pos = encode_bytes(field, field_size, pos);
        pos = encode_uint64(value, pos);
        return encode_uint64(offset, pos);
    }

    inline void
    encode_index_key(uint16_t           partition,
                     const char*        field,
                     uint16_t           field_size,
                     uint64_t           value,
                     uint64_t           offset,
                     std::vector<char>& output)
    {
        output.resize(get_index_key_size(partition, field, field_size, value, offset));
        encode_index_key(partition, field, field_size, value, offset, &output[0]);
    }

} // namespace prz

#endif // __X_ENCODE_HPP_INCLUDED__
