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
#include "trigram.hpp"

BOOST_AUTO_TEST_SUITE(trigram)

BOOST_AUTO_TEST_CASE(simple)
{
    std::string input = "foobar";
    mtn::trigram_t output;

    std::string::iterator iter = mtn::trigram_t::init(input.begin(), input.end(), output);

    BOOST_CHECK(input.begin() + 3 == iter);
    BOOST_CHECK_EQUAL(102, output.one);
    BOOST_CHECK_EQUAL(111, output.two);
    BOOST_CHECK_EQUAL(111, output.three);

    BOOST_CHECK_EQUAL(102, (uint32_t) (output.hash() >> 64));
    BOOST_CHECK_EQUAL(111, (uint32_t) (output.hash() >> 32));
    BOOST_CHECK_EQUAL(111, (uint32_t) output.hash());
}

BOOST_AUTO_TEST_CASE(small)
{
    std::string input = "fo";
    mtn::trigram_t output;

    std::string::iterator iter = mtn::trigram_t::init(input.begin(), input.end(), output);

    BOOST_CHECK(input.begin() + 2 == iter);
    BOOST_CHECK_EQUAL(102, output.one);
    BOOST_CHECK_EQUAL(111, output.two);
    BOOST_CHECK_EQUAL(0, output.three);

    BOOST_CHECK_EQUAL(102, (uint32_t) (output.hash() >> 64));
    BOOST_CHECK_EQUAL(111, (uint32_t) (output.hash() >> 32));
    BOOST_CHECK_EQUAL(0, (uint32_t) output.hash());
}

BOOST_AUTO_TEST_CASE(order)
{
    std::string input1 = "fo";
    std::string input2 = "foo";
    mtn::trigram_t tri1;
    mtn::trigram_t tri2;

    mtn::trigram_t::init(input1.begin(), input1.end(), tri1);
    mtn::trigram_t::init(input2.begin(), input2.end(), tri2);

    BOOST_CHECK(tri1.hash() < tri2.hash());
}

BOOST_AUTO_TEST_CASE(loop)
{
    std::string input = "fooooooo";
    mtn::trigram_t output;

    int counter = 0;
    std::string::iterator pos = input.begin();

    while (pos != input.end()) {
        output.zero();
        pos = mtn::trigram_t::init(pos, input.end(), output);
        ++counter;
    }

    BOOST_CHECK_EQUAL(111, output.one);
    BOOST_CHECK_EQUAL(111, output.two);
    BOOST_CHECK_EQUAL(0, output.three);
    BOOST_CHECK_EQUAL(3, counter);
}

BOOST_AUTO_TEST_SUITE_END()
