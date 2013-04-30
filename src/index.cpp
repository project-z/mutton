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

#include "index.hpp"


prz::index_t::index_t(index_partition_t partition,
        const char*       field,
        size_t            field_size)
{}

prz::status_t
prz::index_t::execute(index_operation_enum operation,
        index_t&             a_index,
        index_t&             b_index,
        prz::range_t*        ranges,
        index_slice_t&       output)
{
    return prz::status_t();
}

prz::status_t
prz::index_t::execute(index_operation_enum operation,
        index_t&             a_index,
        index_t&             b_index,
        index_slice_t&       output)
{
    return prz::status_t();
}

prz::status_t
prz::index_t::execute(prz::index_operation_enum operation,
        prz::index_reader_t*      reader,
        index_partition_t         partition,
        const byte_t*             field,
        size_t                    field_size,
        index_slice_t&            output)
{
    return prz::status_t();
}

prz::status_t
prz::index_t::execute(prz::index_operation_enum operation,
        prz::index_reader_t*      reader,
        index_partition_t         partition,
        const byte_t*             field,
        size_t                    field_size,
        prz::range_t*             ranges,
        index_slice_t&            output)
{
    return prz::status_t();
}

prz::index_partition_t
prz::index_t::partition() const
{
    return _partition;
}

const prz::byte_t*
prz::index_t::field() const
{
    return &_field[0];
}

size_t
prz::index_t::field_size() const
{
    return _field.size();
}
