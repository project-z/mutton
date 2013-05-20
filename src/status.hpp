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

#ifndef __MUTTON_STATUS_H_
#define __MUTTON_STATUS_H_

#include <string>
#include "libcql/cql.hpp"

#define MTN_ERROR_INDEX_OPERATION 1
#define MTN_ERROR_NOT_FOUND 2

namespace mtn {

    struct status_t
    {
        bool        library;
        bool        local_storage;
        int         code;
        std::string message;

        status_t() :
            library(false),
            local_storage(false),
            code(0)
        {}

        status_t(int code,
                const std::string& message) :
            library(true),
            local_storage(false),
            code(code),
            message(message)
        {}

        inline operator
        bool() const
        {
            return ok();
        }

        inline bool
        ok() const
        {
            return !(library || local_storage);
        }
    };
} // namespace mtn

#endif // __MUTTON_STATUS_H_
