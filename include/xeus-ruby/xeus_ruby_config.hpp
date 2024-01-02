/***************************************************************************
* Copyright (c) 2024, Isabel Paredes                                  
*                                                                          
* Distributed under the terms of the BSD 3-Clause License.                 
*                                                                          
* The full license is in the file LICENSE, distributed with this software. 
****************************************************************************/

#ifndef XEUS_RUBY_CONFIG_HPP
#define XEUS_RUBY_CONFIG_HPP

// Project version
#define XEUS_RUBY_VERSION_MAJOR 0
#define XEUS_RUBY_VERSION_MINOR 1
#define XEUS_RUBY_VERSION_PATCH 0

// Composing the version string from major, minor and patch
#define XEUS_RUBY_CONCATENATE(A, B) XEUS_RUBY_CONCATENATE_IMPL(A, B)
#define XEUS_RUBY_CONCATENATE_IMPL(A, B) A##B
#define XEUS_RUBY_STRINGIFY(a) XEUS_RUBY_STRINGIFY_IMPL(a)
#define XEUS_RUBY_STRINGIFY_IMPL(a) #a

#define XEUS_RUBY_VERSION XEUS_RUBY_STRINGIFY(XEUS_RUBY_CONCATENATE(XEUS_RUBY_VERSION_MAJOR,   \
                 XEUS_RUBY_CONCATENATE(.,XEUS_RUBY_CONCATENATE(XEUS_RUBY_VERSION_MINOR,   \
                                  XEUS_RUBY_CONCATENATE(.,XEUS_RUBY_VERSION_PATCH)))))

#ifdef _WIN32
    #ifdef XEUS_RUBY_EXPORTS
        #define XEUS_RUBY_API __declspec(dllexport)
    #else
        #define XEUS_RUBY_API __declspec(dllimport)
    #endif
#else
    #define XEUS_RUBY_API
#endif

#endif