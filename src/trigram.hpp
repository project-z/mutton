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

#ifndef __X_TRIGRAM_HPP_INCLUDED__
#define __X_TRIGRAM_HPP_INCLUDED__

#include <set>

#include "city.h"
#include "utf8.h"

#include "base_types.hpp"
#include "status.hpp"

namespace prz {

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

        inline prz::index_address_t
        hash()
        {
            return CityHash64(reinterpret_cast<char*>(this), sizeof(uint32_t) * 3);
        }

        static inline void
        to_trigrams(const char* start,
                    const char* end,
                    std::set<prz::index_address_t>& output)
        {
            prz::trigram_t trigram;
            char* pos = const_cast<char*>(start);

            for (;;) {
                pos = prz::trigram_t::init(pos, end, &trigram);
                output.insert(trigram.hash());
                if (pos == end) {
                    break;
                }
                trigram.zero();
            }
        }

    };
#pragma pack(pop)

} // namespace prz


#endif // __X_TRIGRAM_HPP_INCLUDED__
