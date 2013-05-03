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

#include <boost/test/unit_test.hpp>
#include "index_slice.hpp"
#include "fixtures.hpp"

BOOST_AUTO_TEST_SUITE(_index_slice)

BOOST_AUTO_TEST_CASE(node_constructor_offset)
{
    prz::index_slice_t::index_node_t n(2);
    BOOST_CHECK_EQUAL(2, n.offset);
}

BOOST_AUTO_TEST_CASE(node_constructor_data)
{
    prz::index_slice_t::index_node_t n(2, SEGMENT_EVERY_OTHER_ODD);
    BOOST_CHECK_EQUAL(2, n.offset);
    BOOST_CHECK_EQUAL(0, memcmp(n.segment, SEGMENT_EVERY_OTHER_ODD, PRZ_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(node_constructor_copy)
{
    prz::index_slice_t::index_node_t a(2, SEGMENT_EVERY_OTHER_ODD);
    prz::index_slice_t::index_node_t b(a);
    BOOST_CHECK_EQUAL(2, b.offset);
    BOOST_CHECK_EQUAL(0, memcmp(b.segment, SEGMENT_EVERY_OTHER_ODD, PRZ_INDEX_SEGMENT_SIZE));
    BOOST_CHECK(a.segment != b.segment);
}

BOOST_AUTO_TEST_CASE(slice_size)
{
    prz::index_slice_t index(1, "foobar", 6, 2);
    BOOST_CHECK_EQUAL(0, index.size());
    index.insert(index.begin(), new prz::index_slice_t::index_node_t(0));
    index.insert(index.begin(), new prz::index_slice_t::index_node_t(2));
    BOOST_CHECK_EQUAL(2, index.size());
}

BOOST_AUTO_TEST_CASE(slice_union_joint)
{
    prz::index_slice_t a(1, "foobar", 6, 2);
    prz::index_slice_t b(1, "foobar", 6, 3);
    prz::index_slice_t o(1, "foobar", 6, 3);
    a.insert(a.begin(), new prz::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new prz::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_ODD));
    BOOST_CHECK(prz::index_slice_t::execute(prz::PRZ_INDEX_OP_UNION, a, b, o));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, SEGMENT_EVERY, PRZ_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_union_disjoint)
{
    prz::index_slice_t a(1, "foobar", 6, 2);
    prz::index_slice_t b(1, "foobar", 6, 3);
    prz::index_slice_t o(1, "foobar", 6, 3);
    a.insert(a.begin(), new prz::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new prz::index_slice_t::index_node_t(1, SEGMENT_EVERY_OTHER_ODD));
    BOOST_CHECK(prz::index_slice_t::execute(prz::PRZ_INDEX_OP_UNION, a, b, o));
    BOOST_CHECK_EQUAL(2, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, SEGMENT_EVERY_OTHER_EVEN, PRZ_INDEX_SEGMENT_SIZE));
    BOOST_CHECK_EQUAL(0, memcmp((++o.begin())->segment, SEGMENT_EVERY_OTHER_ODD, PRZ_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_intersection_joint_nomatch)
{
    prz::index_slice_t a(1, "foobar", 6, 2);
    prz::index_slice_t b(1, "foobar", 6, 3);
    prz::index_slice_t o(1, "foobar", 6, 3);
    a.insert(a.begin(), new prz::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new prz::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_ODD));
    BOOST_CHECK(prz::index_slice_t::execute(prz::PRZ_INDEX_OP_INTERSECTION, a, b, o));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, SEGMENT_NONE, PRZ_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_intersection_joint_match)
{
    prz::index_slice_t a(1, "foobar", 6, 2);
    prz::index_slice_t b(1, "foobar", 6, 3);
    prz::index_slice_t o(1, "foobar", 6, 3);
    a.insert(a.begin(), new prz::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new prz::index_slice_t::index_node_t(0, SEGMENT_EVERY));
    BOOST_CHECK(prz::index_slice_t::execute(prz::PRZ_INDEX_OP_INTERSECTION, a, b, o));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, SEGMENT_EVERY_OTHER_EVEN, PRZ_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_intersection_disjoint)
{
    prz::index_slice_t a(1, "foobar", 6, 2);
    prz::index_slice_t b(1, "foobar", 6, 3);
    prz::index_slice_t o(1, "foobar", 6, 3);
    a.insert(a.begin(), new prz::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new prz::index_slice_t::index_node_t(1, SEGMENT_EVERY_OTHER_ODD));
    BOOST_CHECK(prz::index_slice_t::execute(prz::PRZ_INDEX_OP_INTERSECTION, a, b, o));
    BOOST_CHECK_EQUAL(0, o.size());
}

BOOST_AUTO_TEST_CASE(slice_intersection_joint_match_overwrite)
{
    prz::index_slice_t a(1, "foobar", 6, 2);
    prz::index_slice_t b(1, "foobar", 6, 3);
    a.insert(a.begin(), new prz::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new prz::index_slice_t::index_node_t(0, SEGMENT_EVERY));
    BOOST_CHECK(prz::index_slice_t::execute(prz::PRZ_INDEX_OP_INTERSECTION, a, b, b));
    BOOST_CHECK_EQUAL(1, b.size());
    BOOST_CHECK_EQUAL(0, memcmp(b.begin()->segment, SEGMENT_EVERY_OTHER_EVEN, PRZ_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_intersection_output_contains_data)
{
    prz::index_slice_t a(1, "foobar", 6, 2);
    prz::index_slice_t b(1, "foobar", 6, 3);
    prz::index_slice_t o(1, "foobar", 6, 3);
    a.insert(a.begin(), new prz::index_slice_t::index_node_t(0, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new prz::index_slice_t::index_node_t(0, SEGMENT_EVERY));
    o.insert(o.begin(), new prz::index_slice_t::index_node_t(0, SEGMENT_NONE));
    BOOST_CHECK(prz::index_slice_t::execute(prz::PRZ_INDEX_OP_INTERSECTION, a, b, o));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, SEGMENT_EVERY_OTHER_EVEN, PRZ_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_intersection_output_contains_data_at_beginning)
{
    prz::index_slice_t a(1, "foobar", 6, 2);
    prz::index_slice_t b(1, "foobar", 6, 3);
    prz::index_slice_t o(1, "foobar", 6, 3);
    a.insert(a.begin(), new prz::index_slice_t::index_node_t(1, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new prz::index_slice_t::index_node_t(1, SEGMENT_EVERY));
    o.insert(o.begin(), new prz::index_slice_t::index_node_t(0, SEGMENT_NONE));
    BOOST_CHECK(prz::index_slice_t::execute(prz::PRZ_INDEX_OP_INTERSECTION, a, b, o));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, SEGMENT_EVERY_OTHER_EVEN, PRZ_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_intersection_output_contains_data_at_end)
{
    prz::index_slice_t a(1, "foobar", 6, 2);
    prz::index_slice_t b(1, "foobar", 6, 3);
    prz::index_slice_t o(1, "foobar", 6, 3);
    a.insert(a.begin(), new prz::index_slice_t::index_node_t(1, SEGMENT_EVERY_OTHER_EVEN));
    b.insert(b.begin(), new prz::index_slice_t::index_node_t(1, SEGMENT_EVERY));
    o.insert(o.begin(), new prz::index_slice_t::index_node_t(2, SEGMENT_NONE));
    BOOST_CHECK(prz::index_slice_t::execute(prz::PRZ_INDEX_OP_INTERSECTION, a, b, o));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, SEGMENT_EVERY_OTHER_EVEN, PRZ_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_set_bit)
{
    index_reader_writer_memory_t reader_writer;
    prz::index_slice_t o(1, "foobar", 6, 2);

    BOOST_CHECK(!o.bit(2048));
    BOOST_CHECK_EQUAL(0, o.size());

    o.bit(&reader_writer, &reader_writer, 2048, true);
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(8, o.begin()->offset);

    prz::index_segment_ptr segment = o.begin()->segment;
    BOOST_CHECK_EQUAL(0, memcmp(segment, SEGMENT_ONE, PRZ_INDEX_SEGMENT_SIZE));
    BOOST_CHECK(o.bit(2048));

    o.bit(&reader_writer, &reader_writer, 2048, false);
    BOOST_CHECK(!o.bit(2048));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(8, o.begin()->offset);
}

BOOST_AUTO_TEST_SUITE_END()
