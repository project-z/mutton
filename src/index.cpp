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

#include <boost/foreach.hpp>

#include "range.hpp"
#include "index.hpp"
#include "trigram.hpp"

typedef boost::counting_iterator<mtn::index_address_t> counting_iterator;

mtn::index_t::index_t(mtn::index_partition_t partition,
                      const char*            field,
                      size_t                 field_size) :
    _partition(partition),
    _field(field, field + field_size)
{}

mtn::status_t
mtn::index_t::slice(mtn::index_address_t*     trigrams,
                    size_t                    trigrams_count,
                    mtn::index_operation_enum operation,
                    mtn::index_slice_t&       output)
{
    std::vector<mtn::range_t> ranges(trigrams_count, mtn::range_t());
    for (int i = 0; i < trigrams_count; ++i) {
        ranges[i].start = trigrams[i];
        ranges[i].limit = trigrams[i] + 1;
    }

    return slice(&ranges[0], trigrams_count, operation, output);
}

mtn::status_t
mtn::index_t::slice(mtn::range_t*             ranges,
                    size_t                    range_count,
                    mtn::index_operation_enum operation,
                    mtn::index_slice_t&       output)
{
    mtn::status_t status;
    bool first_iteration = true;

    for (int r = 0; r < range_count; ++r) {
        BOOST_FOREACH(mtn::index_address_t a, boost::make_iterator_range(counting_iterator(ranges[r].start), counting_iterator(ranges[r].limit))) {
            mtn::index_t::iterator iter = find(a);

            if (iter != end()) {
                if (first_iteration) {
                    output = *(iter->second);
                    first_iteration = false;
                }
                else {
                    status = output.execute(operation, *(iter->second), output, output);
                    if (!status) {
                        return status;
                    }
                }
            }
        }
    }

    return status;
}

mtn::status_t
mtn::index_t::slice(mtn::range_t*             ranges,
                    size_t                    range_count,
                    mtn::index_slice_t&       output)
{
    return slice(ranges, range_count, MTN_INDEX_OP_UNION, output);
}

mtn::status_t
mtn::index_t::slice(mtn::index_slice_t& output)
{
    mtn::status_t status;

    for (mtn::index_t::iterator iter = begin(); iter != end(); ++iter) {
        status = output.execute(mtn::MTN_INDEX_OP_UNION, *(iter->second), output, output);
        if (!status) {
            return status;
        }
    }

    return status;
}

mtn::status_t
mtn::index_t::index_value(mtn::index_reader_t* reader,
                          mtn::index_writer_t* writer,
                          mtn::index_address_t value,
                          mtn::index_address_t who_or_what,
                          bool                 state)
{
    mtn::index_t::iterator iter = _index.find(value);
    if (iter == _index.end()) {
        iter = insert(value, new mtn::index_slice_t(_partition, &_field[0], _field.size(), value)).first;
    }

    iter->second->bit(reader, writer, who_or_what, state);
    return mtn::status_t(); // XXX TODO better error handling
}

mtn::status_t
mtn::index_t::index_value_trigram(mtn::index_reader_t* reader,
                                  mtn::index_writer_t* writer,
                                  const char*          value,
                                  const char*          end,
                                  mtn::index_address_t who_or_what,
                                  bool                 state)
{
    mtn::status_t status;
    std::set<mtn::index_address_t> trigrams;
    mtn::trigram_t::to_trigrams(value, end, trigrams);

    BOOST_FOREACH(mtn::index_address_t value, trigrams) {
        status = index_value(reader, writer, value, who_or_what, state);
        if (!status) {
            return status;
        }
    }
    return status;
}

mtn::status_t
mtn::index_t::index_value_hash(mtn::index_reader_t* reader,
                               mtn::index_writer_t* writer,
                               const char*          value,
                               size_t               len,
                               mtn::index_address_t who_or_what,
                               bool                 state)
{
    return index_value(reader, writer, CityHash64(value, len), who_or_what, state);
}

mtn::status_t
mtn::index_t::indexed_value(mtn::index_reader_t* reader,
                            mtn::index_writer_t* writer,
                            mtn::index_address_t value,
                            mtn::index_address_t who_or_what,
                            bool*                state)
{
    mtn::index_t::iterator iter = _index.find(value);
    if (iter == _index.end()) {
        *state = false;
    }
    else {
        *state = iter->second->bit(who_or_what);
    }
    return mtn::status_t(); // XXX TODO better error handling
}

mtn::status_t
mtn::index_t::indexed_value(mtn::index_reader_t* reader,
                            mtn::index_writer_t* writer,
                            mtn::index_address_t value,
                            mtn::index_slice_t** who_or_what)
{
    mtn::index_t::iterator iter = _index.find(value);
    if (iter == _index.end()) {
        *who_or_what = NULL;
    }
    else {
        *who_or_what = iter->second;
    }
    return mtn::status_t(); // XXX TODO better error handling
}

mtn::index_partition_t
mtn::index_t::partition() const
{
    return _partition;
}

const mtn::byte_t*
mtn::index_t::field() const
{
    return &_field[0];
}

size_t
mtn::index_t::field_size() const
{
    return _field.size();
}
