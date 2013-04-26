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

#include <stdint.h>
#include <boost/test/unit_test.hpp>
#include "index.hpp"

// Required to use stdint.h
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

// 0xAAAAAAAAAAAAAAAAULL

prz::index_t::segment_t NONE = {0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0};

prz::index_t::segment_t EVERY_OTHER_ODD = {0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX,
                                           0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX,
                                           0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX,
                                           0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX};

prz::index_t::segment_t EVERY_OTHER_EVEN = {UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0,
                                            UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0,
                                            UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0,
                                            UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0, UINT64_MAX, 0};

prz::index_t::segment_t EVERY = {UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                                 UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                                 UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                                 UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX};

BOOST_AUTO_TEST_SUITE(_index)

BOOST_AUTO_TEST_CASE(node_constructor_offset)
{
    prz::index_t::index_node_t n(2);
    BOOST_CHECK_EQUAL(2, n.offset);
}

BOOST_AUTO_TEST_CASE(node_constructor_data)
{
    prz::index_t::index_node_t n(2, EVERY_OTHER_ODD);
    BOOST_CHECK_EQUAL(2, n.offset);
    BOOST_CHECK_EQUAL(0, memcmp(n.segment, EVERY_OTHER_ODD, SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(node_constructor_copy)
{
    prz::index_t::index_node_t a(2, EVERY_OTHER_ODD);
    prz::index_t::index_node_t b(a);
    BOOST_CHECK_EQUAL(2, b.offset);
    BOOST_CHECK_EQUAL(0, memcmp(b.segment, EVERY_OTHER_ODD, SEGMENT_SIZE));
    BOOST_CHECK(a.segment != b.segment);
}

BOOST_AUTO_TEST_CASE(index_size)
{
    prz::index_t index(1, "foobar", 6, 2);
    BOOST_CHECK_EQUAL(0, index.size());
    index.insert(index.begin(), new prz::index_t::index_node_t(0));
    index.insert(index.begin(), new prz::index_t::index_node_t(2));
    BOOST_CHECK_EQUAL(2, index.size());
}

BOOST_AUTO_TEST_CASE(index_union_joint)
{
    prz::index_t a(1, "foobar", 6, 2);
    prz::index_t b(1, "foobar", 6, 3);
    prz::index_t o(1, "foobar", 6, 3);
    a.insert(a.begin(), new prz::index_t::index_node_t(0, EVERY_OTHER_EVEN));
    b.insert(b.begin(), new prz::index_t::index_node_t(0, EVERY_OTHER_ODD));
    BOOST_CHECK_EQUAL(true, prz::index_t::execute(prz::INDEX_UNION, a, b, o));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, EVERY, SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(index_union_disjoint)
{
    prz::index_t a(1, "foobar", 6, 2);
    prz::index_t b(1, "foobar", 6, 3);
    prz::index_t o(1, "foobar", 6, 3);
    a.insert(a.begin(), new prz::index_t::index_node_t(0, EVERY_OTHER_EVEN));
    b.insert(b.begin(), new prz::index_t::index_node_t(1, EVERY_OTHER_ODD));
    BOOST_CHECK_EQUAL(true, prz::index_t::execute(prz::INDEX_UNION, a, b, o));
    BOOST_CHECK_EQUAL(2, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, EVERY_OTHER_EVEN, SEGMENT_SIZE));
    BOOST_CHECK_EQUAL(0, memcmp((++o.begin())->segment, EVERY_OTHER_ODD, SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(index_intersection_joint_nomatch)
{
    prz::index_t a(1, "foobar", 6, 2);
    prz::index_t b(1, "foobar", 6, 3);
    prz::index_t o(1, "foobar", 6, 3);
    a.insert(a.begin(), new prz::index_t::index_node_t(0, EVERY_OTHER_EVEN));
    b.insert(b.begin(), new prz::index_t::index_node_t(0, EVERY_OTHER_ODD));
    BOOST_CHECK_EQUAL(true, prz::index_t::execute(prz::INDEX_INTERSECTION, a, b, o));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, NONE, SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(index_intersection_joint_match)
{
    prz::index_t a(1, "foobar", 6, 2);
    prz::index_t b(1, "foobar", 6, 3);
    prz::index_t o(1, "foobar", 6, 3);
    a.insert(a.begin(), new prz::index_t::index_node_t(0, EVERY_OTHER_EVEN));
    b.insert(b.begin(), new prz::index_t::index_node_t(0, EVERY));
    BOOST_CHECK_EQUAL(true, prz::index_t::execute(prz::INDEX_INTERSECTION, a, b, o));
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(0, memcmp(o.begin()->segment, EVERY_OTHER_EVEN, SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(index_intersection_disjoint)
{
    prz::index_t a(1, "foobar", 6, 2);
    prz::index_t b(1, "foobar", 6, 3);
    prz::index_t o(1, "foobar", 6, 3);
    a.insert(a.begin(), new prz::index_t::index_node_t(0, EVERY_OTHER_EVEN));
    b.insert(b.begin(), new prz::index_t::index_node_t(1, EVERY_OTHER_ODD));
    BOOST_CHECK_EQUAL(true, prz::index_t::execute(prz::INDEX_INTERSECTION, a, b, o));
    BOOST_CHECK_EQUAL(0, o.size());
}

BOOST_AUTO_TEST_SUITE_END()
