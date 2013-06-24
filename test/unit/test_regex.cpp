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
#include "base_types.hpp"
#include "regex.hpp"

BOOST_AUTO_TEST_SUITE(_regex)

BOOST_AUTO_TEST_CASE(one)
{
    mtn::regex_t r("foo.*");
    std::vector<std::string> pieces;
    BOOST_CHECK(mtn::regex_t::to_pieces(r, pieces));
    BOOST_CHECK_EQUAL(1, pieces.size());
    BOOST_CHECK_EQUAL("foo", pieces[0]);
}

BOOST_AUTO_TEST_CASE(two)
{
    mtn::regex_t r("foo.*ooo");
    std::vector<std::string> pieces;
    BOOST_CHECK(mtn::regex_t::to_pieces(r, pieces));
    BOOST_CHECK_EQUAL(2, pieces.size());
    BOOST_CHECK_EQUAL("ooo", pieces[0]);
    BOOST_CHECK_EQUAL("foo", pieces[1]);
}

BOOST_AUTO_TEST_CASE(two_with_long)
{
    mtn::regex_t r("foooo.*ooo");
    std::vector<std::string> pieces;
    BOOST_CHECK(mtn::regex_t::to_pieces(r, pieces));
    BOOST_CHECK_EQUAL(2, pieces.size());
    BOOST_CHECK_EQUAL("ooo", pieces[0]);
    BOOST_CHECK_EQUAL("foooo", pieces[1]);
}

BOOST_AUTO_TEST_SUITE_END()
