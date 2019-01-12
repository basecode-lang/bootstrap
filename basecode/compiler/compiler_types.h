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

#include <chrono>
#include <cstdint>
#include <functional>
#include <parser/ast.h>
#include <vm/vm_types.h>
#include <unordered_map>
#include <common/rune.h>
#include <common/result.h>
#include <common/id_pool.h>
#include <common/source_file.h>
#include <boost/filesystem.hpp>
#include "elements/element_types.h"

namespace basecode::compiler {

    class session;
    class element_map;
    class ast_evaluator;
    class scope_manager;
    class element_builder;
    class string_intern_map;
    class code_dom_formatter;

    using path_list_t = std::vector<boost::filesystem::path>;
    using source_file_stack_t = std::stack<common::source_file*>;
    using source_file_list_t = std::vector<common::source_file*>;
    using module_map_t = std::unordered_map<std::string, module*>;
    using ast_map_t = std::unordered_map<std::string, syntax::ast_node_t*>;
    using source_file_map_t = std::unordered_map<common::id_t, common::source_file>;
    using address_register_map_t = std::unordered_map<common::id_t, vm::register_t>;
    using source_file_path_map_t = std::unordered_map<std::string, common::source_file*>;

    ///////////////////////////////////////////////////////////////////////////

    enum class type_access_model_t {
        none,
        value,
        pointer
    };

    enum class type_number_class_t {
        none,
        integer,
        floating_point,
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class identifier_usage_t : uint8_t {
        heap = 1,
        stack
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class session_compile_phase_t : uint8_t {
        start,
        success,
        failed
    };

    enum class session_module_type_t : uint8_t {
        program,
        module
    };

    using session_compile_callback = std::function<void (
        session_compile_phase_t,
        session_module_type_t,
        const boost::filesystem::path&)>;

    using session_meta_options_t = std::vector<std::string>;
    using session_module_paths_t = std::vector<boost::filesystem::path>;
    using session_definition_map_t = std::unordered_map<std::string, std::string>;

    struct session_options_t {
        bool verbose = false;
        size_t heap_size = 0;
        bool debugger = false;
        size_t stack_size = 0;
        size_t ffi_heap_size = 4096;
        bool output_ast_graphs = false;
        vm::allocator* allocator = nullptr;
        boost::filesystem::path compiler_path;
        session_meta_options_t meta_options {};
        session_module_paths_t module_paths {};
        boost::filesystem::path dom_graph_file;
        session_definition_map_t definitions {};
        session_compile_callback compile_callback;
    };

    using session_task_callable_t = std::function<bool ()>;

    struct session_task_t {
        std::string name;
        bool include_in_total;
        std::chrono::microseconds elapsed;
    };

    using session_task_list_t = std::vector<session_task_t>;

    ///////////////////////////////////////////////////////////////////////////

    struct emit_result_t {
        std::vector<vm::instruction_operand_t> operands {};
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class stack_frame_entry_type_t : uint8_t {
        local = 1,
        parameter,
        return_slot
    };

    struct stack_frame_base_offsets_t {
        int32_t locals = 8;
        int32_t parameters = 0;
        int32_t return_slot = 0;
    };

    inline static std::string stack_frame_entry_type_name(stack_frame_entry_type_t type) {
        switch (type) {
            case stack_frame_entry_type_t::local:
                return "local";
            case stack_frame_entry_type_t::parameter:
                return "parameter";
            case stack_frame_entry_type_t::return_slot:
                return "return_slot";
            default:
                return "unknown";
        }
    }

}