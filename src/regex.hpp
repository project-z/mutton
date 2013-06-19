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

#ifndef __MUTTON_REGEX_HPP_INCLUDED__
#define __MUTTON_REGEX_HPP_INCLUDED__

#include <set>
#include <vector>

#include <boost/foreach.hpp>

#include <re2/filtered_re2.h>
#include <re2/stringpiece.h>
#include <re2/re2.h>

#include "base_types.hpp"
#include "range.hpp"
#include "status.hpp"
#include "trigram.hpp"

namespace mtn {

    struct regex_t
    {
        std::string pattern;

        static inline void
        to_ranges(const regex_t&             input,
                  std::vector<mtn::range_t>& output)
        {
            re2::FilteredRE2 re2;
            RE2::Options options;
            std::vector<std::string> pieces;

            int id = 0;
            re2.Add(re2::StringPiece(input.pattern), options, &id);
            re2.Compile(&pieces);

            BOOST_FOREACH(const std::string& p, pieces) {
                mtn::trigram_t::to_ranges(p.c_str(), p.c_str() + p.size(), output);
            }
        }
    };

} // namespace mtn


#endif // __MUTTON_REGEX_HPP_INCLUDED__
