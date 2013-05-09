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
#include "trigram.hpp"

BOOST_AUTO_TEST_SUITE(trigram)

BOOST_AUTO_TEST_CASE(simple)
{
    std::string input = "foobar";
    prz::trigram_t output;

    char* new_ptr = prz::trigram_t::init(input.c_str(), input.c_str() + input.size(), &output);

    BOOST_CHECK_EQUAL(input.c_str() + 3, new_ptr);
    BOOST_CHECK_EQUAL(102, output.one);
    BOOST_CHECK_EQUAL(111, output.two);
    BOOST_CHECK_EQUAL(111, output.three);
    BOOST_CHECK_EQUAL(18081745206516578866ULL, output.hash());
}

BOOST_AUTO_TEST_CASE(small)
{
    std::string input = "fo";
    prz::trigram_t output;

    char* new_ptr = prz::trigram_t::init(input.c_str(), input.c_str() + input.size(), &output);

    BOOST_CHECK_EQUAL(input.c_str() + 2, new_ptr);
    BOOST_CHECK_EQUAL(102, output.one);
    BOOST_CHECK_EQUAL(111, output.two);
    BOOST_CHECK_EQUAL(0, output.three);
    BOOST_CHECK_EQUAL(18122317844775107122ULL, output.hash());
}

BOOST_AUTO_TEST_CASE(loop)
{
    std::string input = "fooooooo";
    prz::trigram_t output;

    int counter = 0;
    char* pos = const_cast<char*>(input.c_str());
    const char* end = pos + input.size();

    while ((pos = prz::trigram_t::init(pos, end, &output)) < end) {
        output.zero();
        ++counter;
    }

    BOOST_CHECK_EQUAL(111, output.one);
    BOOST_CHECK_EQUAL(111, output.two);
    BOOST_CHECK_EQUAL(0, output.three);
    BOOST_CHECK_EQUAL(2, counter);
}

BOOST_AUTO_TEST_SUITE_END()
