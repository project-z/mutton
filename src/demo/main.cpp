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

#include <iostream>
#include <list>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

#include "libmutton/mutton.h"

const static char* LUA_SCRIPT_PATH = "src/demo/lua_scripts";
const static char* LUA_SCRIPT_EXT = ".lua";

const static char* BASIC_EVENT_NAME = "basic";
const static char* BASIC_EVENT_JSON = "{\"a_field\":\"TROLOLOL I'm an Event!!!\"}";

const static mtn_index_partition_t INDEX_PARTITION = 1;

const static char* BUCKET_NAME = "i am a bucket";


bool
check_status(
    void*              context,
    bool               success,
    void*              status,
    const std::string& header)
{
    if (success) {
        return true;
    }

    char* message = NULL;
    mutton_status_get_message(context, status, &message);
    std::cout << header << std::endl;
    std::cout << "    error code: " << mutton_status_get_code(context, status) << std::endl;
    std::cout << "    error message: " << message << std::endl;
    delete message;
    return false;
}

std::list<boost::filesystem::path>
scan_dir_for_files(
    const std::string& path,
    const std::string& extension)
{
    std::list<boost::filesystem::path> files;

    std::cout << boost::filesystem::current_path() << std::endl;
    boost::filesystem::path full_path = boost::filesystem::system_complete(boost::filesystem::path(path));

    boost::filesystem::directory_iterator iter(full_path);
    boost::filesystem::directory_iterator end;

    std::cout << "scanning " << full_path << " for Lua scripts..." << std::endl;
    for (; iter != end; ++iter) {
        if (iter->path().extension() == extension) {
            std::cout << "    found " << iter->path() << std::endl;
            files.push_back(*iter);
        }
    }

    return files;
}

int
main()
{
    void* status = NULL;
    boost::shared_ptr<void> context(mutton_new_context(), mutton_free_context);

    if (!check_status(
            context.get(),
            mutton_set_opt(context.get(), MTN_OPT_DB_PATH, (void*) "tmp/demo", 8, &status),
            status,
            "error setting option:"))
    {
        return -1;
    }

    if (!check_status(
            context.get(),
            mutton_init_context(context.get(), &status),
            status,
            "error initializing context:"))
    {
        return -1;
    }

    std::list<boost::filesystem::path> lua_scripts = scan_dir_for_files(LUA_SCRIPT_PATH, LUA_SCRIPT_EXT);
    std::list<boost::filesystem::path>::iterator script_iter = lua_scripts.begin();
    for (; script_iter != lua_scripts.end(); ++script_iter) {

        boost::filesystem::path stem = script_iter->stem();
        std::string event_name = stem.native();

        if (!check_status(
                context.get(),
                mutton_register_script_path(
                    context.get(),
                    MTN_SCRIPT_LUA,
                    (void*) event_name.c_str(),
                    event_name.size(),
                    (void*) script_iter->c_str(),
                    strlen(script_iter->c_str()),
                    &status),
                status,
                "error registering script: " + event_name))
        {
            return -1;
        }

    }

    if (!check_status(
            context.get(),
            mutton_process_event_bucketed(
                context.get(),
                INDEX_PARTITION,
                (void*) BUCKET_NAME,
                strlen(BUCKET_NAME),
                (void*) BASIC_EVENT_NAME,
                strlen(BASIC_EVENT_NAME),
                (void*) BASIC_EVENT_JSON,
                strlen(BASIC_EVENT_JSON),
                &status),
            status,
            "error processing event"))
    {
        return -1;
    }

    return 0;
}
