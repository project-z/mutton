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
#include "trigram.hpp"

typedef boost::counting_iterator<prz::index_address_t> counting_iterator;

prz::index_t::index_t(prz::index_partition_t partition,
                      const char*            field,
                      size_t                 field_size) :
    _partition(partition),
    _field(field, field + field_size)
{}

prz::status_t
prz::index_t::slice(prz::range_t*       ranges,
                    size_t              range_count,
                    prz::index_slice_t& output)
{
    prz::status_t status;
    for (int r = 0; r < range_count; ++r) {
        BOOST_FOREACH(prz::index_address_t a, boost::make_iterator_range(counting_iterator(ranges[r].start), counting_iterator(ranges[r].limit))) {
            prz::index_t::iterator iter = find(a);

            if (iter != end()) {
                status = output.execute(prz::PRZ_INDEX_OP_UNION, *(iter->second), output, output);
                if (!status) {
                    return status;
                }
            }
        }
    }

    return status;
}

prz::status_t
prz::index_t::slice(prz::index_slice_t& output)
{
    prz::status_t status;

    for (prz::index_t::iterator iter = begin(); iter != end(); ++iter) {
        status = output.execute(prz::PRZ_INDEX_OP_UNION, *(iter->second), output, output);
        if (!status) {
            return status;
        }
    }

    return status;
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
        iter = insert(value, new prz::index_slice_t(_partition, &_field[0], _field.size(), value)).first;
    }

    iter->second->bit(reader, writer, who_or_what, state);
    return prz::status_t(); // XXX TODO better error handling
}

prz::status_t
prz::index_t::index_value_trigram(prz::index_reader_t* reader,
                                  prz::index_writer_t* writer,
                                  const char*          value,
                                  const char*          end,
                                  prz::index_address_t who_or_what,
                                  bool                 state)
{
    prz::status_t status;
    std::set<prz::index_address_t> trigrams;
    prz::trigram_t::to_trigrams(value, end, trigrams);

    BOOST_FOREACH(prz::index_address_t value, trigrams) {
        status = index_value(reader, writer, value, who_or_what, state);
        if (!status) {
            return status;
        }
    }
    return status;
}

prz::status_t
prz::index_t::index_value_hash(prz::index_reader_t* reader,
                               prz::index_writer_t* writer,
                               const char*          value,
                               size_t               len,
                               prz::index_address_t who_or_what,
                               bool                 state)
{
    return index_value(reader, writer, CityHash64(value, len), who_or_what, state);
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
