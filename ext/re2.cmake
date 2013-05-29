if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  #
  # Install RE2 from source
  #
  if (NOT RE2_NAME)

    CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)

    include (snappy)
    include (ExternalProject)

    set(ABBREV "RE2")
    set(${ABBREV}_NAME         ${ABBREV})
    set(${ABBREV}_INCLUDE_DIRS ${EXT_PREFIX}/src/RE2/)
    set(APP_DEPENDENCIES ${APP_DEPENDENCIES} ${ABBREV})

    message("Installing ${RE2_NAME} into ext build area: ${EXT_PREFIX} ...")

    ExternalProject_Add(RE2
      PREFIX ${EXT_PREFIX}
      URL https://re2.googlecode.com/files/re2-20130115.tgz
      URL_MD5 "ef66646926e6cb8f11f277b286eac579"
      PATCH_COMMAND ""
      CONFIGURE_COMMAND ""
      BUILD_COMMAND make
      INSTALL_COMMAND ""
      BUILD_IN_SOURCE 1
      )

    set(CXXFLAGS CMAKE_CXX_FLAGS)

    set(${ABBREV}_LIBRARIES ${EXT_PREFIX}/src/RE2/obj/so/libre2.so)
    set(${ABBREV}_STATIC_LIBRARIES ${EXT_PREFIX}/src/RE2/obj/libre2.a)

    set_target_properties(${RE2_NAME} PROPERTIES EXCLUDE_FROM_ALL ON)

  endif (NOT RE2_NAME)
else()
  find_path(RE2_INCLUDE_DIRS NAMES leveldb/db.h HINTS /usr/include /usr/local/include)
  find_library(RE2_LIBRARIES NAMES libleveldb.a libleveldb.lib libleveldb.dylib HINTS /usr/lib /usr/local/lib)
endif()

if(RE2_INCLUDE_DIRS AND RE2_LIBRARIES)
  set(RE2_FOUND TRUE)
endif(RE2_INCLUDE_DIRS AND RE2_LIBRARIES)

if(RE2_FOUND)
  message(STATUS "Found RE2: ${RE2_LIBRARIES}")
else(RE2_FOUND)
  message(FATAL_ERROR "Could not find RE2 library.")
endif(RE2_FOUND)

set(INCLUDES ${INCLUDES} ${RE2_INCLUDE_DIRS} )
set(LIBS ${LIBS} ${RE2_STATIC_LIBRARIES} )