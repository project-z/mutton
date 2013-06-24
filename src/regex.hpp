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

        regex_t() :
            pattern("")
        {}

        regex_t(const std::string& pattern) :
            pattern(pattern)
        {}

        static inline mtn::status_t
        to_pieces(const regex_t&            input,
                  std::vector<std::string>& output)
        {
            re2::FilteredRE2 re2;
            RE2::Options options;

            int id = 0;
            RE2::ErrorCode ec = re2.Add(re2::StringPiece(input.pattern), options, &id);

            switch(ec) {
            case RE2::NoError:
                break;
            case RE2::ErrorBadEscape:
                return mtn::status_t(MTN_ERROR_BAD_REGEX, "bad escape sequence");
            case RE2::ErrorBadCharClass:
                return mtn::status_t(MTN_ERROR_BAD_REGEX, "bad character class");
            case RE2::ErrorBadCharRange:
                return mtn::status_t(MTN_ERROR_BAD_REGEX, "bad character class range");
            case RE2::ErrorMissingBracket:
                return mtn::status_t(MTN_ERROR_BAD_REGEX, "missing closing ]");
            case RE2::ErrorMissingParen:
                return mtn::status_t(MTN_ERROR_BAD_REGEX, "missing closing )");
            case RE2::ErrorTrailingBackslash:
                return mtn::status_t(MTN_ERROR_BAD_REGEX, "trailing \\ at end of regexp");
            case RE2::ErrorRepeatArgument:
                return mtn::status_t(MTN_ERROR_BAD_REGEX, "repeat argument missing: e.g. '*'");
            case RE2::ErrorRepeatSize:
                return mtn::status_t(MTN_ERROR_BAD_REGEX, "bad repetition argument");
            case RE2::ErrorRepeatOp:
                return mtn::status_t(MTN_ERROR_BAD_REGEX, "bad repetition operator");
            case RE2::ErrorBadPerlOp:
                return mtn::status_t(MTN_ERROR_BAD_REGEX, "bad perl operator");
            case RE2::ErrorBadUTF8:
                return mtn::status_t(MTN_ERROR_BAD_REGEX, "invalid UTF-8 in regexp");
            case RE2::ErrorBadNamedCapture:
                return mtn::status_t(MTN_ERROR_BAD_REGEX, "bad named capture group");
            case RE2::ErrorPatternTooLarge:
                return mtn::status_t(MTN_ERROR_BAD_REGEX, "pattern too large (compile failed)");
            default:
                std::stringstream ss;
                ss << "unknown regex error: " << ec;
                return mtn::status_t(MTN_ERROR_BAD_REGEX, ss.str());
            }

            re2.Compile(&output);
            return mtn::status_t();
        }

        static inline mtn::status_t
        to_ranges(const regex_t&             input,
                  std::vector<mtn::range_t>& output)
        {
            std::vector<std::string> pieces;
            mtn::status_t status = to_pieces(input, pieces);
            if (status) {
                for (std::vector<std::string>::iterator iter = pieces.begin(); iter != pieces.end(); ++iter) {
                    mtn::trigram_t::to_ranges(iter->c_str(), iter->c_str() + iter->size(), output);
                }
            }
            return status;
        }
    };

} // namespace mtn


#endif // __MUTTON_REGEX_HPP_INCLUDED__
