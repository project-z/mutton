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

#include <boost/test/unit_test.hpp>
#include "encode.hpp"

BOOST_AUTO_TEST_SUITE(encode)

////////////////////////////////////////////////////////////////////////
// Decode
////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(ntohll)
{
    uint64_t input = 0x1122334455667788;
    BOOST_CHECK_EQUAL(0x8877665544332211, ntohll(input));
    BOOST_CHECK_EQUAL(input, ntohll(ntohll(input)));
}

BOOST_AUTO_TEST_CASE(decode_uint16)
{
    mtn::byte_t input[] = {0x11, 0x22};
    uint16_t output = 0x0000;
    BOOST_CHECK_EQUAL(input + sizeof(uint16_t), mtn::decode_uint16(input, &output));
    BOOST_CHECK_EQUAL(0x1122, output);
}

BOOST_AUTO_TEST_CASE(decode_uint32)
{
    mtn::byte_t input[] = {0x11, 0x22, 0x33, 0x44};
    uint32_t output = 0x00000000;
    BOOST_CHECK_EQUAL(input + sizeof(uint32_t), mtn::decode_uint32(input, &output));
    BOOST_CHECK_EQUAL(0x11223344, output);
}

BOOST_AUTO_TEST_CASE(decode_uint64)
{
    mtn::byte_t input[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint64_t output = 0x0000000000000000;
    BOOST_CHECK_EQUAL(input + sizeof(uint64_t), mtn::decode_uint64(input, &output));
    BOOST_CHECK_EQUAL(0x1122334455667788, output);
}

BOOST_AUTO_TEST_CASE(decode_parition)
{
    mtn::byte_t input[] = {0x11, 0x22};
    uint16_t output = 0x0000;
    BOOST_CHECK_EQUAL(input + sizeof(uint16_t), mtn::decode_parition(input, &output));
    BOOST_CHECK_EQUAL(0x1122, output);
}

BOOST_AUTO_TEST_CASE(decode_bytes)
{
    mtn::byte_t input[] = {0x00, 0x02, 0x11, 0x22};
    mtn::byte_t* output = NULL;
    uint16_t output_size = 0x0000;
    BOOST_CHECK_EQUAL(input + sizeof(input), mtn::decode_bytes(input, &output, &output_size));
    BOOST_CHECK_EQUAL(input + 2, output);
    BOOST_CHECK_EQUAL(2, output_size);
    BOOST_CHECK_EQUAL(0, memcmp(input + 2, output, output_size));
}

BOOST_AUTO_TEST_CASE(decode_index_key)
{
    mtn::byte_t input[] = {0x01, 0x02, 0x00, 0x02, 0x03, 0x04, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0xDD, 0xEE, 0xAA, 0xDD, 0xBB, 0xEE, 0xEE, 0xFF};
    uint16_t partition = 0;
    mtn::byte_t*    field = NULL;
    uint16_t field_size = 0;
    uint64_t value = 0;
    uint64_t offset = 0;

    BOOST_CHECK_EQUAL(input + sizeof(input), mtn::decode_index_key(input, &partition, &field, &field_size, &value, &offset));
    BOOST_CHECK_EQUAL(0x0102, partition);
    BOOST_CHECK_EQUAL(0x03, field[0]);
    BOOST_CHECK_EQUAL(0x04, field[1]);
    BOOST_CHECK_EQUAL(2, field_size);
    BOOST_CHECK_EQUAL(0x1122334455667788, value);
    BOOST_CHECK_EQUAL(0xDDEEAADDBBEEEEFF, offset);
}

////////////////////////////////////////////////////////////////////////
// Encode
////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(htonll)
{
    uint64_t input = 0x1122334455667788;
    BOOST_CHECK_EQUAL(0x8877665544332211, htonll(input));
    BOOST_CHECK_EQUAL(input, htonll(htonll(input)));
}

BOOST_AUTO_TEST_CASE(encode_uint16)
{
    mtn::byte_t output_ref[] = {0x11, 0x22};
    uint16_t input = 0x1122;
    mtn::byte_t output[2];
    BOOST_CHECK_EQUAL(output + sizeof(input), mtn::encode_uint16(input, output));
    BOOST_CHECK_EQUAL(0, memcmp(output_ref, output, sizeof(output)));
}

BOOST_AUTO_TEST_CASE(encode_uint32)
{
    mtn::byte_t output_ref[] = {0x11, 0x22, 0x33, 0x44};
    uint32_t input = 0x11223344;
    mtn::byte_t output[4];
    BOOST_CHECK_EQUAL(output + sizeof(input), mtn::encode_uint32(input, output));
    BOOST_CHECK_EQUAL(0, memcmp(output_ref, output, sizeof(output)));
}

BOOST_AUTO_TEST_CASE(encode_uint64)
{
    mtn::byte_t output_ref[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint64_t input = 0x1122334455667788;
    mtn::byte_t output[8];
    BOOST_CHECK_EQUAL(output + sizeof(input), mtn::encode_uint64(input, output));
    BOOST_CHECK_EQUAL(0, memcmp(output_ref, output, sizeof(output)));
}

BOOST_AUTO_TEST_CASE(encode_parition)
{
    mtn::byte_t output_ref[] = {0x11, 0x22};
    uint16_t input = 0x1122;
    mtn::byte_t output[2];
    BOOST_CHECK_EQUAL(output + sizeof(input), mtn::encode_parition(input, output));
    BOOST_CHECK_EQUAL(0, memcmp(output_ref, output, sizeof(output)));
}

BOOST_AUTO_TEST_CASE(encode_bytes)
{
    mtn::byte_t output_ref[] = {0x00, 0x02, 0x11, 0x22};
    mtn::byte_t output[4];
    mtn::byte_t input[2] = {0x11, 0x22};
    BOOST_CHECK_EQUAL(output + sizeof(output), mtn::encode_bytes(input, sizeof(input), output));
    BOOST_CHECK_EQUAL(0, memcmp(output_ref, output, sizeof(output)));
}

BOOST_AUTO_TEST_CASE(encode_index_key)
{

    mtn::byte_t     output_ref[] = {0x01, 0x02, 0x00, 0x02, 0x03, 0x04, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0xDD, 0xEE, 0xAA, 0xDD, 0xBB, 0xEE, 0xEE, 0xFF};
    mtn::byte_t     output[22];
    uint16_t partition    = 0x0102;
    mtn::byte_t     field[]      = {0x03, 0x04};
    uint16_t field_size   = 2;
    uint64_t value        = 0x1122334455667788;
    uint64_t offset       = 0xDDEEAADDBBEEEEFF;

    BOOST_CHECK_EQUAL(output + sizeof(output), mtn::encode_index_key(partition, field, field_size, value, offset, output));
    BOOST_CHECK(memcmp(output_ref, output, sizeof(output)) == 0);
}

BOOST_AUTO_TEST_SUITE_END()
