/***************************************************************************
* Copyright (c) 2024, Isabel Paredes
*
* Distributed under the terms of the BSD 3-Clause License.
*
* The full license is in the file LICENSE, distributed with this software.
****************************************************************************/

#ifndef XEUS_RUBY_UTILS_HPP
#define XEUS_RUBY_UTILS_HPP

#include <string>

#include "xeus/xinterpreter.hpp"

namespace xeus_ruby
{
    void capture_stdout(const std::string ruby_stdout);
}

#endif // XEUS_RUBY_UTILS_HPP
