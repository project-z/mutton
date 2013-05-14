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

#ifndef __X_INDEX_READER_WRITER_LEVELDB_HPP_INCLUDED__
#define __X_INDEX_READER_WRITER_LEVELDB_HPP_INCLUDED__

#include <leveldb/db.h>

#include "index_reader.hpp"
#include "index_writer.hpp"

namespace mtn {

    class index_slice_t;

    class index_reader_writer_leveldb_t :
        public index_reader_t,
        public index_writer_t
    {

    public:

        index_reader_writer_leveldb_t(leveldb::DB*          db,
                                      leveldb::ReadOptions  read_options,
                                      leveldb::WriteOptions write_options);

        mtn::status_t
        write_segment(mtn::index_partition_t partition,
                      const mtn::byte_t*     field,
                      size_t                 field_size,
                      mtn::index_address_t   value,
                      mtn::index_address_t   offset,
                      mtn::index_segment_ptr input);

        mtn::status_t
        read_index(mtn::index_partition_t partition,
                   const mtn::byte_t*     field,
                   size_t                 field_size,
                   mtn::index_t*          output);

        mtn::status_t
        read_index_slice(mtn::index_partition_t partition,
                         const mtn::byte_t*     field,
                         size_t                 field_size,
                         mtn::index_address_t   value,
                         mtn::index_slice_t*    output);

        mtn::status_t
        read_segment(mtn::index_partition_t partition,
                     const mtn::byte_t*     field,
                     size_t                 field_size,
                     mtn::index_address_t   value,
                     mtn::index_address_t   offset,
                     mtn::index_segment_ptr output);

        mtn::status_t
        estimateSize(mtn::index_partition_t partition,
                     const mtn::byte_t*     field,
                     size_t                 field_size,
                     mtn::index_address_t   value,
                     uint64_t*              output);

    private:
        leveldb::DB*          _db;
        leveldb::ReadOptions  _read_options;
        leveldb::WriteOptions _write_options;
    };

} // namespace mtn

#endif // __X_INDEX_READER_WRITER_LEVELDB_HPP_INCLUDED__
