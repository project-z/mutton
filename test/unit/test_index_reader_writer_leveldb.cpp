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

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <leveldb/db.h>

#include "fixtures.hpp"
#include "context.hpp"
#include "index_reader_writer_leveldb.hpp"


BOOST_AUTO_TEST_SUITE(_index_reader_writer_leveldb)

struct auto_path_t {

    auto_path_t() :
        path("tmp/testdb")
    {
        boost::filesystem::create_directories(path);
    }

    ~auto_path_t()
    {
        boost::filesystem::remove_all(path);
    }

    std::string path;
};

BOOST_AUTO_TEST_CASE(index_partition)
{
    auto_path_t path;
    mtn::context_t context(new mtn::index_reader_writer_leveldb_t());

    context.set_opt(MTN_OPT_DB_PATH, static_cast<const void*>(path.path.c_str()), path.path.size());
    context.init();

    mtn::byte_t bucket_name_array[] = "bizbang";
    mtn::byte_t field_name_array[] = "foobar";

    std::vector<mtn::byte_t> bucket(bucket_name_array, bucket_name_array + 7);
    std::vector<mtn::byte_t> field(field_name_array, field_name_array + 6);

    mtn::index_t* index = NULL;
    context.index_reader_writer().read_index(1, bucket, field, &index);
    BOOST_CHECK(NULL != index);
    BOOST_CHECK_EQUAL(1, index->partition());
}

BOOST_AUTO_TEST_CASE(index_bucket)
{
    auto_path_t path;
    mtn::context_t context(new mtn::index_reader_writer_leveldb_t());

    context.set_opt(MTN_OPT_DB_PATH, static_cast<const void*>(path.path.c_str()), path.path.size());
    BOOST_CHECK(context.init());

    mtn::byte_t bucket_name_array[] = "bizbang";
    mtn::byte_t field_name_array[] = "foobar";

    std::vector<mtn::byte_t> bucket(bucket_name_array, bucket_name_array + 7);
    std::vector<mtn::byte_t> field(field_name_array, field_name_array + 6);

    mtn::index_t* index = NULL;
    context.index_reader_writer().read_index(1, bucket, field, &index);
    BOOST_CHECK(NULL != index);
    BOOST_CHECK_EQUAL(index->bucket().size(), bucket.size());
    BOOST_CHECK(std::equal(index->bucket().begin(), index->bucket().end(), bucket.begin()));
}

BOOST_AUTO_TEST_CASE(index_field)
{
    auto_path_t path;
    mtn::context_t context(new mtn::index_reader_writer_leveldb_t());

    context.set_opt(MTN_OPT_DB_PATH, static_cast<const void*>(path.path.c_str()), path.path.size());
    BOOST_CHECK(context.init());

    mtn::byte_t bucket_name_array[] = "bizbang";
    mtn::byte_t field_name_array[] = "foobar";

    std::vector<mtn::byte_t> bucket(bucket_name_array, bucket_name_array + 7);
    std::vector<mtn::byte_t> field(field_name_array, field_name_array + 6);

    mtn::index_t* index = NULL;
    context.index_reader_writer().read_index(1, bucket, field, &index);
    BOOST_CHECK(NULL != index);
    BOOST_CHECK_EQUAL(index->field().size(), field.size());
    BOOST_CHECK(std::equal(index->field().begin(), index->field().end(), field.begin()));
}

BOOST_AUTO_TEST_CASE(read_write_segment)
{
    auto_path_t path;
    mtn::context_t context(new mtn::index_reader_writer_leveldb_t());

    context.set_opt(MTN_OPT_DB_PATH, static_cast<const void*>(path.path.c_str()), path.path.size());
    context.init();

    mtn::byte_t bucket_name_array[] = "bizbang";
    mtn::byte_t field_name_array[] = "foobar";

    std::vector<mtn::byte_t> bucket(bucket_name_array, bucket_name_array + 7);
    std::vector<mtn::byte_t> field(field_name_array, field_name_array + 6);

    mtn::index_segment_t segment_one;
    mtn::index_segment_t segment_two;
    memset(segment_one, 0xFFFFFFFF, MTN_INDEX_SEGMENT_SIZE);
    memset(segment_two, 0, MTN_INDEX_SEGMENT_SIZE);

    mtn::status_t status;
    status = context.index_reader_writer().write_segment(1, bucket, field, 2, 3, segment_one);
    BOOST_CHECK(status);

    status = context.index_reader_writer().read_segment(1, bucket, field, 2, 3, segment_two);
    BOOST_CHECK(status);

    BOOST_CHECK_EQUAL(0, memcmp(segment_one, segment_two, MTN_INDEX_SEGMENT_SIZE));
}

BOOST_AUTO_TEST_CASE(slice_set_bit_simple)
{
    auto_path_t path;
    mtn::context_t context(new mtn::index_reader_writer_leveldb_t());

    context.set_opt(MTN_OPT_DB_PATH, static_cast<const void*>(path.path.c_str()), path.path.size());
    context.init();

    mtn::byte_t bucket_name_array[] = "bizbang";
    mtn::byte_t field_name_array[] = "foobar";

    std::vector<mtn::byte_t> bucket(bucket_name_array, bucket_name_array + 7);
    std::vector<mtn::byte_t> field(field_name_array, field_name_array + 6);

    mtn::index_t* index_one = NULL;
    context.index_reader_writer().read_index(1, bucket, field, &index_one);
    BOOST_CHECK(NULL != index_one);
    BOOST_CHECK(index_one->index_value(context.index_reader_writer(), 1, 1, true));
    BOOST_CHECK_EQUAL(1, index_one->size());

    mtn::index_slice_t slice_one;
    BOOST_CHECK(!slice_one.bit(1));
    BOOST_CHECK(index_one->slice(slice_one));
    BOOST_CHECK_EQUAL(1, slice_one.size());
    BOOST_CHECK(slice_one.bit(1));

    mtn::index_t* index_two = NULL;
    context.index_reader_writer().read_index(1, bucket, field, &index_two);
    BOOST_CHECK(NULL != index_two);

    mtn::index_slice_t slice_two;
    BOOST_CHECK(!slice_two.bit(1));
    BOOST_CHECK(index_two->slice(slice_two));
    BOOST_CHECK_EQUAL(1, slice_two.size());
    BOOST_CHECK(slice_two.bit(1));
}

BOOST_AUTO_TEST_CASE(slice_set_bit_larger_offset)
{
    auto_path_t path;
    mtn::context_t context(new mtn::index_reader_writer_leveldb_t());

    context.set_opt(MTN_OPT_DB_PATH, static_cast<const void*>(path.path.c_str()), path.path.size());
    context.init();

    mtn::byte_t bucket_name_array[] = "bizbang";
    mtn::byte_t field_name_array[] = "foobar";

    std::vector<mtn::byte_t> bucket(bucket_name_array, bucket_name_array + 7);
    std::vector<mtn::byte_t> field(field_name_array, field_name_array + 6);

    mtn::index_t* index_one = NULL;
    context.index_reader_writer().read_index(1, bucket, field, &index_one);
    BOOST_CHECK(NULL != index_one);
    BOOST_CHECK(index_one->index_value(context.index_reader_writer(), 2048, 1, true));

    mtn::index_slice_t slice_one;
    BOOST_CHECK(!slice_one.bit(1));
    BOOST_CHECK(index_one->slice(slice_one));
    BOOST_CHECK(slice_one.bit(1));

    mtn::index_t* index_two = NULL;
    context.index_reader_writer().read_index(1, bucket, field, &index_two);
    BOOST_CHECK(NULL != index_two);

    mtn::index_slice_t slice_two;
    BOOST_CHECK(!slice_two.bit(1));
    BOOST_CHECK(index_two->slice(slice_two));
    BOOST_CHECK(slice_two.bit(1));
}

BOOST_AUTO_TEST_CASE(slice_set_bit_larger_user)
{
    auto_path_t path;
    mtn::context_t context(new mtn::index_reader_writer_leveldb_t());

    context.set_opt(MTN_OPT_DB_PATH, static_cast<const void*>(path.path.c_str()), path.path.size());
    context.init();

    mtn::byte_t bucket_name_array[] = "bizbang";
    mtn::byte_t field_name_array[] = "foobar";

    std::vector<mtn::byte_t> bucket(bucket_name_array, bucket_name_array + 7);
    std::vector<mtn::byte_t> field(field_name_array, field_name_array + 6);

    mtn::index_t* index_one = NULL;
    context.index_reader_writer().read_index(1, bucket, field, &index_one);
    BOOST_CHECK(NULL != index_one);
    BOOST_CHECK(index_one->index_value(context.index_reader_writer(), 2048, 4096, true));

    mtn::index_slice_t slice_one;
    BOOST_CHECK(!slice_one.bit(4096));
    BOOST_CHECK(index_one->slice(slice_one));
    BOOST_CHECK(slice_one.bit(4096));

    mtn::index_t* index_two = NULL;
    context.index_reader_writer().read_index(1, bucket, field, &index_two);
    BOOST_CHECK(NULL != index_two);

    mtn::index_slice_t slice_two;
    BOOST_CHECK(!slice_two.bit(4096));
    BOOST_CHECK(index_two->slice(slice_two));
    BOOST_CHECK(slice_two.bit(4096));
}

BOOST_AUTO_TEST_SUITE_END()
