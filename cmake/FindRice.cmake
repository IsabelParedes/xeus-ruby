#############################################################################
# Copyright (c) 2024, Isabel Paredes
#
# Distributed under the terms of the BSD 3-Clause License.
#
# The full license is in the file LICENSE, distributed with this software.
#############################################################################

if (EMSCRIPTEN)
    set(Rice_INCLUDE_DIR $ENV{CMAKE_PREFIX_PATH}/include/rice/rice.hpp)
endif()

find_path(Rice_INCLUDE_DIR rice.hpp)
