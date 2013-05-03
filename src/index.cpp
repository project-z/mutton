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

#include <boost/foreach.hpp>

#include "range.hpp"
#include "index.hpp"

typedef boost::counting_iterator<prz::index_address_t> counting_iterator;

inline prz::status_t
union_range_behavior(prz::index_t&       a_index,
                     prz::index_t&       b_index,
                     prz::range_t*       ranges,
                     size_t              range_count,
                     prz::index_slice_t& output)
{
    prz::status_t status;
    for (int r = 0; r < range_count; ++r) {

        BOOST_FOREACH(prz::index_address_t a, boost::make_iterator_range(counting_iterator(ranges[r].start), counting_iterator(ranges[r].limit))) {
            prz::index_t::iterator a_iter = a_index.find(a);
            prz::index_t::iterator b_iter = b_index.find(a);

            if (a_iter != a_index.end()) {
                status = output.execute(prz::PRZ_INDEX_OP_UNION, *(a_iter->second), output, output);
                if (!status) {
                    return status;
                }
            }

            if (b_iter != b_index.end()) {
                status = output.execute(prz::PRZ_INDEX_OP_UNION, *(b_iter->second), output, output);
                if (!status) {
                    return status;
                }
            }
        }
    }
    return status;
}

inline prz::status_t
intersection_range_behavior(prz::index_t&       a_index,
                            prz::index_t&       b_index,
                            prz::range_t*       ranges,
                            size_t              range_count,
                            prz::index_slice_t& output)
{
    prz::status_t status;
    for (int r = 0; r < range_count; ++r) {

        BOOST_FOREACH(prz::index_address_t a, ranges[r].get_iterator()) {
            prz::index_t::iterator a_iter = a_index.find(a);
            prz::index_t::iterator b_iter = b_index.find(a);

            if (a_iter != a_index.end() && b_iter != b_index.end()) {
                status = output.execute(prz::PRZ_INDEX_OP_INTERSECTION, *(a_iter->second), output, output);
                if (!status) {
                    return status;
                }

                status = output.execute(prz::PRZ_INDEX_OP_INTERSECTION, *(b_iter->second), output, output);
                if (!status) {
                    return status;
                }
            }
        }
    }
    return status;
}


prz::index_t::index_t(index_partition_t partition,
                      const char*       field,
                      size_t            field_size)
{}

prz::status_t
prz::index_t::execute(index_operation_enum operation,
                      index_t&             a_index,
                      index_t&             b_index,
                      prz::range_t*        ranges,
                      size_t               range_count,
                      index_slice_t&       output)
{
    if (operation == PRZ_INDEX_OP_INTERSECTION) {
        return intersection_range_behavior(a_index, b_index, ranges, range_count, output);
    }
    else if (operation == PRZ_INDEX_OP_UNION) {
        return union_range_behavior(a_index, b_index, ranges, range_count, output);
    }
    return prz::status_t(PRZ_ERROR_INDEX_OPERATION, "unkown/unsupported index operation");
}

prz::status_t
prz::index_t::execute(index_operation_enum operation,
                      index_t&             a_index,
                      index_t&             b_index,
                      index_slice_t&       output)
{

    prz::status_t status;
    if (operation == PRZ_INDEX_OP_INTERSECTION) {
        BOOST_FOREACH(const prz::index_t::index_container::value_type& a_pair, a_index) {
            prz::index_t::iterator b_iter = b_index.find(a_pair.first);

            if (b_iter != b_index.end()) {
                status = output.execute(prz::PRZ_INDEX_OP_INTERSECTION, *(a_pair->second), output, output);
                if (!status) {
                    return status;
                }

                status = output.execute(prz::PRZ_INDEX_OP_INTERSECTION, *(b_iter->second), output, output);
                if (!status) {
                    return status;
                }
            }
        }
    }
    else if (operation == PRZ_INDEX_OP_UNION) {

        BOOST_FOREACH(const prz::index_t::index_container::value_type& a_pair, a_index) {
            status = output.execute(prz::PRZ_INDEX_OP_UNION, *(a_pair.second), output, output);
            if (!status) {
                return status;
            }
        }

        BOOST_FOREACH(const prz::index_t::index_container::value_type& b_pair, b_index) {
            status = output.execute(prz::PRZ_INDEX_OP_UNION, *(b_pair.second), output, output);
            if (!status) {
                return status;
            }
        }
    }
    else {
        return prz::status_t(PRZ_ERROR_INDEX_OPERATION, "unkown/unsupported index operation");
    }

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
                      size_t                    range_count,
                      index_slice_t&            output)
{
    return prz::status_t();
}

prz::status_t
prz::index_t::index_value(prz::index_reader_t* reader,
                          prz::index_writer_t* writer,
                          prz::index_address_t value,
                          prz::index_address_t who_or_what,
                          bool                 state)
{
    prz::index_t::iterator iter = _index.find(value);
    if (iter == _index.end()) {
        iter = insert(value, new prz::index_slice_t(_partition, &_field[0], _field.size(), who_or_what)).first;
    }
    return iter->second->bit(reader, writer, who_or_what, state);
}

prz::status_t
prz::index_t::indexed_value(prz::index_reader_t* reader,
                            prz::index_writer_t* writer,
                            prz::index_address_t value,
                            prz::index_address_t who_or_what,
                            bool*                state)
{
    prz::index_t::iterator iter = _index.find(value);
    if (iter == _index.end()) {
        *state = false;
    }
    else {
        *state = iter->second->bit(who_or_what);
    }
    return prz::status_t(); // XXX TODO better error handling
}

prz::status_t
prz::index_t::indexed_value(prz::index_reader_t* reader,
                            prz::index_writer_t* writer,
                            prz::index_address_t value,
                            prz::index_slice_t** who_or_what)
{
    prz::index_t::iterator iter = _index.find(value);
    if (iter == _index.end()) {
        *who_or_what = NULL;
    }
    else {
        *who_or_what = iter->second;
    }
    return prz::status_t(); // XXX TODO better error handling
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
