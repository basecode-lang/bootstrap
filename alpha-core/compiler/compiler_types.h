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

#include <filesystem>
#include <functional>
#include "elements/element_types.h"

namespace basecode::compiler {

    class session;

    using path_list_t = std::vector<std::filesystem::path>;

    enum class session_compile_phase_t : uint8_t {
        start,
        success,
        failed
    };

    using session_compile_callback = std::function<void (
        session_compile_phase_t,
        const std::filesystem::path&)>;

    struct session_options_t {
        bool verbose = false;
        size_t heap_size = 0;
        size_t stack_size = 0;
        std::filesystem::path ast_graph_file;
        std::filesystem::path dom_graph_file;
        session_compile_callback compile_callback;
    };

}