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

#ifndef __MUTTON_QUERY_PARSER_HPP_INCLUDED__
#define __MUTTON_QUERY_PARSER_HPP_INCLUDED__

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/support_utree.hpp>
#include <boost/variant/recursive_wrapper.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>

#include <city.h>

#define BOOST_SPIRIT_UNICODE

namespace qi    = boost::spirit::qi;
namespace phx   = boost::phoenix;

namespace mtn {

typedef std::string quoted_string;
typedef int64_t integer;
struct op_and;
struct op_not;
struct op_or;
struct op_slice;
struct op_xor;

struct op_regex
{
    std::string pattern;
};

struct op_range
{
    op_range() : start(0), limit(0) {}
    op_range(integer s, integer l) : start(s), limit(l) {}
    op_range(const std::string& s) : start(hash(s)), limit(0) {}

    uint64_t
    hash(const std::string& input)
    {
        return CityHash64(input.c_str(), input.size());
    }

    integer start;
    integer limit;
};

typedef boost::variant<integer,
                       op_range,
                       op_slice,
                       op_regex,
                       boost::recursive_wrapper<op_or>,
                       boost::recursive_wrapper<op_not>,
                       boost::recursive_wrapper<op_and>,
                       boost::recursive_wrapper<op_xor>,
                       boost::recursive_wrapper<op_or>
                       > expr;

struct op_slice
{
    std::string index;
    std::vector<expr> values;
};

struct op_and
{
    std::vector<expr> children;
};

struct op_not
{
    expr child;
};

struct op_or
{
    std::vector<expr> children;
};

struct op_xor
{
    std::vector<expr> children;
};


template <typename Iterator, typename Skipper = qi::space_type>
struct query_parser_t : qi::grammar<Iterator, expr(), Skipper>
{
    query_parser_t()
        : query_parser_t::base_type(expr_)
    {
        qi::uint_parser<unsigned char, 16, 2, 2> hex2;

        expr_ = ('(' >> (slice_ | or_ | and_ | xor_ | not_)  >> ')');

        byte_string_ = qi::lexeme['#' > +hex2 > '#'];
        quoted_string_ %= qi::lexeme ['"' >> *(qi::char_ - qi::char_('\\') - qi::char_('"') | '\\' >> qi::char_) >> '"'];
        integer_ = boost::spirit::lexeme[qi::no_case["0x"] > qi::hex] | boost::spirit::lexeme['0' >> qi::oct] | qi::int_;

        // value_ = "(value"
        //     >> (integer_) [qi::_val = phx::construct<op_range>(qi::_1, 0)]
        //     >> (quoted_string_ | byte_string_) [phx::bind(&op_range::hash, qi::_val) = qi::_1]
        //     > ")";

        range_ = ("(range" > integer_ > integer_ > ")") [qi::_val = phx::construct<op_range>(qi::_1, qi::_2)];

        regex_ = ("(regex" > quoted_string_  > ")") [phx::bind(&op_regex::pattern, qi::_val) = qi::_1];

        slice_ = "slice"
            > (quoted_string_) [phx::bind(&op_slice::index, qi::_val) = qi::_1]
            > *(regex_ | range_ | value_) [phx::push_back(phx::bind(&op_slice::values, qi::_val), qi::_1)];

        and_ = "and"
            > +(expr_) [phx::push_back(phx::bind(&op_and::children, qi::_val), qi::_1)];

        not_ = "not"
            > (expr_) [phx::bind(&op_not::child, qi::_val) = qi::_1];

        or_ = "or"
            > +(expr_) [phx::push_back(phx::bind(&op_or::children, qi::_val), qi::_1)];

        xor_ = "xor"
            > +(expr_) [phx::push_back(phx::bind(&op_xor::children, qi::_val), qi::_1)];

        BOOST_SPIRIT_DEBUG_NODE(and_);
        BOOST_SPIRIT_DEBUG_NODE(expr_);
        BOOST_SPIRIT_DEBUG_NODE(integer_);
        BOOST_SPIRIT_DEBUG_NODE(not_);
        BOOST_SPIRIT_DEBUG_NODE(or_);
        BOOST_SPIRIT_DEBUG_NODE(range_);
        BOOST_SPIRIT_DEBUG_NODE(regex_);
        BOOST_SPIRIT_DEBUG_NODE(value_);
        BOOST_SPIRIT_DEBUG_NODE(xor_);
    }

  private:
    qi::rule<Iterator, expr(), Skipper>     expr_;
    qi::rule<Iterator, integer(), Skipper>  integer_;
    qi::rule<Iterator, op_and(), Skipper>   and_;
    qi::rule<Iterator, op_not(), Skipper>   not_;
    qi::rule<Iterator, op_or(), Skipper>    or_;
    qi::rule<Iterator, op_range(), Skipper> range_;
    qi::rule<Iterator, op_range(), Skipper> value_;
    qi::rule<Iterator, op_regex(), Skipper> regex_;
    qi::rule<Iterator, op_slice(), Skipper> slice_;
    qi::rule<Iterator, op_xor(), Skipper>   xor_;

    qi::rule<Iterator, boost::spirit::binary_string_type()>              byte_string_;
    qi::rule<Iterator, std::string(), qi::space_type, qi::locals<char> > quoted_string_;
};


} // namespace mtn

#endif // __MUTTON_QUERY_PARSER_HPP_INCLUDED__
