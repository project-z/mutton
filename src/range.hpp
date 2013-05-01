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

#ifndef __X_RANGE_HPP_INCLUDED__
#define __X_RANGE_HPP_INCLUDED__

#include <boost/iterator/counting_iterator.hpp>
#include <boost/range/iterator_range.hpp>

#include "base_types.hpp"

namespace prz {

    struct range_t {
        prz::index_address_t start;
        prz::index_address_t limit;

        typedef boost::counting_iterator<prz::index_address_t> counting_iterator;
        typedef boost::iterator_range<counting_iterator> iterator;

        range_t(const prz::index_address_t s,
                const prz::index_address_t l) :
            start(s),
            limit(l)
        {}

        inline bool
        in_range(prz::index_address_t value)
        {
            return value >= start && value < limit;
        }

        inline iterator
        get_iterator()
        {
            return boost::make_iterator_range(counting_iterator(start), counting_iterator(limit));
        }
    };

} // namespace prz

#endif // __X_RANGE_HPP_INCLUDED__
