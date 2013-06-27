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

#ifndef __MUTTON_INDEX_READER_WRITER_HPP_INCLUDED__
#define __MUTTON_INDEX_READER_WRITER_HPP_INCLUDED__

#include <boost/ptr_container/ptr_map.hpp>

#include "base_types.hpp"
#include "status.hpp"

namespace mtn {

    class index_t;
    class index_slice_t;

    class index_reader_writer_t
    {

    public:

        typedef boost::ptr_map<std::vector<mtn::byte_t>, mtn::index_t> index_container;

        virtual
        ~index_reader_writer_t()
        {}

        virtual mtn::status_t
        read_indexes(mtn_index_partition_t                        partition,
                     const std::vector<mtn::byte_t>&              start_bucket,
                     const std::vector<mtn::byte_t>&              start_field,
                     const std::vector<mtn::byte_t>&              end_bucket,
                     const std::vector<mtn::byte_t>&              end_field,
                     mtn::index_reader_writer_t::index_container& output) = 0;

        virtual mtn::status_t
        read_index(mtn_index_partition_t           partition,
                   const std::vector<mtn::byte_t>& bucket,
                   const std::vector<mtn::byte_t>& field,
                   mtn::index_t**                  output) = 0;

        virtual mtn::status_t
        read_index_slice(mtn_index_partition_t           partition,
                         const std::vector<mtn::byte_t>& bucket,
                         const std::vector<mtn::byte_t>& field,
                         mtn_index_address_t             value,
                         mtn::index_slice_t&             output) = 0;

        virtual mtn::status_t
        read_segment(mtn_index_partition_t           partition,
                     const std::vector<mtn::byte_t>& bucket,
                     const std::vector<mtn::byte_t>& field,
                     mtn_index_address_t             value,
                     mtn_index_address_t             offset,
                     mtn::index_segment_ptr          output) = 0;

        virtual mtn::status_t
        estimateSize(mtn_index_partition_t           partition,
                     const std::vector<mtn::byte_t>& bucket,
                     const std::vector<mtn::byte_t>& field,
                     mtn_index_address_t             value,
                     uint64_t*                       output) = 0;

        virtual mtn::status_t
        write_segment(mtn_index_partition_t           partition,
                      const std::vector<mtn::byte_t>& bucket,
                      const std::vector<mtn::byte_t>& field,
                      mtn_index_address_t             value,
                      mtn_index_address_t             offset,
                      index_segment_ptr input) = 0;
    };

} // namespace mtn

#endif // __MUTTON_INDEX_READER_WRITER_HPP_INCLUDED__
