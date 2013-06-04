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

#ifndef __MUTTON_TRIGRAM_HPP_INCLUDED__
#define __MUTTON_TRIGRAM_HPP_INCLUDED__

#include <set>

#include "utf8.h"

#include "base_types.hpp"
#include "range.hpp"
#include "status.hpp"

namespace mtn {

#pragma pack(push, 1)

    struct trigram_t {
        uint32_t one;
        uint32_t two;
        uint32_t three;

        trigram_t() :
            one(0),
            two(0),
            three(0)
        {}

        inline void
        zero()
        {
            one = 0;
            two = 0;
            three = 0;
        }

        inline static char*
        init(const char* input,
             const char* end,
             trigram_t*  output)
        {
            char* input_pos = const_cast<char*>(input);
            uint32_t* output_pos = reinterpret_cast<uint32_t*>(output);

            while (input_pos < end && output_pos < reinterpret_cast<uint32_t*>(output) + 3) {
                *output_pos = utf8::next(input_pos, const_cast<char*>(end));
                if (*output_pos == 0) {
                    continue;
                }
                output_pos += 1;
            }
            return input_pos;
        }

        inline mtn::index_address_t
        hash()
        {
            return ((uint128_t) one ) << 64 | ((uint128_t) two) << 32 | ((uint128_t) three);
        }

        static inline void
        to_trigrams(const char* start,
                    const char* end,
                    std::set<mtn::index_address_t>& output)
        {
            mtn::trigram_t trigram;
            char* pos = const_cast<char*>(start);

            for (;;) {
                pos = mtn::trigram_t::init(pos, end, &trigram);
                output.insert(trigram.hash());
                if (pos == end) {
                    break;
                }
                trigram.zero();
            }
        }

        static inline void
        to_ranges(std::set<mtn::index_address_t> trigrams,
                  std::vector<mtn::range_t>&     output)
        {
            output.reserve(trigrams.size());
            std::set<mtn::index_address_t>::iterator iter = trigrams.begin();
            for (;iter != trigrams.end(); ++iter) {
                output.push_back(mtn::range_t(*iter, *iter + 1));
            }
        }

        static inline void
        to_ranges(const char*                start,
                  const char*                end,
                  std::vector<mtn::range_t>& output)
        {
            std::set<mtn::index_address_t> trigrams;
            to_trigrams(start, end, trigrams);
            to_ranges(trigrams, output);
        }

    };
#pragma pack(pop)

} // namespace mtn


#endif // __MUTTON_TRIGRAM_HPP_INCLUDED__
