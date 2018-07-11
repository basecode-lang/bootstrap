// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <vector>
#include <string>

namespace basecode::common {

    class result_message {
    public:
        enum types {
            info,
            error,
            data
        };

        result_message(
                const std::string& code,
                const std::string& message,
                const std::string& details = "",
                types type = types::info) : _type(type),
                                            _code(code),
                                            _message(message),
                                            _details(details) {
        }

        inline types type() const {
            return _type;
        }

        inline bool is_error() const {
            return _type == types::error;
        }

        inline const std::string& code() const {
            return _code;
        }

        inline const std::string& details() const {
            return _details;
        }

        inline const std::string& message() const {
            return _message;
        }

    private:
        types _type;
        std::string _code;
        std::string _message {};
        std::string _details {};
    };

    using result_message_list = std::vector <result_message>;

};