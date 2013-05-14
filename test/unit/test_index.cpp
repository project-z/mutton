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
#include "fixtures.hpp"
#include "index.hpp"
#include "range.hpp"

#include "city.h"

BOOST_AUTO_TEST_SUITE(_index)

BOOST_AUTO_TEST_CASE(index_partition)
{
    mtn::index_t index(1, "foobar", 6);
    BOOST_CHECK_EQUAL(1, index.partition());
}

BOOST_AUTO_TEST_CASE(index_field)
{
    mtn::index_t index(1, "foobar", 6);
    BOOST_CHECK_EQUAL(0, memcmp("foobar", index.field(), 6));
}

BOOST_AUTO_TEST_CASE(index_size)
{
    mtn::index_t index(1, "foobar", 6);
    BOOST_CHECK_EQUAL(0, index.size());
    index.insert(1, new mtn::index_slice_t(1, "foobar", 6, 1));
    index.insert(2, new mtn::index_slice_t(1, "foobar", 6, 2));
    BOOST_CHECK_EQUAL(2, index.size());
}

BOOST_AUTO_TEST_CASE(index_find)
{
    mtn::index_t index(1, "foobar", 6);
    index.insert(1, new mtn::index_slice_t(1, "foobar", 6, 1));
    BOOST_CHECK(index.find(1) != index.end());
    BOOST_CHECK(index.find(2) == index.end());
}

BOOST_AUTO_TEST_CASE(index_slice_norange)
{
    index_reader_writer_memory_t reader_writer;

    mtn::index_t index(1, "foobar", 6);
    index.index_value(&reader_writer, &reader_writer, 1, 2048, true);
    index.index_value(&reader_writer, &reader_writer, 3, 2049, true);

    mtn::index_slice_t o;
    index.slice(o);
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(8, o.begin()->offset);
    BOOST_CHECK_EQUAL(3, o.begin()->segment[0]);
}

BOOST_AUTO_TEST_CASE(index_slice_single_range)
{
    index_reader_writer_memory_t reader_writer;

    mtn::index_t index(1, "foobar", 6);
    index.index_value(&reader_writer, &reader_writer, 1, 2048, true);
    index.index_value(&reader_writer, &reader_writer, 2, 1024, true);
    index.index_value(&reader_writer, &reader_writer, 3, 2049, true);

    mtn::range_t range(0, 2);

    mtn::index_slice_t o;
    index.slice(&range, 1, o);
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(8, o.begin()->offset);
    BOOST_CHECK_EQUAL(1, o.begin()->segment[0]);
}

BOOST_AUTO_TEST_CASE(index_slice_multiple_ranges)
{
    index_reader_writer_memory_t reader_writer;

    mtn::index_t index(1, "foobar", 6);
    index.index_value(&reader_writer, &reader_writer, 1, 2048, true);
    index.index_value(&reader_writer, &reader_writer, 2, 1024, true);
    index.index_value(&reader_writer, &reader_writer, 3, 2049, true);
    index.index_value(&reader_writer, &reader_writer, 4, 3049, true);

    std::vector<mtn::range_t> ranges;
    ranges.reserve(2);
    ranges.push_back(mtn::range_t(0, 2));
    ranges.push_back(mtn::range_t(3, 4));

    mtn::index_slice_t o;
    index.slice(&ranges[0], 2, o);
    BOOST_CHECK_EQUAL(1, o.size());
    BOOST_CHECK_EQUAL(8, o.begin()->offset);
    BOOST_CHECK_EQUAL(3, o.begin()->segment[0]);
}

// BOOST_AUTO_TEST_CASE(index_index_hash)
// {
//     index_reader_writer_memory_t reader_writer;

//     mtn::index_t index(1, "foobar", 6);
//     index.index_value_hash(&reader_writer, &reader_writer, "foobar", 6, 2048, true);
//     std::cout << "foobar: " << CityHash64("foobar", 6) << std::endl;

//     mtn::index_slice_t o;
//     index.slice(o);

//     BOOST_CHECK_EQUAL(1, o.size());
//     BOOST_CHECK_EQUAL(8, o.begin()->offset);
//     BOOST_CHECK_EQUAL(1, o.begin()->segment[0]);
// }

BOOST_AUTO_TEST_SUITE_END()
