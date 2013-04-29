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

#ifndef __X_BASE_TYPES_HPP_INCLUDED__
#define __X_BASE_TYPES_HPP_INCLUDED__

#define INDEX_SEGMENT_LENGTH 32
#define INDEX_SEGMENT_SIZE INDEX_SEGMENT_LENGTH * sizeof(uint64_t)

namespace prz {
    typedef uint8_t       index_partition_t;
    typedef uint64_t      index_address_t;
    typedef uint64_t      index_segment_t[INDEX_SEGMENT_LENGTH];
    typedef uint64_t*     index_segment_ptr;
    typedef unsigned char byte_t;

} // namespace prz

#endif // __X_BASE_TYPES_HPP_INCLUDED__
