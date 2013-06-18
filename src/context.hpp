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

#ifndef __MUTTON_CONTEXT_HPP_INCLUDED__
#define __MUTTON_CONTEXT_HPP_INCLUDED__

#include <boost/ptr_container/ptr_map.hpp>

#include "base_types.hpp"
#include "index.hpp"
#include "status.hpp"

namespace mtn {

    class index_reader_t;
    class index_writer_t;

    class context_t {
    public:

        typedef std::vector<mtn::byte_t> index_key_t;
        typedef boost::ptr_map<index_key_t, mtn::index_t> index_container_t;

        inline mtn::status_t
        get_index(mtn::index_partition_t          partition,
                  const std::vector<mtn::byte_t>& bucket,
                  const std::vector<mtn::byte_t>& field,
                  const mtn::index_t*             output)
        {
            index_key_t key;
            key.reserve(bucket.size() + field.size());
            key.insert(key.end(), bucket.begin(), bucket.end());
            key.insert(key.end(), field.begin(), field.end());

            mtn::status_t status;
            index_container_t::iterator iter = _indexes.find(key);
            if (iter != _indexes.end()) {
                output = iter->second;
                return status;
            }

            status.library = true;
            status.code = MTN_ERROR_NOT_FOUND;
            status.message = "index not found";
            return status;
        }

        inline mtn::status_t
        create_index(mtn::index_partition_t          partition,
                     const std::vector<mtn::byte_t>& bucket,
                     const std::vector<mtn::byte_t>& field,
                     const mtn::index_t*             output)
        {
            index_key_t key;
            key.reserve(bucket.size() + field.size());
            key.insert(key.end(), bucket.begin(), bucket.end());
            key.insert(key.end(), field.begin(), field.end());

            mtn::status_t status;
            index_container_t::iterator iter = _indexes.find(key);
            if (iter != _indexes.end()) {
                output = iter->second;
                return status;
            }

            std::pair<index_container_t::iterator, bool> insert_result
                = _indexes.insert(key, new mtn::index_t(partition, bucket, field));

            if (insert_result.second) {
                output = insert_result.first->second;
                return status;
            }

            status.library = true;
            status.code = MTN_ERROR_UNKOWN;
            status.message = "could not create new index";
            return status;
        }

    private:
        index_container_t _indexes;
    };

} // namespace mtn

#endif // __MUTTON_CONTEXT_HPP_INCLUDED__
