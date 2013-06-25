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
#include "index_slice.hpp"
#include "fixtures.hpp"

mtn::index_segment_t SEGMENT_NONE = {0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0};

mtn::index_segment_t SEGMENT_ONE = {1, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0};

mtn::index_segment_t SEGMENT_EVERY_OTHER_ODD = {0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX,
                                                0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX,
                                                0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX,
                                                0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX};

mtn::index_segment_t SEGMENT_EVERY_OTHER_EVEN = {UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0,
                                                 UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0,
                                                 UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0,
                                                 UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0};

mtn::index_segment_t SEGMENT_EVERY = {UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                                      UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                                      UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                                      UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX};

BOOST_AUTO_TEST_SUITE(_index_slice)

BOOST_AUTO_TEST_CASE(node_constructor_offset)
{
    mtn::index_slice_t::index_node_t n(2);
    BOOST_CHECK(2 == n.offset);
}

BOOST_AUTO_TEST_CASE(node_constructor_data)
{
    mtn::index_slice_t::index_node_t n(2, SEGMENT_EVERY_OTHER_ODD);
    BOOST_CHECK(2 == n.offset);
    BOOST_CHECK_EQUAL(0, memcmp(n.segment, SEGMENT_EVERY_OTHER_ODD, MTN_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(node_constructor_copy)
{
    mtn::index_slice_t::index_node_t a(2, SEGMENT_EVERY_OTHER_ODD);
    mtn::index_slice_t::index_node_t b(a);
    BOOST_CHECK(2 == b.offset);
    BOOST_CHECK_EQUAL(0, memcmp(b.segment, SEGMENT_EVERY_OTHER_ODD, MTN_INDEX_SEGMENT_SIZE));
    BOOST_CHECK(a.segment != b.segment);
}

BOOST_AUTO_TEST_CASE(slice_size)
{
    mtn::index_slice_t index(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 2);
    BOOST_CHECK_EQUAL(0, index.size());
    index.insert(index.begin(), new mtn::index_slice_t::index_node_t(0));
    index.insert(index.begin(), new mtn::index_slice_t::index_node_t(2));
    BOOST_CHECK_EQUAL(2, index.size());
}

BOOST_AUTO_TEST_CASE(slice_union_joint)
{
    mtn::index_slice_t a(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 2);
    mtn::index_slice_t b(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    mtn::index_slice_t o(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    a.insert(a.begin(), new mtn::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new mtn::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_ODD));
    BOOST_CHECK(mtn::index_slice_t::execute(mtn::MTN_INDEX_OP_UNION, a, b, o));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, SEGMENT_EVERY, MTN_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_union_disjoint)
{
    mtn::index_slice_t a(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 2);
    mtn::index_slice_t b(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    mtn::index_slice_t o(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    a.insert(a.begin(), new mtn::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new mtn::index_slice_t::index_node_t(1, SEGMENT_EVERY_OTHER_ODD));
    BOOST_CHECK(mtn::index_slice_t::execute(mtn::MTN_INDEX_OP_UNION, a, b, o));
    BOOST_CHECK_EQUAL(2, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, SEGMENT_EVERY_OTHER_EVEN, MTN_INDEX_SEGMENT_SIZE));
    BOOST_CHECK_EQUAL(0, memcmp((++o.begin())->segment, SEGMENT_EVERY_OTHER_ODD, MTN_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_intersection_joint_nomatch)
{
    mtn::index_slice_t a(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 2);
    mtn::index_slice_t b(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    mtn::index_slice_t o(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    a.insert(a.begin(), new mtn::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new mtn::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_ODD));
    BOOST_CHECK(mtn::index_slice_t::execute(mtn::MTN_INDEX_OP_INTERSECTION, a, b, o));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, SEGMENT_NONE, MTN_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_intersection_joint_match)
{
    mtn::index_slice_t a(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 2);
    mtn::index_slice_t b(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    mtn::index_slice_t o(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    a.insert(a.begin(), new mtn::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new mtn::index_slice_t::index_node_t(0, SEGMENT_EVERY));
    BOOST_CHECK(mtn::index_slice_t::execute(mtn::MTN_INDEX_OP_INTERSECTION, a, b, o));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, SEGMENT_EVERY_OTHER_EVEN, MTN_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_intersection_disjoint)
{
    mtn::index_slice_t a(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 2);
    mtn::index_slice_t b(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    mtn::index_slice_t o(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    a.insert(a.begin(), new mtn::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new mtn::index_slice_t::index_node_t(1, SEGMENT_EVERY_OTHER_ODD));
    BOOST_CHECK(mtn::index_slice_t::execute(mtn::MTN_INDEX_OP_INTERSECTION, a, b, o));
    BOOST_CHECK_EQUAL(0, o.size());
}

BOOST_AUTO_TEST_CASE(slice_intersection_joint_match_overwrite)
{
    mtn::index_slice_t a(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 2);
    mtn::index_slice_t b(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    a.insert(a.begin(), new mtn::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new mtn::index_slice_t::index_node_t(0, SEGMENT_EVERY));
    BOOST_CHECK(mtn::index_slice_t::execute(mtn::MTN_INDEX_OP_INTERSECTION, a, b, b));
    BOOST_CHECK_EQUAL(1, b.size());
    BOOST_CHECK_EQUAL(0, memcmp(b.begin()->segment, SEGMENT_EVERY_OTHER_EVEN, MTN_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_intersection_output_contains_data)
{
    mtn::index_slice_t a(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 2);
    mtn::index_slice_t b(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    mtn::index_slice_t o(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    a.insert(a.begin(), new mtn::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new mtn::index_slice_t::index_node_t(0, SEGMENT_EVERY));
    o.insert(o.begin(), new mtn::index_slice_t::index_node_t(0, SEGMENT_NONE));
    BOOST_CHECK(mtn::index_slice_t::execute(mtn::MTN_INDEX_OP_INTERSECTION, a, b, o));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, SEGMENT_EVERY_OTHER_EVEN, MTN_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_intersection_output_contains_data_at_beginning)
{
    mtn::index_slice_t a(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 2);
    mtn::index_slice_t b(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    mtn::index_slice_t o(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    a.insert(a.begin(), new mtn::index_slice_t::index_node_t(1, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new mtn::index_slice_t::index_node_t(1, SEGMENT_EVERY));
    o.insert(o.begin(), new mtn::index_slice_t::index_node_t(0, SEGMENT_NONE));
    BOOST_CHECK(mtn::index_slice_t::execute(mtn::MTN_INDEX_OP_INTERSECTION, a, b, o));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, SEGMENT_EVERY_OTHER_EVEN, MTN_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_intersection_output_contains_data_at_end)
{
    mtn::index_slice_t a(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 2);
    mtn::index_slice_t b(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    mtn::index_slice_t o(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 3);
    a.insert(a.begin(), new mtn::index_slice_t::index_node_t(1, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new mtn::index_slice_t::index_node_t(1, SEGMENT_EVERY));
    o.insert(o.begin(), new mtn::index_slice_t::index_node_t(2, SEGMENT_NONE));
    BOOST_CHECK(mtn::index_slice_t::execute(mtn::MTN_INDEX_OP_INTERSECTION, a, b, o));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, SEGMENT_EVERY_OTHER_EVEN, MTN_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_set_bit)
{
    index_reader_writer_memory_t reader_writer;
    mtn::index_slice_t o(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 2);

    BOOST_CHECK(!o.bit(2048));
    BOOST_CHECK_EQUAL(0, o.size());

    o.bit(reader_writer, reader_writer, 2048, true);
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK(8 == o.begin()->offset);

    mtn::index_segment_ptr segment = o.begin()->segment;
    BOOST_CHECK_EQUAL(0, memcmp(segment, SEGMENT_ONE, MTN_INDEX_SEGMENT_SIZE));
    BOOST_CHECK(o.bit(2048));

    o.bit(reader_writer, reader_writer, 2048, false);
    BOOST_CHECK(!o.bit(2048));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK(8 == o.begin()->offset);
}

BOOST_AUTO_TEST_CASE(slice_check_bit_32)
{
   index_reader_writer_memory_t reader_writer;
    mtn::index_slice_t o(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 2);

    BOOST_CHECK(!o.bit(32));
    o.bit(reader_writer, reader_writer, 0, true);
    BOOST_CHECK(!o.bit(32));
}

BOOST_AUTO_TEST_CASE(slice_set_bit_32)
{
   index_reader_writer_memory_t reader_writer;
    mtn::index_slice_t o(1, reinterpret_cast<const mtn::byte_t*>("bizbang"), 7, reinterpret_cast<const mtn::byte_t*>("foobar"), 6, 2);

    BOOST_CHECK(!o.bit(32));
    o.bit(reader_writer, reader_writer, 32, true);
    BOOST_CHECK(o.bit(32));
}

BOOST_AUTO_TEST_SUITE_END()
