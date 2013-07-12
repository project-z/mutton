if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  #
  # Install LIBCQL from source
  #
  if (NOT LIBCQL_NAME)

    CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)

    include (snappy)
    include (ExternalProject)

    set(ABBREV "LIBCQL")
    set(${ABBREV}_NAME         ${ABBREV})
    set(${ABBREV}_INCLUDE_DIRS ${EXT_PREFIX}/src/LIBCQL/)
    set(APP_DEPENDENCIES ${APP_DEPENDENCIES} ${ABBREV})

    message("Installing ${LIBCQL_NAME} into ext build area: ${EXT_PREFIX} ...")

    ExternalProject_Add(LIBCQL
      PREFIX ${EXT_PREFIX}
      URL https://github.com/mstump/libcql/archive/master.zip
      CMAKE_ARGS "-DCMAKE_BUILD_TYPE=debug"
      BUILD_IN_SOURCE 1
      )

    set(CXXFLAGS CMAKE_CXX_FLAGS)

    set(${ABBREV}_LIBRARIES ${EXT_PREFIX}/src/libcql/lib/libcql.dylib)
    set(${ABBREV}_STATIC_LIBRARIES ${EXT_PREFIX}/src/libcql/lib/libcql.a)

    set_target_properties(${LIBCQL_NAME} PROPERTIES EXCLUDE_FROM_ALL ON)

  endif (NOT LIBCQL_NAME)
else()
  find_path(LIBCQL_INCLUDE_DIRS NAMES libcql/cql.hpp HINTS /usr/include /usr/local/include)
  find_library(LIBCQL_LIBRARIES NAMES libcql.a libcql.lib libcql.dylib HINTS /usr/lib /usr/local/lib)
endif()

if(LIBCQL_INCLUDE_DIRS AND LIBCQL_LIBRARIES)
  set(LIBCQL_FOUND TRUE)
endif(LIBCQL_INCLUDE_DIRS AND LIBCQL_LIBRARIES)

if(LIBCQL_FOUND)
  message(STATUS "Found libcql: ${LIBCQL_LIBRARIES}")
else(LIBCQL_FOUND)
  message(FATAL_ERROR "Could not find libcql library.")
endif(LIBCQL_FOUND)

set(INCLUDES ${INCLUDES} ${LIBCQL_INCLUDE_DIRS} )
set(LIBS ${LIBS} ${LIBCQL_STATIC_LIBRARIES} )