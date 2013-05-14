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

#ifndef __MUTTON_BASE_TYPES_HPP_INCLUDED__
#define __MUTTON_BASE_TYPES_HPP_INCLUDED__

#define MTN_INDEX_SEGMENT_LENGTH 32
#define MTN_INDEX_SEGMENT_SIZE MTN_INDEX_SEGMENT_LENGTH * sizeof(uint64_t)

namespace mtn {
    typedef uint16_t      index_partition_t;
    typedef uint64_t      index_address_t;
    typedef uint64_t      index_segment_t[MTN_INDEX_SEGMENT_LENGTH];
    typedef uint64_t*     index_segment_ptr;
    typedef unsigned char byte_t;

    enum index_operation_enum {
        MTN_INDEX_OP_INTERSECTION = 0,
        MTN_INDEX_OP_UNION = 1
    };

} // namespace mtn

#endif // __MUTTON_BASE_TYPES_HPP_INCLUDED__
