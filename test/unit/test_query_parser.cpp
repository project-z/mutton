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
#include "query_parser.hpp"

BOOST_AUTO_TEST_SUITE(_query_planner)

BOOST_AUTO_TEST_CASE(test_slice_ascii)
{
    std::string input = "(slice \"foobar\")";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));
    BOOST_CHECK_EQUAL(2, result.which());
    BOOST_CHECK_EQUAL("foobar", boost::get<mtn::op_slice>(result).index);
    BOOST_CHECK(boost::get<mtn::op_slice>(result).values.empty());
}

BOOST_AUTO_TEST_CASE(test_slice_utf8)
{
    std::string input = "(slice \"ταБЬℓσ\")";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));
    BOOST_CHECK_EQUAL(2, result.which());
    BOOST_CHECK_EQUAL("ταБЬℓσ", boost::get<mtn::op_slice>(result).index);
    BOOST_CHECK(boost::get<mtn::op_slice>(result).values.empty());
}

BOOST_AUTO_TEST_CASE(test_slice_range)
{
    std::string input = "(slice \"foobar\" (range 1 2))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));
    BOOST_CHECK_EQUAL(2, result.which());

    mtn::op_slice slice = boost::get<mtn::op_slice>(result);
    BOOST_CHECK_EQUAL("foobar", slice.index);
    BOOST_CHECK_EQUAL(1, slice.values.size());

    mtn::op_slice::iterator iter = slice.values.begin();
    BOOST_CHECK_EQUAL(0, iter->which());
    mtn::range_t range_one = boost::get<mtn::range_t>(*iter);

    BOOST_CHECK(1 == range_one.start);
    BOOST_CHECK(2 == range_one.limit);
}

BOOST_AUTO_TEST_CASE(test_slice_range_randint)
{
    std::string input = "(slice \"foobar\" (range 1 184467440737095516))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));
    BOOST_CHECK_EQUAL(2, result.which());

    mtn::op_slice slice = boost::get<mtn::op_slice>(result);
    BOOST_CHECK_EQUAL("foobar", slice.index);
    BOOST_CHECK_EQUAL(1, slice.values.size());

    mtn::op_slice::iterator iter = slice.values.begin();
    BOOST_CHECK_EQUAL(0, iter->which());
    mtn::range_t range_one = boost::get<mtn::range_t>(*iter);

    BOOST_CHECK(1 == range_one.start);
    BOOST_CHECK(184467440737095516 == range_one.limit);
}

BOOST_AUTO_TEST_CASE(test_slice_range_big64uint)
{
    std::string input = "(slice \"foobar\" (range 1 18446744073709551615))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));
    BOOST_CHECK_EQUAL(2, result.which());

    mtn::op_slice slice = boost::get<mtn::op_slice>(result);
    BOOST_CHECK_EQUAL("foobar", slice.index);
    BOOST_CHECK_EQUAL(1, slice.values.size());

    mtn::op_slice::iterator iter = slice.values.begin();
    BOOST_CHECK_EQUAL(0, iter->which());
    mtn::range_t range_one = boost::get<mtn::range_t>(*iter);

    BOOST_CHECK(1 == range_one.start);
    BOOST_CHECK(18446744073709551615ULL == range_one.limit);
}

BOOST_AUTO_TEST_CASE(test_slice_range_big128uint)
{
    std::string input = "(slice \"foobar\" (range 1 340282366920938463463374607431768211455))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));
    BOOST_CHECK_EQUAL(2, result.which());

    mtn::op_slice slice = boost::get<mtn::op_slice>(result);
    BOOST_CHECK_EQUAL("foobar", slice.index);
    BOOST_CHECK_EQUAL(1, slice.values.size());

    mtn::op_slice::iterator iter = slice.values.begin();
    BOOST_CHECK_EQUAL(0, iter->which());
    mtn::range_t range_one = boost::get<mtn::range_t>(*iter);

    BOOST_CHECK(1 == range_one.start);
    BOOST_CHECK_EQUAL(0xFFFFFFFFFFFFFFFF, (uint64_t) range_one.limit);
    BOOST_CHECK_EQUAL(0xFFFFFFFFFFFFFFFF, (uint64_t) (range_one.limit >> 64));
}

BOOST_AUTO_TEST_CASE(test_slice_ranges)
{
    std::string input = "(slice \"foobar\" (range 1 2) (range 3 4))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));
    BOOST_CHECK_EQUAL(2, result.which());

    mtn::op_slice slice = boost::get<mtn::op_slice>(result);
    BOOST_CHECK_EQUAL("foobar", slice.index);
    BOOST_CHECK_EQUAL(2, slice.values.size());

    mtn::op_slice::iterator iter = slice.values.begin();
    BOOST_CHECK_EQUAL(0, iter->which());
    mtn::range_t range_one = boost::get<mtn::range_t>(*iter);

    BOOST_CHECK(1 == range_one.start);
    BOOST_CHECK(2 == range_one.limit);

    ++iter;

    BOOST_CHECK_EQUAL(0, iter->which());
    mtn::range_t range_two = boost::get<mtn::range_t>(*iter);

    BOOST_CHECK(3 == range_two.start);
    BOOST_CHECK(4 == range_two.limit);
}

BOOST_AUTO_TEST_CASE(test_slice_regex)
{
    std::string input = "(slice \"foobar\" (regex \"foo.*bar\"))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));
    BOOST_CHECK_EQUAL(2, result.which());

    mtn::op_slice slice = boost::get<mtn::op_slice>(result);
    BOOST_CHECK_EQUAL("foobar", slice.index);
    BOOST_CHECK_EQUAL(1, slice.values.size());

    BOOST_CHECK_EQUAL(1, slice.values.begin()->which());
    mtn::regex_t regex = boost::get<mtn::regex_t>(*slice.values.begin());
    BOOST_CHECK_EQUAL("foo.*bar", regex.pattern);
}

BOOST_AUTO_TEST_CASE(test_slice_range_regex)
{
    std::string input = "(slice \"foobar\" (range 1 2) (regex \"foo.*bar\"))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));
    BOOST_CHECK_EQUAL(2, result.which());

    mtn::op_slice slice = boost::get<mtn::op_slice>(result);
    BOOST_CHECK_EQUAL("foobar", slice.index);
    BOOST_CHECK_EQUAL(2, slice.values.size());

    mtn::op_slice::iterator iter = slice.values.begin();
    BOOST_CHECK_EQUAL(0, iter->which());
    mtn::range_t range_one = boost::get<mtn::range_t>(*iter);

    BOOST_CHECK(1 == range_one.start);
    BOOST_CHECK(2 == range_one.limit);

    ++iter;

    BOOST_CHECK_EQUAL(1, iter->which());
    mtn::regex_t regex = boost::get<mtn::regex_t>(*iter);
    BOOST_CHECK_EQUAL("foo.*bar", regex.pattern);
}

BOOST_AUTO_TEST_CASE(test_or_slice)
{
    std::string input = "(or (slice \"foobar\"))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));

    BOOST_CHECK_EQUAL(3, result.which());
    mtn::op_or orop = boost::get<mtn::op_or>(result);
    BOOST_CHECK_EQUAL(1, orop.children.size());

    mtn::op_or::iterator iter = orop.children.begin();

    BOOST_CHECK_EQUAL(2, iter->which());
    mtn::op_slice slice = boost::get<mtn::op_slice>(*iter);
    BOOST_CHECK_EQUAL("foobar", slice.index);
    BOOST_CHECK_EQUAL(0, slice.values.size());
}

BOOST_AUTO_TEST_CASE(test_or_slices)
{
    std::string input = "(or (slice \"foobar\") (slice \"bizbang\"))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));

    BOOST_CHECK_EQUAL(3, result.which());
    mtn::op_or orop = boost::get<mtn::op_or>(result);
    BOOST_CHECK_EQUAL(2, orop.children.size());

    mtn::op_or::iterator iter = orop.children.begin();

    BOOST_CHECK_EQUAL(2, iter->which());
    mtn::op_slice slice_one = boost::get<mtn::op_slice>(*iter);
    BOOST_CHECK_EQUAL("foobar", slice_one.index);
    BOOST_CHECK_EQUAL(0, slice_one.values.size());

    ++iter;

    BOOST_CHECK_EQUAL(2, iter->which());
    mtn::op_slice slice_two = boost::get<mtn::op_slice>(*iter);
    BOOST_CHECK_EQUAL("bizbang", slice_two.index);
    BOOST_CHECK_EQUAL(0, slice_two.values.size());
}

BOOST_AUTO_TEST_CASE(test_and_slices)
{
    std::string input = "(and (slice \"foobar\") (slice \"bizbang\"))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));

    BOOST_CHECK_EQUAL(5, result.which());
    mtn::op_and andop = boost::get<mtn::op_and>(result);
    BOOST_CHECK_EQUAL(2, andop.children.size());

    mtn::op_and::iterator iter = andop.children.begin();

    BOOST_CHECK_EQUAL(2, iter->which());
    mtn::op_slice slice_one = boost::get<mtn::op_slice>(*iter);
    BOOST_CHECK_EQUAL("foobar", slice_one.index);
    BOOST_CHECK_EQUAL(0, slice_one.values.size());

    ++iter;

    BOOST_CHECK_EQUAL(2, iter->which());
    mtn::op_slice slice_two = boost::get<mtn::op_slice>(*iter);
    BOOST_CHECK_EQUAL("bizbang", slice_two.index);
    BOOST_CHECK_EQUAL(0, slice_two.values.size());
}

BOOST_AUTO_TEST_CASE(test_xor_slices)
{
    std::string input = "(xor (slice \"foobar\") (slice \"bizbang\"))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));

    BOOST_CHECK_EQUAL(6, result.which());
    mtn::op_xor xorop = boost::get<mtn::op_xor>(result);
    BOOST_CHECK_EQUAL(2, xorop.children.size());

    mtn::op_xor::iterator iter = xorop.children.begin();

    BOOST_CHECK_EQUAL(2, iter->which());
    mtn::op_slice slice_one = boost::get<mtn::op_slice>(*iter);
    BOOST_CHECK_EQUAL("foobar", slice_one.index);
    BOOST_CHECK_EQUAL(0, slice_one.values.size());

    ++iter;

    BOOST_CHECK_EQUAL(2, iter->which());
    mtn::op_slice slice_two = boost::get<mtn::op_slice>(*iter);
    BOOST_CHECK_EQUAL("bizbang", slice_two.index);
    BOOST_CHECK_EQUAL(0, slice_two.values.size());
}

BOOST_AUTO_TEST_CASE(test_not_slice)
{
    std::string input = "(not (slice \"foobar\"))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));

    BOOST_CHECK_EQUAL(4, result.which());
    mtn::op_not notop = boost::get<mtn::op_not>(result);

    BOOST_CHECK_EQUAL(2, notop.child.which());
    mtn::op_slice slice = boost::get<mtn::op_slice>(notop.child);
    BOOST_CHECK_EQUAL("foobar", slice.index);
    BOOST_CHECK_EQUAL(0, slice.values.size());
}

BOOST_AUTO_TEST_CASE(test_and_or_slices)
{
    std::string input = "(and (or (slice \"foobar\") (slice \"bizbang\")) (or (slice \"foobar\") (slice \"bizbang\")))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));

    BOOST_CHECK_EQUAL(5, result.which());
    mtn::op_and andop = boost::get<mtn::op_and>(result);
    BOOST_CHECK_EQUAL(2, andop.children.size());

    mtn::op_and::iterator and_iter = andop.children.begin();

    {
        BOOST_CHECK_EQUAL(3, and_iter->which());
        mtn::op_or orop = boost::get<mtn::op_or>(*and_iter);
        BOOST_CHECK_EQUAL(2, orop.children.size());

        mtn::op_and::iterator or_iter = orop.children.begin();
        BOOST_CHECK_EQUAL(2, or_iter->which());
        mtn::op_slice slice_one = boost::get<mtn::op_slice>(*or_iter);
        BOOST_CHECK_EQUAL("foobar", slice_one.index);
        BOOST_CHECK_EQUAL(0, slice_one.values.size());

        ++or_iter;

        BOOST_CHECK_EQUAL(2, or_iter->which());
        mtn::op_slice slice_two = boost::get<mtn::op_slice>(*or_iter);
        BOOST_CHECK_EQUAL("bizbang", slice_two.index);
        BOOST_CHECK_EQUAL(0, slice_two.values.size());
    }

    ++and_iter;

    {
        BOOST_CHECK_EQUAL(3, and_iter->which());
        mtn::op_or orop = boost::get<mtn::op_or>(*and_iter);
        BOOST_CHECK_EQUAL(2, orop.children.size());

        mtn::op_and::iterator or_iter = orop.children.begin();
        BOOST_CHECK_EQUAL(2, or_iter->which());
        mtn::op_slice slice_one = boost::get<mtn::op_slice>(*or_iter);
        BOOST_CHECK_EQUAL("foobar", slice_one.index);
        BOOST_CHECK_EQUAL(0, slice_one.values.size());

        ++or_iter;

        BOOST_CHECK_EQUAL(2, or_iter->which());
        mtn::op_slice slice_two = boost::get<mtn::op_slice>(*or_iter);
        BOOST_CHECK_EQUAL("bizbang", slice_two.index);
        BOOST_CHECK_EQUAL(0, slice_two.values.size());
    }

}

BOOST_AUTO_TEST_CASE(test_not_and_or_slices)
{
    std::string input = "(not (and (or (slice \"foobar\") (slice \"bizbang\"))))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr result;
    BOOST_CHECK_EQUAL(0, result.which());
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, result));

    BOOST_CHECK_EQUAL(4, result.which());
    mtn::op_not notop = boost::get<mtn::op_not>(result);

    mtn::op_and andop = boost::get<mtn::op_and>(notop.child);
    BOOST_CHECK_EQUAL(1, andop.children.size());

    mtn::op_and::iterator and_iter = andop.children.begin();

    {
        BOOST_CHECK_EQUAL(3, and_iter->which());
        mtn::op_or orop = boost::get<mtn::op_or>(*and_iter);
        BOOST_CHECK_EQUAL(2, orop.children.size());

        mtn::op_and::iterator or_iter = orop.children.begin();
        BOOST_CHECK_EQUAL(2, or_iter->which());
        mtn::op_slice slice_one = boost::get<mtn::op_slice>(*or_iter);
        BOOST_CHECK_EQUAL("foobar", slice_one.index);
        BOOST_CHECK_EQUAL(0, slice_one.values.size());

        ++or_iter;

        BOOST_CHECK_EQUAL(2, or_iter->which());
        mtn::op_slice slice_two = boost::get<mtn::op_slice>(*or_iter);
        BOOST_CHECK_EQUAL("bizbang", slice_two.index);
        BOOST_CHECK_EQUAL(0, slice_two.values.size());
    }

}

BOOST_AUTO_TEST_SUITE_END()
