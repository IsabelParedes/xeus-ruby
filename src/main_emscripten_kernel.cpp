/***************************************************************************
* Copyright (c) 2024, Isabel Paredes
*
* Distributed under the terms of the BSD 3-Clause License.
*
* The full license is in the file LICENSE, distributed with this software.
****************************************************************************/

#include <iostream>
#include <memory>

#include <emscripten/bind.h>

#include "xeus-ruby/xinterpreter.hpp"
#include "xeus/xembind.hpp"


EMSCRIPTEN_BINDINGS(my_module) {
    xeus::export_core();
    using interpreter_type = xeus_ruby::interpreter;
    xeus::export_kernel<interpreter_type>("xkernel");
}
