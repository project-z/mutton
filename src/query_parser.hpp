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

#include "query_ops.hpp"

#define BOOST_SPIRIT_UNICODE

namespace qi    = boost::spirit::qi;
namespace phx   = boost::phoenix;

namespace mtn {

    template <typename Iterator, typename Skipper = qi::space_type>
    struct query_parser_t
        : qi::grammar<Iterator, expr(), Skipper>
    {
        query_parser_t()
            : query_parser_t::base_type(expr_)
        {
            qi::uint_parser<unsigned char, 16, 2, 2> hex2;
            qi::uint_parser<uint128_t, 10, 1, 39> uint;

            expr_ = ('(' >> (slice_ | or_ | and_ | xor_ | not_)  >> ')');

            byte_string_ = qi::lexeme['#' > +hex2 > '#'];
            quoted_string_ %= qi::lexeme ['"' >> *(qi::char_ - qi::char_('\\') - qi::char_('"') | '\\' >> qi::char_) >> '"'];
            uint_ = boost::spirit::lexeme[qi::no_case["0x"] > qi::hex] | uint;

            range_ = ("(range" > uint_ > uint_ > ")") [qi::_val = phx::construct<mtn::range_t>(qi::_1, qi::_2)];

            regex_ = ("(regex" > quoted_string_  > ")") [phx::bind(&mtn::regex_t::pattern, qi::_val) = qi::_1];

            slice_ = "slice"
                > (quoted_string_) [phx::bind(&op_slice::index, qi::_val) = qi::_1]
                > *(regex_ | range_) [phx::push_back(phx::bind(&op_slice::values, qi::_val), qi::_1)];

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
            BOOST_SPIRIT_DEBUG_NODE(uint_);
            BOOST_SPIRIT_DEBUG_NODE(not_);
            BOOST_SPIRIT_DEBUG_NODE(or_);
            BOOST_SPIRIT_DEBUG_NODE(range_);
            BOOST_SPIRIT_DEBUG_NODE(regex_);
            BOOST_SPIRIT_DEBUG_NODE(xor_);
        }

    private:
        qi::rule<Iterator, expr(), Skipper>         expr_;
        qi::rule<Iterator, uint128_t(), Skipper>    uint_;
        qi::rule<Iterator, op_and(), Skipper>       and_;
        qi::rule<Iterator, op_not(), Skipper>       not_;
        qi::rule<Iterator, op_or(), Skipper>        or_;
        qi::rule<Iterator, mtn::range_t(), Skipper> range_;
        qi::rule<Iterator, mtn::regex_t(), Skipper> regex_;
        qi::rule<Iterator, op_slice(), Skipper>     slice_;
        qi::rule<Iterator, op_xor(), Skipper>       xor_;

        qi::rule<Iterator, boost::spirit::binary_string_type()>              byte_string_;
        qi::rule<Iterator, std::string(), qi::space_type, qi::locals<char> > quoted_string_;
    };


} // namespace mtn

#endif // __MUTTON_QUERY_PARSER_HPP_INCLUDED__
