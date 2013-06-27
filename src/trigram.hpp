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

        template<class InputIterator>
        inline static InputIterator
        init(InputIterator input,
             InputIterator end,
             trigram_t&    output)
        {
            output.one = utf8::next(input, end);
            if (input == end) {
                return input;
            }

            output.two = utf8::next(input, end);
            if (input == end) {
                return input;
            }

            output.three = utf8::next(input, end);
            return input;
        }

        inline mtn_index_address_t
        hash()
        {
            return ((uint128_t) one ) << 64 | ((uint128_t) two) << 32 | ((uint128_t) three);
        }

        template<class InputIterator>
        static inline void
        to_trigrams(InputIterator first,
                    InputIterator last,
                    std::set<mtn_index_address_t>& output)
        {
            mtn::trigram_t trigram;
            for (;;) {
                first = mtn::trigram_t::init(first, last, trigram);
                output.insert(trigram.hash());
                if (first == last) {
                    break;
                }
                trigram.zero();
            }
        }

        static inline void
        to_ranges(std::set<mtn_index_address_t> trigrams,
                  std::vector<mtn::range_t>&     output)
        {
            output.reserve(trigrams.size());
            std::set<mtn_index_address_t>::iterator iter = trigrams.begin();
            for (;iter != trigrams.end(); ++iter) {
                if (((uint32_t) *iter) == 0) {
                    output.push_back(mtn::range_t(*iter, *iter | (((uint128_t) 0x0000000000000000) << 64 | 0x1111111111111111)));
                }
                else {
                    output.push_back(mtn::range_t(*iter, *iter + 1));
                }
            }
        }

        template<class InputIterator>
        static inline void
        to_ranges(InputIterator              first,
                  InputIterator              last,
                  std::vector<mtn::range_t>& output)
        {
            std::set<mtn_index_address_t> trigrams;
            to_trigrams(first, last, trigrams);
            to_ranges(trigrams, output);
        }

    };
#pragma pack(pop)

} // namespace mtn


#endif // __MUTTON_TRIGRAM_HPP_INCLUDED__
