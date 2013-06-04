CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)
set(PROJECT_NAME_STR mutton)
project(${PROJECT_NAME_STR} C CXX)

#-------------------
# The version number
#-------------------
set (libmutton_VERSION_MAJOR 0)
set (libmutton_VERSION_MINOR 5)
set (libmutton_VERSION_STRING ${libmutton_VERSION_MAJOR}.${libmutton_VERSION_MINOR})

#-------------------
# Enable C++11
#-------------------
set(CMAKE_CXX_FLAGS "-Wall -Werror -g3 -O2")

#-------------------
# setup deps
#-------------------
set(EXT_PREFIX ${PROJECT_SOURCE_DIR}/ext)
set(CMAKE_MODULE_PATH ${EXT_PREFIX})

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_STATIC_RUNTIME ON)
else()
  set(Boost_USE_STATIC_LIBS OFF)
  set(Boost_USE_STATIC_RUNTIME OFF)
endif()

# UTF8-CPP
set(INCLUDES ${INCLUDES} ${PROJECT_SOURCE_DIR}/ext/utf8/source)

# Boost
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.47.0 REQUIRED)
set(INCLUDES ${INCLUDES} ${Boost_INCLUDE_DIRS} )

# Threading
find_package(Threads)
set(LIBS ${LIBS} ${CMAKE_THREAD_LIBS_INIT} )

# LevelDB
include(leveldb)

# RE2
include(re2)

# # CityHash
# find_path(CityHash_INCLUDE_DIRS NAMES city.h HINTS /usr/include /usr/local/include)
# find_library(CityHash_LIBRARIES NAMES libcityhash.a libcityhash.lib libcityhash.dylib HINTS /usr/lib /usr/local/lib)

# if(CityHash_INCLUDE_DIRS AND CityHash_LIBRARIES)
#   set(CityHash_FOUND TRUE)
# endif(CityHash_INCLUDE_DIRS AND CityHash_LIBRARIES)

# if(CityHash_FOUND)
#   message(STATUS "Found CityHash: ${CityHash_LIBRARIES}")
# else(CityHash_FOUND)
#   message(FATAL_ERROR "Could not find CityHash library.")
# endif(CityHash_FOUND)

# set(INCLUDES ${INCLUDES} ${CityHash_INCLUDE_DIRS} )
# set(LIBS ${LIBS} ${CityHash_LIBRARIES} )

# Lua 5.1
find_package(Lua51 REQUIRED)
set(INCLUDES ${INCLUDES} ${LUA_INCLUDE_DIR} )
set(LIBS ${LIBS} ${LUA_LIBRARIES} )

add_custom_target (AppDependencies ALL
  DEPENDS ${APP_DEPENDENCIES}
  COMMENT "Installing any missing dependencies.")

#-------------------
# libmutton static, shared and executable
#-------------------
set(PROJECT_LIB_NAME ${PROJECT_NAME_STR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "build/bin/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)
set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

include_directories(${INCLUDES} ${PROJECT_INCLUDE_DIR}/libmutton ${PROJECT_SOURCE_DIR}/src)
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_LIB_NAME} SHARED ${SRC_FILES})
add_library(${PROJECT_LIB_NAME}-static STATIC ${SRC_FILES})

target_link_libraries(${PROJECT_LIB_NAME} ${LIBS})
SET_TARGET_PROPERTIES(${PROJECT_LIB_NAME}-static PROPERTIES OUTPUT_NAME ${PROJECT_LIB_NAME})
SET_TARGET_PROPERTIES(${PROJECT_LIB_NAME} PROPERTIES VERSION ${libmutton_VERSION_MAJOR}.${libmutton_VERSION_MINOR} SOVERSION ${libmutton_VERSION_MAJOR})

#-------------------
# install target
#-------------------
FILE(GLOB INSTALL_HEADERS "${PROJECT_INCLUDE_DIR}/libmutton/*.hpp")
INSTALL(FILES ${INSTALL_HEADERS} DESTINATION include/libmutton)

INSTALL(TARGETS ${PROJECT_LIB_NAME}-static ${PROJECT_LIB_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

#-------------------
# uninstall target
#-------------------
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

#-------------------
# unit tests
#-------------------
enable_testing()
add_subdirectory(test/unit)

#-------------------
# the demo program
#-------------------
add_subdirectory(src/demo EXCLUDE_FROM_ALL)
