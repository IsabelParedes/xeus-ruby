/***************************************************************************
* Copyright (c) 2024, Isabel Paredes
*
* Distributed under the terms of the BSD 3-Clause License.
*
* The full license is in the file LICENSE, distributed with this software.
****************************************************************************/

#include <string>
#include <vector>
#include <iostream>
#include <streambuf>
#include <sstream>

#include <ruby.h>
#include <rice/rice.hpp>
#include <rice/stl.hpp>

#include "nlohmann/json.hpp"

#include "xeus/xinput.hpp"
#include "xeus/xinterpreter.hpp"
#include "xeus/xhelper.hpp"

#include "xeus-ruby/xinterpreter.hpp"
#include "xeus-ruby/xutils.hpp"

namespace nl = nlohmann;

namespace xeus_ruby
{

    interpreter::interpreter()
    {
        xeus::register_interpreter(this);
        // Construct VM
        ruby_init();

        // To be able to load gems with require
        ruby_init_loadpath();

    }

    nl::json interpreter::execute_request_impl(
        int execution_counter, // Typically the cell number
        const std::string& code, // Code to execute
        bool silent,
        bool /*store_history*/,
        nl::json /*user_expressions*/,
        bool /*allow_stdin*/
        )
    {
        // This method publishes the execution result to the client
        nl::json pub_data{};
        std::vector<std::string> trace_back{};

        try
        {
            Rice::Object r_result = Rice::detail::protect(rb_eval_string, code.c_str());
            if (!NIL_P(r_result.value()))
            {
                auto r_string = rb_obj_as_string(r_result);
                pub_data["text/plain"] = StringValueCStr(r_string);
            }
        }
        catch (const Rice::Exception& ex)
        {
            // publish_execution_error(error_name, error_value, error_traceback);
            publish_stream("stderr", ex.what());
            // TODO: get trace_back
            return xeus::create_error_reply(ex.what(), "Error", trace_back);
        }

        if (!silent)
            publish_execution_result(execution_counter, std::move(pub_data), nl::json::object());

        // ( payload, user_expressions )
        return xeus::create_successful_reply(nl::json::array(), nl::json::object());
    }

    void interpreter::configure_impl()
    {
        // Runs after the custom_interpreter creation and before the execution
        // of  any request.
        // stdout from Ruby is redirected to be printed on the notebook
        // stderr is NOT redirected because this is handled with Rice exceptions

        // Define the output capture in Ruby
        Rice::Class rb_cOutputCapture =
            Rice::define_class("CaptureClass")
            .define_function("capture_stdout", &capture_stdout);

        std::string ruby_code =
            "$capture_obj = CaptureClass.new\n"
            "class StdoutCapture\n"
            "   def write(text)\n"
            "       $capture_obj.capture_stdout(text)\n"
            "   end\n"
            "end\n"
            "$stdout = StdoutCapture.new\n";

        Rice::detail::protect(rb_eval_string, ruby_code.c_str());
    }

    nl::json interpreter::is_complete_request_impl(const std::string& code)
    {
        // Insert code here to validate the ``code``
        // and use `create_is_complete_reply` with the corresponding status
        // "unknown", "incomplete", "invalid", "complete"
        return xeus::create_is_complete_reply("complete"/*status*/, "   "/*indent*/);
    }

    nl::json interpreter::complete_request_impl(const std::string& code,
                                                     int cursor_pos)
    {
        // Should be replaced with code performing the completion
        // and use the returned `matches` to `create_complete_reply`
        // i.e if the code starts with 'H', it could be the following completion
        if (code[0] == 'H')
        {

            return xeus::create_complete_reply(
                {
                    std::string("Hello"),
                    std::string("Hey"),
                    std::string("Howdy")
                },          /*matches*/
                5,          /*cursor_start*/
                cursor_pos  /*cursor_end*/
            );
        }

        // No completion result
        else
        {

            return xeus::create_complete_reply(
                nl::json::array(),  /*matches*/
                cursor_pos,         /*cursor_start*/
                cursor_pos          /*cursor_end*/
            );
        }
    }

    nl::json interpreter::inspect_request_impl(const std::string& /*code*/,
                                                      int /*cursor_pos*/,
                                                      int /*detail_level*/)
    {

        return xeus::create_inspect_reply(true/*found*/,
            {{std::string("text/plain"), std::string("hello!")}}, /*data*/
            {{std::string("text/plain"), std::string("hello!")}}  /*meta-data*/
        );

    }

    void interpreter::shutdown_request_impl() {
        // ruby_finalize();
        std::cout << "Bye!!" << std::endl;
        VALUE val = ruby_cleanup(0);
        if (val)
        {
            // Handle exceptions here
        }
    }

    nl::json interpreter::kernel_info_request_impl()
    {

        const std::string  protocol_version = "5.3";
        const std::string  implementation = "xruby";
        const std::string  implementation_version = XEUS_RUBY_VERSION;
        const std::string  language_name = "ruby";
        const std::string  language_version = "3.2.2";
        const std::string  language_mimetype = "text/x-ruby";;
        const std::string  language_file_extension = "rb";;
        const std::string  language_pygments_lexer = "";
        const std::string  language_codemirror_mode = "";
        const std::string  language_nbconvert_exporter = "";
        const std::string  banner = "xruby";
        const bool         debugger = false;
        const nl::json     help_links = nl::json::array();

        return xeus::create_info_reply(
            protocol_version,
            implementation,
            implementation_version,
            language_name,
            language_version,
            language_mimetype,
            language_file_extension,
            language_pygments_lexer,
            language_codemirror_mode,
            language_nbconvert_exporter,
            banner,
            debugger,
            help_links
        );
    }

}
