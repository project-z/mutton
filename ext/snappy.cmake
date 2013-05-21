#
# Install snappy from source
#

if (NOT snappy_NAME)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)

include (ExternalProject)

set(ABBREV "snappy")
set(${ABBREV}_NAME         ${ABBREV})
set(${ABBREV}_INCLUDE_DIRS ${EXT_PREFIX}/include)
set(APP_DEPENDENCIES ${APP_DEPENDENCIES} ${ABBREV})

message("Installing ${snappy_NAME} into ext build area: ${EXT_PREFIX} ...")

ExternalProject_Add(snappy
  PREFIX ${EXT_PREFIX}
  URL https://snappy.googlecode.com/files/snappy-1.1.0.tar.gz
  URL_MD5 "c8f3ef29b5281e78f4946b2d739cea4f"
  PATCH_COMMAND ""
  CONFIGURE_COMMAND ./configure --prefix=${EXT_PREFIX} --enable-static --enable-shared
  BUILD_COMMAND make
  INSTALL_COMMAND make install prefix=${EXT_PREFIX}
  BUILD_IN_SOURCE 1
  )

set(${ABBREV}_STATIC_LIBRARIES ${EXT_PREFIX}/lib/libsnappy.a)

set_target_properties(${snappy_NAME} PROPERTIES EXCLUDE_FROM_ALL ON)

endif (NOT snappy_NAME)