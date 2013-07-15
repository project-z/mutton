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

#ifndef __MUTTON_NAIVE_QUERY_PLANNER_HPP_INCLUDED__
#define __MUTTON_NAIVE_QUERY_PLANNER_HPP_INCLUDED__

#include "context.hpp"
#include "index.hpp"
#include "index_slice.hpp"
#include "query_ops.hpp"

namespace mtn {

    struct range_visitor_t :
        boost::static_visitor<void>
    {

        range_visitor_t(std::vector<mtn::range_t>& ranges) :
            ranges(ranges)
        {}

        void
        operator()(const mtn::op_and&)
        {
            throw "shouldn't happen";
        }

        void
        operator()(const mtn::op_or&)
        {
            throw "shouldn't happen";
        }

        void
        operator()(const mtn::op_xor&)
        {
            throw "shouldn't happen";
        }

        void
        operator()(const mtn::op_not&)
        {
            throw "shouldn't happen";
        }

        void
        operator()(const mtn::op_slice&)
        {
            throw "shouldn't happen";
        }

        void
        operator()(const mtn::op_group&)
        {
            throw "shouldn't happen";
        }

        void
        operator()(const mtn::range_t& r)
        {
            ranges.push_back(r);
        }

        void
        operator()(const mtn::regex_t& r)
        {
            mtn::regex_t::to_ranges(r, ranges);
        }

        std::vector<mtn::range_t>& ranges;
    };


    struct naive_query_planner_t :
    boost::static_visitor<mtn::index_slice_t>
    {

        naive_query_planner_t(mtn_index_partition_t      partition,
                              mtn::context_t&            context,
                              const std::vector<byte_t>& bucket) :
            _partition(partition),
            _context(context),
            _bucket(bucket)
        {};

        mtn::index_slice_t
        operator()(const mtn::op_or& o)
        {
            mtn::index_slice_t result;
            mtn::op_or::const_iterator iter = o.children.begin();
            for (; iter != o.children.end(); ++iter) {
                if (!_status) {
                    break;
                }
                mtn::index_slice_t temp_slice = boost::apply_visitor(*this, *iter);
                _status = mtn::index_slice_t::execute(MTN_INDEX_OP_UNION, temp_slice, result, result);
            }
            return result;
        }

        mtn::index_slice_t
        operator()(const mtn::op_and& o)
        {
            mtn::index_slice_t result;
            mtn::op_and::const_iterator iter = o.children.begin();
            for (; iter != o.children.end(); ++iter) {
                if (!_status) {
                    break;
                }
                mtn::index_slice_t temp_slice = boost::apply_visitor(*this, *iter);
                _status = mtn::index_slice_t::execute(MTN_INDEX_OP_INTERSECTION, temp_slice, result, result);
            }
            return result;
        }

        mtn::index_slice_t
        operator()(const mtn::op_xor& o)
        {
            mtn::index_slice_t result;
            mtn::op_xor::const_iterator iter = o.children.begin();
            for (; iter != o.children.end(); ++iter) {
                if (!_status) {
                    break;
                }
                mtn::index_slice_t temp_slice = boost::apply_visitor(*this, *iter);
                _status = mtn::index_slice_t::execute(MTN_INDEX_OP_SYMMETRIC_DIFFERENCE, temp_slice, result, result);
            }
            return result;
        }

        mtn::index_slice_t
        operator()(const mtn::op_not&)
        {
            throw "XXX TODO fix me";
        }

        mtn::index_slice_t
        operator()(const mtn::op_group&)
        {
            throw "XXX TODO fix me";
        }

        mtn::index_slice_t
        operator()(const mtn::range_t&)
        {
            throw "shouldn't happen";
        }

        mtn::index_slice_t
        operator()(const mtn::regex_t&)
        {
            throw "shouldn't happen";
        }

        mtn::index_slice_t
        operator()(const mtn::op_slice& o)
        {
            mtn::index_slice_t result;
            if (!_status) {
                return result;
            }

            mtn::index_t* index = NULL;
            _status = _context.get_index(_partition, _bucket, o.to_vector(), &index);
            if (!_status) {
                return result;
            }

            if (o.values.empty()) {
                index->slice(result);
            }
            else {
                std::vector<mtn::range_t> ranges;
                range_visitor_t visitor(ranges);

                mtn::op_slice::const_iterator iter = o.values.begin();
                for (; iter != o.values.end(); ++iter) {
                    boost::apply_visitor(visitor, *iter);
                }

                index->slice(&ranges[0],
                             ranges.size(),
                             MTN_INDEX_OP_UNION,
                             result);
            }
            return result;
        }

        inline const mtn::status_t&
        status()
        {
            return _status;
        }

    private:
        mtn::status_t            _status;
        mtn_index_partition_t    _partition;
        mtn::context_t&          _context;
        std::vector<mtn::byte_t> _bucket;
    };

} // namespace mtn

#endif // __MUTTON_NAIVE_QUERY_PLANNER_HPP_INCLUDED__
