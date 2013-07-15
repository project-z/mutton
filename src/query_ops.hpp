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

#include <boost/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>

#include "range.hpp"
#include "regex.hpp"

namespace mtn {
    struct op_and;
    struct op_not;
    struct op_or;
    struct op_xor;
    struct op_slice;
    struct op_group;

    typedef boost::variant<mtn::range_t,
                           mtn::regex_t,
                           boost::recursive_wrapper<op_slice>,
                           boost::recursive_wrapper<op_or>,
                           boost::recursive_wrapper<op_not>,
                           boost::recursive_wrapper<op_and>,
                           boost::recursive_wrapper<op_xor>,
                           boost::recursive_wrapper<op_group>
                           > expr;

    struct op_group
    {
        expr        child;
        std::string index;
    };

    struct op_slice
    {
        typedef std::vector<expr>          collection;
        typedef collection::iterator       iterator;
        typedef collection::const_iterator const_iterator;

        inline std::vector<mtn::byte_t>
        to_vector() const
        {
            return std::vector<mtn::byte_t>(index.begin(), index.end());
        }

        std::string                        index;
        collection                         values;
    };

    struct op_and
    {
        typedef std::vector<expr>          collection;
        typedef collection::iterator       iterator;
        typedef collection::const_iterator const_iterator;

        collection                         children;
    };

    struct op_not
    {
        expr child;
    };

    struct op_or
    {
        typedef std::vector<expr>          collection;
        typedef collection::iterator       iterator;
        typedef collection::const_iterator const_iterator;

        collection                         children;
    };

    struct op_xor
    {
        typedef std::vector<expr>          collection;
        typedef collection::iterator       iterator;
        typedef collection::const_iterator const_iterator;

        collection                         children;
    };

} // namespace mtn

#endif // __MUTTON_QUERY_OPS_HPP_INCLUDED__
