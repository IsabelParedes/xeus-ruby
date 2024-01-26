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
#undef snprintf
#undef vsnprintf
// #include <rice/rice.hpp>
// #include <rice/stl.hpp>

#include "nlohmann/json.hpp"

#include "xeus/xinput.hpp"
#include "xeus/xinterpreter.hpp"
#include "xeus/xhelper.hpp"

#include "xeus-ruby/xinterpreter.hpp"

namespace nl = nlohmann;

namespace xeus_ruby
{
    void capture_output(const char* msg)
    {
        // auto msg_string = rb_obj_as_string(ruby_message);
        std::string tmp{ msg };
        nl::json pub_data;
        pub_data["text/plain"] = tmp;
        // std::cout << "Got output: " << StringValueCStr(msg_string) << '\n';
        std::cout << "Getting there..." << tmp << '\n';
        // publish_execution_result(0, std::move(pub_data), nl::json::object());

    }


    interpreter::interpreter()
    {
        xeus::register_interpreter(this);
        // int argc{ 0 };
        // char* argv{ nullptr };
        // char** pArgv{ &argv };

        // ruby_sysinit(&argc, &pArgv);
        // Construct VM
        ruby_init();
        // ruby_init_loadpath();

        // For error messages???
        ruby_script("ruby_script");

        // To be able to load gems with require
        ruby_init_loadpath();

        m_rice_module = Rice::define_module("rice_module");

        // Define the output capture in Ruby
        Rice::Class rb_cOutputCapture =
            Rice::define_class("CaptureClass")
            .define_function("capture_output", &capture_output);

        std::string ruby_code =
            "$capture_fun = CaptureClass.new\n"
            "class OutputCapture\n"
            "   def write(text)\n"
            "       $capture_fun.capture_output(text)\n"
            "   end\n"
            "end\n"
            "\n"
            "$stdout = OutputCapture.new\n"
            "$stdout.write('You better work')\n";

        try
        {
            Rice::detail::protect(rb_eval_string, ruby_code.c_str());
        }
        catch (const Rice::Exception& ex)
        {
            std::cout << "BAAD " << ex.what() << '\n';
        }

        // Testing output capture
        // std::string ruby_code =
        //     "class CaptureOutput < IO"
        //     "   def initialize"
        //     "       super()"
        //     "   end"
        //     "end"
        //     ""
        //     "dout, serr, sout = $defout, $stderr, $stdout"
        //     "buf = CaptureOutput.new"
        //     "begin"
        //     "   $defout = buf"
        //     "   $stderr = buf"
        //     "   $stdout = buf"
        //     "   yield"
        //     "ensure"
        //     "   $defout, $stderr, $stdout = dout, serr, sout"
        //     "end";
    }

    nl::json interpreter::execute_request_impl(
        int execution_counter, // Typically the cell number
        const std::string& code, // Code to execute
        bool /*silent*/,
        bool /*store_history*/,
        nl::json /*user_expressions*/,
        bool /*allow_stdin*/
        )
    {
        // Use this method for publishing the execution result to the client,
        // this method takes the ``execution_counter`` as first argument,
        // the data to publish (mime type data) as second argument and metadata
        // as third argument.
        nl::json pub_data{};
        std::vector<std::string> trace_back{};

        // Redirect output stream
        // std::streambuf* strCout = std::cout.rdbuf();
        // std::ostringstream strCout;
        // std::cout.rdbuf( strCout.rdbuf() );


        // Rice::Module mod = Rice::define_module("XeusRubyModule");
        // mod.module_eval(code.c_str());
        // Rice::Object riceult = m_rice_module.module_eval(code.c_str());
        // std::cout << "RICESULT " << rb_obj_as_string(riceult) << '\n';
        // int state;
        // char redirect_code[] =
        //     "$stdout = StringIO.new"
        //     "yield";
        // rb_eval_string_protect(redirect_code, &state);
        // if (state)
        // {
        //     std::cout << "SOMETHING BAD\n";
        // }
        // TODO: clean this up
        std::string wrapped_code = "require 'stringio'\n";
        wrapped_code.append("$stdout = StringIO.new\n");
        wrapped_code.append(code);
        wrapped_code.append("\n$stdout.string\n");

        try
        {
            Rice::Object r_result = Rice::detail::protect(rb_eval_string, code.c_str());
            // TODO: this only gets the last result, need to redirect output
            auto r_string = rb_obj_as_string(r_result);

            // Rice::Object string_output = Rice::detail::protect(rb_eval_string, "$stdout.string");
            // auto string_output_stringy = rb_obj_as_string(string_output);

            pub_data["text/plain"] = StringValueCStr(r_string);
        }
        catch (const Rice::Exception& ex)
        {
            publish_stream("stderr", ex.what());
            // TODO: get trace_back
            return xeus::create_error_reply(ex.what(), "Error", trace_back);
        }
        // int state;

        // VALUE ruby_result = rb_eval_string_protect(code.c_str(), &state);
        // auto ruby_string = rb_obj_as_string(ruby_result);

        // pub_data["text/plain"] = StringValueCStr(ruby_string);

        // if (state)
        // {
        //     // Handle exceptions
        //     publish_stream("stderr", "Something bad happened " + std::to_string(state));
        //     return  xeus::create_error_reply("error value", "Error name", trace_back);
        // }

        // Restore stream redirect
        // std::cout.rdbuf( oldCoutStreamBuf );

        // switch (TYPE(ruby_result))
        // {
        //     case T_NIL:
        //         pub_data["text/plain"] = "nil";
        //         break;
        //     case T_FIXNUM:
        //         pub_data["text/plain"] = FIX2LONG(ruby_result);
        //         break;
        //     case T_STRING:
        //         pub_data["text/plain"] = StringValueCStr(ruby_result);
        //         break;
        //     default:
        //         pub_data["text/plain"] = "None of the above";
        //         break;
        // }
        // Rice::Object exc = Rice::detail::protect(rb_eval_string, code.c_str());

        // If silent is set to true, do not publish anything!
        // Otherwise:
        // Publish the execution result
        publish_execution_result(execution_counter, std::move(pub_data), nl::json::object());

        // You can also use this method for publishing errors to the client, if the code
        // failed to execute
        // publish_execution_error(error_name, error_value, error_traceback);
        // publish_execution_error("TypeError", "123", {"!@#$", "*(*"});

        // Use publish_stream to publish a stream message or error:
        // publish_stream("stdout", "I am publishing a message");
        // publish_stream("stderr", "Error!");

        // Use Helpers that create replies to the server to be returned
        // ( payload, user_expressions )
        return xeus::create_successful_reply(nl::json::array(), nl::json::object());
    }

    void interpreter::configure_impl()
    {
        // `configure_impl` allows you to perform some operations
        // after the custom_interpreter creation and before executing any request.
        // This is optional, but can be useful;
        // you can for example initialize an engine here or redirect output.
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
