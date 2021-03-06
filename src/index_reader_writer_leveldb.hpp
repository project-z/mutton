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

#ifndef __MUTTON_INDEX_READER_WRITER_LEVELDB_HPP_INCLUDED__
#define __MUTTON_INDEX_READER_WRITER_LEVELDB_HPP_INCLUDED__

#include <leveldb/db.h>
#include "index_reader_writer.hpp"

namespace mtn {

    class index_slice_t;
    class context_t;

    class index_reader_writer_leveldb_t :
        public index_reader_writer_t
    {

    public:

        index_reader_writer_leveldb_t();

        ~index_reader_writer_leveldb_t();

        mtn::status_t
        init(mtn::context_t& context);

        mtn::status_t
        write_segment(mtn_index_partition_t           partition,
                      const std::vector<mtn::byte_t>& bucket,
                      const std::vector<mtn::byte_t>& field,
                      mtn_index_address_t             value,
                      mtn_index_address_t             offset,
                      mtn::index_segment_ptr          input);

        mtn::status_t
        read_indexes(mtn_index_partition_t                        partition,
                     const std::vector<mtn::byte_t>&              start_bucket,
                     const std::vector<mtn::byte_t>&              start_field,
                     const std::vector<mtn::byte_t>&              end_bucket,
                     const std::vector<mtn::byte_t>&              end_field,
                     mtn::index_reader_writer_t::index_container& output);

        mtn::status_t
        read_index(mtn_index_partition_t           partition,
                   const std::vector<mtn::byte_t>& bucket,
                   const std::vector<mtn::byte_t>& field,
                   mtn::index_t**                  output);

        mtn::status_t
        read_index_slice(mtn_index_partition_t           partition,
                         const std::vector<mtn::byte_t>& bucket,
                         const std::vector<mtn::byte_t>& field,
                         mtn_index_address_t             value,
                         mtn::index_slice_t&             output);

        mtn::status_t
        read_segment(mtn_index_partition_t           partition,
                     const std::vector<mtn::byte_t>& bucket,
                     const std::vector<mtn::byte_t>& field,
                     mtn_index_address_t             value,
                     mtn_index_address_t             offset,
                     mtn::index_segment_ptr          output);

        mtn::status_t
        estimateSize(mtn_index_partition_t           partition,
                     const std::vector<mtn::byte_t>& bucket,
                     const std::vector<mtn::byte_t>& field,
                     mtn_index_address_t             value,
                     uint64_t*                       output);

    private:
        leveldb::DB*          _db;
        leveldb::ReadOptions  _read_options;
        leveldb::WriteOptions _write_options;
    };

} // namespace mtn

#endif // __MUTTON_INDEX_READER_WRITER_LEVELDB_HPP_INCLUDED__
