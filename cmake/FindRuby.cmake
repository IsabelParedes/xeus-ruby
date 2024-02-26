#############################################################################
# Copyright (c) 2024, Isabel Paredes
#
# Distributed under the terms of the BSD 3-Clause License.
#
# The full license is in the file LICENSE, distributed with this software.
#############################################################################

# set(CMAKE_FIND_DEBUG_MODE TRUE)

message("PREFIX IS ${CMAKE_PREFIX_PATH}")

if (EMSCRIPTEN)
    # set(Ruby_EXECUTABLE $ENV{CMAKE_PREFIX_PATH}/bin/ruby)
    set(Ruby_INCLUDE_DIRS $ENV{CMAKE_PREFIX_PATH}/include/ruby.h)
    set(Ruby_INCLUDE_DIR $ENV{CMAKE_PREFIX_PATH}/include/ruby.h)
    set(Ruby_LIBRARIES $ENV{CMAKE_PREFIX_PATH}/lib/libruby-static.a)
    set(Ruby_LIBRARY $ENV{CMAKE_PREFIX_PATH}/lib/libruby-static.a)
endif()

message("Ruby_EXECUTABLE IS ${Ruby_EXECUTABLE}")

find_path(Ruby_INCLUDE_DIR NAMES ruby.h)

find_library(Ruby_LIBRARY NAMES ruby)

if(NOT Ruby_INCLUDE_DIR)
    message(FATAL_ERROR "Ruby include directory not found")
endif()

if(NOT Ruby_LIBRARY)
    message(FATAL_ERROR "Ruby library not found")
endif()

message(STATUS "Found Ruby include directory: ${Ruby_INCLUDE_DIR}")
message(STATUS "Found Ruby library: ${Ruby_LIBRARY}")

mark_as_advanced(Ruby_INCLUDE_DIR Ruby_INCLUDE_DIRS Ruby_LIBRARY Ruby_LIBRARIES)
