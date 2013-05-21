
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  #
  # Install leveldb from source
  #
  if (NOT LevelDB_NAME)

    CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)

    include (snappy)
    include (ExternalProject)

    set(ABBREV "LevelDB")
    set(${ABBREV}_NAME         ${ABBREV})
    set(${ABBREV}_INCLUDE_DIRS ${EXT_PREFIX}/include)
    set(APP_DEPENDENCIES ${APP_DEPENDENCIES} ${ABBREV})

    message("Installing ${LevelDB_NAME} into ext build area: ${EXT_PREFIX} ...")

    ExternalProject_Add(LevelDB
      PREFIX ${EXT_PREFIX}
      URL https://leveldb.googlecode.com/files/leveldb-1.10.0.tar.gz
      URL_MD5 "7e5d8fd6de0daf545bb523b53a9d47c6"
      PATCH_COMMAND patch -p1 -t -d ${EXT_PREFIX}/src/LevelDB -N < ${EXT_PREFIX}/leveldb.patch
      CONFIGURE_COMMAND ""
      BUILD_COMMAND make
      INSTALL_COMMAND ""
      BUILD_IN_SOURCE 1
      )

    set(${ABBREV}_LIBRARIES ${EXT_PREFIX}/src/LevelDB/libleveldb.dylib)
    set(${ABBREV}_STATIC_LIBRARIES ${EXT_PREFIX}/src/LevelDB/libleveldb.a)

    set_target_properties(${LevelDB_NAME} PROPERTIES EXCLUDE_FROM_ALL ON)

  endif (NOT LevelDB_NAME)
else()
  find_path(LevelDB_INCLUDE_DIRS NAMES leveldb/db.h HINTS /usr/include /usr/local/include)
  find_library(LevelDB_LIBRARIES NAMES libleveldb.a libleveldb.lib libleveldb.dylib HINTS /usr/lib /usr/local/lib)
endif()

if(LevelDB_INCLUDE_DIRS AND LevelDB_LIBRARIES)
  set(LevelDB_FOUND TRUE)
endif(LevelDB_INCLUDE_DIRS AND LevelDB_LIBRARIES)

if(LevelDB_FOUND)
  message(STATUS "Found LevelDB: ${LevelDB_LIBRARIES}")
else(LevelDB_FOUND)
  message(FATAL_ERROR "Could not find leveldb library.")
endif(LevelDB_FOUND)

set(INCLUDES ${INCLUDES} ${LevelDB_INCLUDE_DIRS} )
set(LIBS ${LIBS} ${LevelDB_LIBRARIES} )