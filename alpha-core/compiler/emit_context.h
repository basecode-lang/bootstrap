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

#include <stack>
#include <string>
#include <vm/terp.h>
#include <boost/any.hpp>
#include <vm/assembler.h>
#include <vm/stack_frame.h>
#include <vm/instruction_block.h>
#include <compiler/compiler_types.h>

namespace basecode::compiler {

    struct if_data_t {
        std::string true_branch_label;
        std::string false_branch_label;
    };

    struct variable_register_t {
        bool reserve(vm::assembler* assembler);

        void release(vm::assembler* assembler);

        bool allocated = false;
        vm::register_t reg;
    };

    struct emit_context_t;

    struct variable_t {
        bool init(
            emit_context_t& context,
            vm::instruction_block* block);

        bool read(
            emit_context_t& context,
            vm::instruction_block* block);

        bool write(
            emit_context_t& context,
            vm::instruction_block* block);

        void make_live(emit_context_t& context);

        void make_dormant(emit_context_t& context);

        std::string name;
        bool live = false;
        bool written = false;
        identifier_usage_t usage;
        int64_t address_offset = 0;
        bool requires_read = false;
        bool address_loaded = false;
        variable_register_t value_reg;
        compiler::type* type = nullptr;
        variable_register_t address_reg {
            .reg = {
                .size = vm::op_sizes::qword,
                .type = vm::register_type_t::integer
            },
        };
        vm::stack_frame_entry_t* frame_entry = nullptr;
    };

    class program;

    struct emit_context_t {
        emit_context_t(
            compiler::session& session,
            vm::terp* terp,
            vm::assembler* assembler,
            compiler::program* program);

        template <typename T>
        T* top() {
            if (data_stack.empty())
                return nullptr;
            try {
                return boost::any_cast<T>(&data_stack.top());
            } catch (const boost::bad_any_cast& e) {
                return nullptr;
            }
        }

        void pop();

        void push_if(
            const std::string& true_label_name,
            const std::string& false_label_name);

        variable_t* allocate_variable(
            common::result& r,
            const std::string& name,
            compiler::type* type,
            identifier_usage_t usage,
            vm::stack_frame_entry_t* frame_entry = nullptr);

        void clear_scratch_registers();

        bool has_scratch_register() const;

        vm::register_t pop_scratch_register();

        void free_variable(const std::string& name);

        variable_t* variable(const std::string& name);

        void push_scratch_register(const vm::register_t& reg);

        variable_t* variable_for_element(compiler::element* element);

        uint8_t indent = 0;
        vm::terp* terp = nullptr;
        compiler::session& session;
        vm::assembler* assembler = nullptr;
        compiler::program* program = nullptr;
        std::stack<boost::any> data_stack {};
        std::stack<vm::register_t> scratch_registers {};
        std::unordered_map<std::string, variable_t> variables {};
    };

};

