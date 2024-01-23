#############################################################################
# Copyright (c) 2024, Isabel Paredes
#
# Distributed under the terms of the BSD 3-Clause License.
#
# The full license is in the file LICENSE, distributed with this software.
#############################################################################

cmake_minimum_required(VERSION 3.5)
project(xeus-ruby)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

set(XEUS_RUBY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Versionning
# ===========

file(STRINGS "${XEUS_RUBY_INCLUDE_DIR}/xeus-ruby/xeus_ruby_config.hpp" version_defines
     REGEX "#define XEUS_RUBY_VERSION_(MAJOR|MINOR|PATCH)")
foreach (ver ${version_defines})
    if (ver MATCHES "#define XEUS_RUBY_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XEUS_RUBY_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif ()
endforeach ()
set(${PROJECT_NAME}_VERSION
    ${XEUS_RUBY_VERSION_MAJOR}.${XEUS_RUBY_VERSION_MINOR}.${XEUS_RUBY_VERSION_PATCH})
message(STATUS "Building xeus-ruby v${${PROJECT_NAME}_VERSION}")

# Configuration
# =============

include(GNUInstallDirs)

if (NOT DEFINED XEUS_RUBY_KERNELSPEC_PATH)
    set(XEUS_RUBY_KERNELSPEC_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/")
endif ()

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xruby/kernel.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xruby/kernel.json"
)

# Build options
# =============

option(XEUS_RUBY_BUILD_STATIC "Build xeus-ruby static library" ON)
option(XEUS_RUBY_BUILD_SHARED "Split xruby build into executable and library" ON)
option(XEUS_RUBY_BUILD_EXECUTABLE "Build the xruby executable" ON)
option(XEUS_RUBY_USE_SHARED_XEUS "Link xruby  with the xeus shared library (instead of the static library)" ON)
option(XEUS_RUBY_USE_SHARED_XEUS_RUBY "Link xruby  with the xeus shared library (instead of the static library)" ON)

if(EMSCRIPTEN)
    add_compile_definitions(XEUS_RUBY_EMSCRIPTEN_WASM_BUILD)
    message("Build with emscripten")
    set(XEUS_RUBY_BUILD_STATIC ON)
    set(XEUS_RUBY_BUILD_SHARED OFF)
    set(XEUS_RUBY_BUILD_EXECUTABLE OFF)
    set(XEUS_RUBY_USE_SHARED_XEUS OFF)
    set(XEUS_RUBY_USE_SHARED_XEUS_RUBY OFF)
    set(XEUS_RUBY_BUILD_TESTS OFF)

endif()

# Dependencies
# ============

set(xtl_REQUIRED_VERSION 0.7.0)
set(xeus_REQUIRED_VERSION 3.0.5)
set(Ruby_REQUIRED_VERSION 3.0)
# set(rice_REQUIRED_VERSION 4.0)

if (NOT TARGET xtl)
    find_package(xtl ${xtl_REQUIRED_VERSION} REQUIRED)
endif ()
if (NOT TARGET xeus AND NOT TARGET xeus-static)
    find_package(xeus ${xeus_REQUIRED_VERSION} REQUIRED)
endif ()

find_package(Ruby ${Ruby_REQUIRED_VERSION} REQUIRED)
# find_package(Rice ${rice_REQUIRED_VERSION} REQUIRED)

# Flags
# =====

include(CheckCXXCompilerFlag)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4141")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4018 /wd4267 /wd4715 /wd4146 /wd4129")
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    if(NOT EMSCRIPTEN)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-parameter -Wextra -Wreorder")
    endif()

    CHECK_CXX_COMPILER_FLAG("-std=c++17" HAS_CPP_17_FLAG)
    if (HAS_CPP_17_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    else ()
        message(FATAL_ERROR "Unsupported compiler -- xeus requires C++17 support!")
    endif ()
endif ()

if(EMSCRIPTEN)
    # ENV (https://github.com/emscripten-core/emscripten/commit/6d9681ad04f60b41ef6345ab06c29bbc9eeb84e0)
    set(EMSCRIPTEN_FEATURES "${EMSCRIPTEN_FEATURES} -s \"EXTRA_EXPORTED_RUNTIME_METHODS=[ENV']\"")
endif()

# Source files
# ============

set(XEUS_RUBY_HEADERS
    include/xeus-ruby/xeus_ruby_config.hpp
    include/xeus-ruby/xinterpreter.hpp
)

set(XEUS_RUBY_SRC
    src/xinterpreter.cpp
)

set(XEUS_RUBY_MAIN_SRC
    src/main.cpp
)

# Targets and link - Macros
# =========================

include(CheckCXXCompilerFlag)

string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib; ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

macro(xeus_ruby_set_common_options target_name)
    if (MSVC)
        target_compile_options(${target_name} PUBLIC /wd4251 /wd4141)
        target_compile_options(${target_name} PUBLIC /wd4018 /wd4267 /wd4715 /wd4146 /wd4129)
    endif ()

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
        CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
        CMAKE_CXX_COMPILER_ID MATCHES "Intel")

        target_compile_options(${target_name} PUBLIC -Wunused-parameter -Wextra -Wreorder)

        # C++17 flag
        CHECK_CXX_COMPILER_FLAG("-std=c++17" HAS_CPP_17_FLAG)
        if (HAS_CPP_17_FLAG)
            target_compile_features(${target_name} PRIVATE cxx_std_17)
        else ()
            message(FATAL_ERROR "Unsupported compiler -- xeus-ruby requires C++17 support!")
        endif ()
    endif ()

    if (APPLE)
        set_target_properties(${target_name} PROPERTIES
            MACOSX_RPATH ON
        )
    else ()
        set_target_properties(${target_name} PROPERTIES
            BUILD_WITH_INSTALL_RPATH 1
            SKIP_BUILD_RPATH FALSE
        )
    endif ()

    set_target_properties(${target_name} PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endmacro()

# Common macro kernels (xruby)
macro(xeus_ruby_set_kernel_options target_name)
    target_link_libraries(${target_name} PUBLIC ${Ruby_LIBRARIES})
    if (XEUS_RUBY_USE_SHARED_XEUS_RUBY)
        target_link_libraries(${target_name} PRIVATE xeus-ruby)
        if(CMAKE_DL_LIBS)
            target_link_libraries(${target_name} PRIVATE ${CMAKE_DL_LIBS} util)
        endif()
    else ()
        target_link_libraries(${target_name} PRIVATE xeus-ruby-static)
    endif()

    if(NOT EMSCRIPTEN)
        find_package(Threads)
        target_link_libraries(${target_name} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    endif()
endmacro()

# Common macro for shared and static library
macro(xeus_ruby_create_target target_name linkage output_name)
    string(TOUPPER "${linkage}" linkage_upper)

    if (NOT ${linkage_upper} MATCHES "^(SHARED|STATIC)$")
        message(FATAL_ERROR "Invalid library linkage: ${linkage}")
    endif ()

    add_library(${target_name} ${linkage_upper} ${XEUS_RUBY_SRC} ${XEUS_RUBY_HEADERS})
    xeus_ruby_set_common_options(${target_name})

    set_target_properties(${target_name} PROPERTIES
                          PUBLIC_HEADER "${XEUS_RUBY_HEADERS}"
                          PREFIX ""
                          VERSION ${${PROJECT_NAME}_VERSION}
                          SOVERSION ${XEUS_RUBY_VERSION_MAJOR}
                          OUTPUT_NAME "lib${output_name}")

    target_compile_definitions(${target_name} PUBLIC "XEUS_RUBY_EXPORTS")

    target_compile_features(${target_name} PRIVATE cxx_std_17)

    target_include_directories(${target_name}
                               PUBLIC
                               ${Ruby_INCLUDE_DIRS}
                               $<BUILD_INTERFACE:${XEUS_RUBY_INCLUDE_DIR}>
                               $<INSTALL_INTERFACE:include>)

    if (XEUS_RUBY_USE_SHARED_XEUS)
        set(XEUS_RUBY_XEUS_TARGET xeus)
    else ()
        set(XEUS_RUBY_XEUS_TARGET xeus-static)
    endif ()

    target_link_libraries(${target_name} PUBLIC ${XEUS_RUBY_XEUS_TARGET} xtl)
    if (WIN32 OR CYGWIN)
        #
    elseif (APPLE)
        target_link_libraries(${target_name} PRIVATE "-undefined dynamic_lookup")
    endif ()
    if(NOT EMSCRIPTEN)
        find_package(Threads) # TODO: add Threads as a dependence of xeus-static?
        target_link_libraries(${target_name} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    endif()

endmacro()

# xeus-ruby
# ===========

set(XEUS_RUBY_TARGETS "")

if (XEUS_RUBY_BUILD_SHARED)
    # Build libraries
    xeus_ruby_create_target(xeus-ruby SHARED xeus-ruby)
    list(APPEND XEUS_RUBY_TARGETS xeus-ruby)
endif ()

if (XEUS_RUBY_BUILD_STATIC)
    # On Windows, a static library should use a different output name
    # to avoid the conflict with the import library of a shared one.
    if (CMAKE_HOST_WIN32)
        xeus_ruby_create_target(xeus-ruby-static STATIC xeus-ruby-static)
    else ()
        xeus_ruby_create_target(xeus-ruby-static STATIC xeus-ruby)
    endif ()
    if(EMSCRIPTEN)
        target_compile_options(xeus-ruby-static PRIVATE -fPIC)
    endif()
    list(APPEND XEUS_RUBY_TARGETS xeus-ruby-static)
endif ()

# xruby
# =======
if (XEUS_RUBY_BUILD_EXECUTABLE)
    find_package(xeus-zmq 1.0.2 REQUIRED)
    add_executable(xruby ${XEUS_RUBY_MAIN_SRC})
    target_compile_features(xruby PRIVATE cxx_std_17)
    xeus_ruby_set_common_options(xruby)
    xeus_ruby_set_kernel_options(xruby)
    target_link_libraries(xruby PRIVATE xeus-zmq)
endif()

if(EMSCRIPTEN)
    include(WasmBuildOptions)
    find_package(xeus-lite REQUIRED)
    add_executable(xruby src/main_emscripten_kernel.cpp)
    target_link_libraries(xruby PRIVATE xeus-lite)
    target_compile_options(xruby PRIVATE -fPIC)
    xeus_ruby_set_kernel_options(xruby)
    xeus_wasm_compile_options(xruby)
    xeus_wasm_link_options(xruby "web,worker")
endif()

# Installation
# ============

include(CMakePackageConfigHelpers)

set(XEUS_RUBY_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "install path for xeus-rubyConfig.cmake")

# Install xeus-ruby and xeus-ruby-static
if (XEUS_RUBY_BUILD_SHARED)
    install(TARGETS ${XEUS_RUBY_TARGETS}
            EXPORT ${PROJECT_NAME}-targets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xeus-ruby)

    # Makes the project importable from the build directory
    export(EXPORT ${PROJECT_NAME}-targets
           FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
endif ()

# Install xruby
if (XEUS_RUBY_BUILD_EXECUTABLE)
    install(TARGETS xruby
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if (XEUS_RUBY_BUILD_EXECUTABLE OR EMSCRIPTEN)
    # Configuration and data directories for jupyter and xeus-ruby
    set(XJUPYTER_DATA_DIR "share/jupyter"    CACHE STRING "Jupyter data directory")

    # Install xruby Jupyter kernelspec
    set(KERNELSPEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels)
    install(DIRECTORY ${KERNELSPEC_DIR}
            DESTINATION ${XJUPYTER_DATA_DIR}
            PATTERN "*.in" EXCLUDE)


    # Extra path for installing Jupyter kernelspec
    if (XEXTRA_JUPYTER_DATA_DIR)
        install(DIRECTORY ${KERNELSPEC_DIR}
                DESTINATION ${XEXTRA_JUPYTER_DATA_DIR}
                PATTERN "*.in" EXCLUDE)
    endif ()
endif ()

# Configure 'xeus-rubyConfig.cmake' for a build tree
set(XEUS_RUBY_CONFIG_CODE "####### Expanded from \@XEUS_RUBY_CONFIG_CODE\@ #######\n")
set(XEUS_RUBY_CONFIG_CODE "${XEUS_RUBY_CONFIG_CODE}set(CMAKE_MODULE_PATH \"${CMAKE_CURRENT_SOURCE_DIR}/cmake;\${CMAKE_MODULE_PATH}\")\n")
set(XEUS_RUBY_CONFIG_CODE "${XEUS_RUBY_CONFIG_CODE}##################################################")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${PROJECT_BINARY_DIR})

# Configure 'xeus-rubyConfig.cmake.in for an install tree
set(XEUS_RUBY_CONFIG_CODE "")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${XEUS_RUBY_CMAKECONFIG_INSTALL_DIR})

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              DESTINATION ${XEUS_RUBY_CMAKECONFIG_INSTALL_DIR})

if (XEUS_RUBY_BUILD_SHARED)
    install(EXPORT ${PROJECT_NAME}-targets
            FILE ${PROJECT_NAME}Targets.cmake
            DESTINATION ${XEUS_RUBY_CMAKECONFIG_INSTALL_DIR})
endif ()

if(EMSCRIPTEN)
    install(TARGETS xruby
            ARCHIVE DESTINATION ${XEUS_RUBY_CMAKECONFIG_INSTALL_DIR})

    install(FILES
            "$<TARGET_FILE_DIR:xruby>/xruby.js"
            "$<TARGET_FILE_DIR:xruby>/xruby.wasm"
            DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
