/***************************************************************************
* Copyright (c) 2024, Isabel Paredes
*
* Distributed under the terms of the BSD 3-Clause License.
*
* The full license is in the file LICENSE, distributed with this software.
****************************************************************************/

#include <string>

#include "xeus/xinterpreter.hpp"

namespace xeus_ruby
{
    void capture_stdout(const std::string ruby_stdout)
    {
        // This function is called when Ruby writes any output to stdout
        auto& interpreter = xeus::get_interpreter();
        interpreter.publish_stream("stdout", ruby_stdout);
    }
}
