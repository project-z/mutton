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
#include "fixtures.hpp"
#include "index.hpp"

BOOST_AUTO_TEST_SUITE(_index)

BOOST_AUTO_TEST_CASE(index_partition)
{
    prz::index_t index(1, "foobar", 6);
    BOOST_CHECK_EQUAL(1, index.partition());
}

BOOST_AUTO_TEST_CASE(index_field)
{
    prz::index_t index(1, "foobar", 6);
    BOOST_CHECK_EQUAL(0, memcmp("foobar", index.field(), 6));
}

BOOST_AUTO_TEST_CASE(index_size)
{
    prz::index_t index(1, "foobar", 6);
    BOOST_CHECK_EQUAL(0, index.size());
    index.insert(1, new prz::index_slice_t(1, "foobar", 6, 1));
    index.insert(2, new prz::index_slice_t(1, "foobar", 6, 2));
    BOOST_CHECK_EQUAL(2, index.size());
}

BOOST_AUTO_TEST_CASE(index_find)
{
    prz::index_t index(1, "foobar", 6);
    index.insert(1, new prz::index_slice_t(1, "foobar", 6, 1));
    BOOST_CHECK(index.find(1) != index.end());
    BOOST_CHECK(index.find(2) == index.end());
}

BOOST_AUTO_TEST_CASE(index_execute_static_norange_union_unaligned)
{
    index_reader_writer_memory_t reader_writer;

    prz::index_t a(1, "foobar", 6);
    a.insert(1, new prz::index_slice_t(1, "foobar", 6, 1));
    a.insert(2, new prz::index_slice_t(1, "foobar", 6, 2));
    a.begin()->second->bit(&reader_writer, &reader_writer, 2048, true);

    prz::index_t b(1, "bizbang", 7);
    b.insert(1, new prz::index_slice_t(1, "bizbang", 7, 1));
    b.insert(2, new prz::index_slice_t(1, "bizbang", 7, 2));
    b.begin()->second->bit(&reader_writer, &reader_writer, 1, true);

    prz::index_slice_t a_slice;
    a.slice(a_slice);

    prz::index_slice_t b_slice;
    b.slice(b_slice);

    prz::index_slice_t o;
    BOOST_CHECK_EQUAL(0, o.size());
    prz::index_slice_t::execute(prz::PRZ_INDEX_OP_UNION, a_slice, b_slice, o);
    BOOST_CHECK_EQUAL(2, o.size());
}

BOOST_AUTO_TEST_CASE(index_execute_static_norange_intersection_unaligned)
{
    index_reader_writer_memory_t reader_writer;

    prz::index_t a(1, "foobar", 6);
    a.insert(1, new prz::index_slice_t(1, "foobar", 6, 1));
    a.insert(2, new prz::index_slice_t(1, "foobar", 6, 2));
    a.begin()->second->bit(&reader_writer, &reader_writer, 2048, true);

    prz::index_t b(1, "bizbang", 7);
    b.insert(1, new prz::index_slice_t(1, "bizbang", 7, 1));
    b.insert(2, new prz::index_slice_t(1, "bizbang", 7, 2));
    b.begin()->second->bit(&reader_writer, &reader_writer, 1, true);

    prz::index_slice_t a_slice;
    a.slice(a_slice);

    prz::index_slice_t b_slice;
    b.slice(b_slice);

    prz::index_slice_t o;
    BOOST_CHECK_EQUAL(0, o.size());
    prz::index_slice_t::execute(prz::PRZ_INDEX_OP_INTERSECTION, a_slice, b_slice, o);
    BOOST_CHECK_EQUAL(0, o.size());
}

BOOST_AUTO_TEST_CASE(index_execute_static_norange_intersection_aligned)
{
    index_reader_writer_memory_t reader_writer;

    prz::index_t a(1, "foobar", 6);
    a.insert(1, new prz::index_slice_t(1, "foobar", 6, 1));
    a.insert(2, new prz::index_slice_t(1, "foobar", 6, 2));
    a.begin()->second->bit(&reader_writer, &reader_writer, 3, true);

    prz::index_t b(1, "bizbang", 7);
    b.insert(1, new prz::index_slice_t(1, "bizbang", 7, 1));
    b.insert(2, new prz::index_slice_t(1, "bizbang", 7, 2));
    b.begin()->second->bit(&reader_writer, &reader_writer, 3, true);

    prz::index_slice_t a_slice;
    a.slice(a_slice);

    prz::index_slice_t b_slice;
    b.slice(b_slice);

    prz::index_slice_t o;
    BOOST_CHECK_EQUAL(0, o.size());
    prz::index_slice_t::execute(prz::PRZ_INDEX_OP_INTERSECTION, a_slice, b_slice, o);
    BOOST_CHECK_EQUAL(1, o.size());

    BOOST_CHECK_EQUAL(0, o.begin()->offset);
    BOOST_CHECK_EQUAL(8, o.begin()->segment[0]);
}

BOOST_AUTO_TEST_SUITE_END()
