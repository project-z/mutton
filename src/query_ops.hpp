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

#ifndef __MUTTON_QUERY_OPS_HPP_INCLUDED__
#define __MUTTON_QUERY_OPS_HPP_INCLUDED__

#include <city.h>
#include <boost/variant/recursive_wrapper.hpp>

namespace mtn {

    typedef std::string quoted_string;
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
        op_range(uint64_t s, uint64_t l) : start(s), limit(l) {}
        op_range(const std::string& s) : start(hash(s)), limit(0) {}

        uint64_t
        hash(const std::string& input)
        {
            return CityHash64(input.c_str(), input.size());
        }

        uint64_t start;
        uint64_t limit;
    };

    typedef boost::variant<uint64_t,
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

} // namespace mtn

#endif // __MUTTON_QUERY_OPS_HPP_INCLUDED__
