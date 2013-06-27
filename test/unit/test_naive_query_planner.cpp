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
#include "context.hpp"
#include "naive_query_planner.hpp"
#include "query_parser.hpp"

BOOST_AUTO_TEST_SUITE(_query_planner)

BOOST_AUTO_TEST_CASE(test_slice)
{
    std::string input = "(slice \"foobar\")";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr query;
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, query));

    mtn::byte_t bucket_name_array[] = "bizbang";
    std::vector<mtn::byte_t> bucket(bucket_name_array, bucket_name_array + 7);

    mtn::byte_t field_name_array[] = "foobar";
    std::vector<mtn::byte_t> field(field_name_array, field_name_array + 6);

    index_reader_writer_memory_t rw;
    mtn::context_t<index_reader_writer_memory_t> context(new index_reader_writer_memory_t());
    context.index_value(1, bucket, field, 1, 2, true);

    mtn::naive_query_planner_t<mtn::context_t<index_reader_writer_memory_t> > planner(1, context, bucket);
    mtn::index_slice_t result = boost::apply_visitor(planner, query);
    BOOST_CHECK(planner.status());
    BOOST_CHECK_EQUAL(1, result.size());
    BOOST_CHECK(result.bit(2));
}

BOOST_AUTO_TEST_CASE(test_slice_range)
{
    std::string input = "(slice \"foobar\" (range 100 200))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr query;
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, query));

    mtn::byte_t bucket_name_array[] = "bizbang";
    std::vector<mtn::byte_t> bucket(bucket_name_array, bucket_name_array + 7);

    mtn::byte_t field_name_array[] = "foobar";
    std::vector<mtn::byte_t> field(field_name_array, field_name_array + 6);

    index_reader_writer_memory_t rw;
    mtn::context_t<index_reader_writer_memory_t> context(new index_reader_writer_memory_t());
    context.index_value(1, bucket, field, 1, 1, true);
    context.index_value(1, bucket, field, 100, 2, true);

    mtn::naive_query_planner_t<mtn::context_t<index_reader_writer_memory_t> > planner(1, context, bucket);
    mtn::index_slice_t result = boost::apply_visitor(planner, query);
    BOOST_CHECK(planner.status());
    BOOST_CHECK_EQUAL(1, result.size());
    BOOST_CHECK(!result.bit(1));
    BOOST_CHECK(result.bit(2));
}

BOOST_AUTO_TEST_CASE(test_slice_regex)
{
    std::string input = "(slice \"foobar\" (regex \"foo.*\"))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr query;
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, query));

    mtn::byte_t bucket_name_array[] = "bizbang";
    std::vector<mtn::byte_t> bucket(bucket_name_array, bucket_name_array + 7);

    mtn::byte_t field_name_array[] = "foobar";
    std::vector<mtn::byte_t> field(field_name_array, field_name_array + 6);

    index_reader_writer_memory_t rw;
    mtn::context_t<index_reader_writer_memory_t> context(new index_reader_writer_memory_t());

    std::string value = "foobar";
    context.index_value_trigram(1, bucket, field, value.begin(), value.end(), 1, true);

    mtn::naive_query_planner_t<mtn::context_t<index_reader_writer_memory_t> > planner(1, context, bucket);
    mtn::index_slice_t result = boost::apply_visitor(planner, query);
    BOOST_CHECK(planner.status());
    BOOST_CHECK_EQUAL(1, result.size());
    BOOST_CHECK(result.bit(1));
}

BOOST_AUTO_TEST_CASE(test_slice_regex_partial_trigram)
{
    std::string input = "(slice \"foobar\" (regex \"foob.*\"))";
    std::string::const_iterator f(input.begin());
    std::string::const_iterator l(input.end());
    mtn::query_parser_t<std::string::const_iterator> p;

    mtn::expr query;
    BOOST_CHECK(qi::phrase_parse(f, l, p, qi::space, query));

    mtn::byte_t bucket_name_array[] = "bizbang";
    std::vector<mtn::byte_t> bucket(bucket_name_array, bucket_name_array + 7);

    mtn::byte_t field_name_array[] = "foobar";
    std::vector<mtn::byte_t> field(field_name_array, field_name_array + 6);

    index_reader_writer_memory_t rw;
    mtn::context_t<index_reader_writer_memory_t> context(new index_reader_writer_memory_t());

    std::string value = "foobar";
    context.index_value_trigram(1, bucket, field, value.begin(), value.end(), 1, true);

    mtn::naive_query_planner_t<mtn::context_t<index_reader_writer_memory_t> > planner(1, context, bucket);
    mtn::index_slice_t result = boost::apply_visitor(planner, query);
    BOOST_CHECK(planner.status());
    BOOST_CHECK_EQUAL(1, result.size());
    BOOST_CHECK(result.bit(1));
}


BOOST_AUTO_TEST_SUITE_END()
