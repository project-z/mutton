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

#ifndef __X_INDEX_READER_WRITER_LEVELDB_HPP_INCLUDED__
#define __X_INDEX_READER_WRITER_LEVELDB_HPP_INCLUDED__

#include <leveldb/db.h>

#include "index_reader.hpp"
#include "index_writer.hpp"

namespace prz {

    class index_slice_t;

    class index_reader_writer_leveldb_t :
        public index_reader_t,
        public index_writer_t
    {

    public:

        index_reader_writer_leveldb_t(leveldb::DB*          db,
                                      leveldb::ReadOptions  read_options,
                                      leveldb::WriteOptions write_options);

        prz::status_t
        write_segment(prz::index_partition_t partition,
                      const prz::byte_t*     field,
                      size_t                 field_size,
                      prz::index_address_t   value,
                      prz::index_address_t   offset,
                      prz::index_segment_ptr input);

        prz::status_t
        read_index(prz::index_partition_t partition,
                   const prz::byte_t*     field,
                   size_t                 field_size,
                   prz::index_address_t   value,
                   prz::index_slice_t*          output);

        prz::status_t
        read_segment(prz::index_partition_t partition,
                     const prz::byte_t*     field,
                     size_t                 field_size,
                     prz::index_address_t   value,
                     prz::index_address_t   offset,
                     prz::index_segment_ptr output);

        prz::status_t
        estimateSize(prz::index_partition_t partition,
                     const prz::byte_t*     field,
                     size_t                 field_size,
                     prz::index_address_t   value,
                     uint64_t*              output);

    private:
        leveldb::DB*          _db;
        leveldb::ReadOptions  _read_options;
        leveldb::WriteOptions _write_options;
    };

} // namespace prz

#endif // __X_INDEX_READER_WRITER_LEVELDB_HPP_INCLUDED__
