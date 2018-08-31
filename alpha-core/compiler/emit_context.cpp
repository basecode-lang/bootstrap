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

#include <compiler/session.h>
#include <compiler/elements/type.h>
#include <compiler/elements/identifier.h>
#include <compiler/elements/symbol_element.h>
#include <compiler/elements/string_literal.h>
#include <compiler/elements/identifier_reference.h>
#include "emit_context.h"

namespace basecode::compiler {

    bool variable_register_t::reserve(compiler::session& session) {
        allocated = session.assembler().allocate_reg(reg);
        return allocated;
    }

    void variable_register_t::release(compiler::session& session) {
        if (!allocated)
            return;
        session.assembler().free_reg(reg);
        allocated = false;
    }

    ///////////////////////////////////////////////////////////////////////////

    bool variable_t::init(
            compiler::session& session,
            vm::instruction_block* block) {
        if (!live)
            return false;

        if (address_loaded)
            return true;

        if (usage == identifier_usage_t::heap) {
            if (!address_reg.reserve(session))
                return false;

            block->blank_line();
            block->comment(
                fmt::format(
                    "identifier '{}' address (global)",
                    name),
                session.emit_context().indent);

            auto label_ref = session.assembler().make_label_ref(name);
            if (address_offset != 0) {
                block->move_label_to_reg_with_offset(
                    address_reg.reg,
                    label_ref,
                    address_offset);
            } else {
                block->move_label_to_reg(address_reg.reg, label_ref);
            }
        }

        value_reg.reg.type = vm::register_type_t::integer;
        if (type != nullptr) {
            if (type->access_model() == type_access_model_t::value) {
                value_reg.reg.size = vm::op_size_for_byte_size(type->size_in_bytes());
                if (type->number_class() == type_number_class_t::floating_point) {
                    value_reg.reg.type = vm::register_type_t::floating_point;
                }
            } else {
                value_reg.reg.size = vm::op_sizes::qword;
            }
        }

        address_loaded = true;

        return true;
    }

    bool variable_t::read(
            compiler::session& session,
            vm::instruction_block* block) {
        if (!live)
            return false;

        if (!init(session, block))
            return false;

        std::string type_name = "global";
        if (requires_read) {
            if (!value_reg.reserve(session))
                return false;

            block->blank_line();
            block->comment(
                fmt::format(
                    "load identifier '{}' value ({})",
                    name,
                    type_name),
                session.emit_context().indent);

            if (value_reg.reg.size != vm::op_sizes::qword)
                block->clr(vm::op_sizes::qword, value_reg.reg);

            if (usage == identifier_usage_t::stack) {
                type_name = stack_frame_entry_type_name(frame_entry->type);
                block->load_to_reg(
                    value_reg.reg,
                    vm::register_t::fp(),
                    frame_entry->offset);
            } else {
                block->load_to_reg(value_reg.reg, address_reg.reg);
            }

            requires_read = false;
        }

        return true;
    }

    bool variable_t::write(
            compiler::session& session,
            vm::instruction_block* block) {
        auto target_reg = session.assembler().current_target_register();
        if (target_reg == nullptr)
            return false;

        block->store_from_reg(
            address_reg.reg,
            *target_reg,
            frame_entry != nullptr ? frame_entry->offset : 0);

        written = true;
        requires_read = true;

        return true;
    }

    void variable_t::make_live(compiler::session& session) {
        if (live)
            return;
        live = true;
        address_loaded = false;
        requires_read = true;
    }

    void variable_t::make_dormant(compiler::session& session) {
        if (!live)
            return;
        live = false;
        requires_read = false;
        address_loaded = false;
        value_reg.release(session);
        address_reg.release(session);
    }

    ///////////////////////////////////////////////////////////////////////////

    void emit_context_t::pop() {
        if (data_stack.empty())
            return;
        data_stack.pop();
    }

    void emit_context_t::push_if(
            const std::string& true_label_name,
            const std::string& false_label_name) {
        data_stack.push(boost::any(if_data_t {
            .true_branch_label = true_label_name,
            .false_branch_label = false_label_name,
        }));
    }

    variable_t* emit_context_t::allocate_variable(
            const std::string& name,
            compiler::type* type,
            identifier_usage_t usage,
            vm::stack_frame_entry_t* frame_entry) {
        auto var = variable(name);
        if (var != nullptr)
            return var;

        auto it = variables.insert(std::make_pair(
            name,
            variable_t {
                .name = name,
                .written = false,
                .usage = usage,
                .requires_read = false,
                .address_loaded = false,
                .type = type,
                .frame_entry = frame_entry,
            }));
        return it.second ? &it.first->second : nullptr;
    }

    void emit_context_t::clear_scratch_registers() {
        while (!scratch_registers.empty())
            scratch_registers.pop();
    }

    bool emit_context_t::has_scratch_register() const {
        return !scratch_registers.empty();
    }

    vm::register_t emit_context_t::pop_scratch_register() {
        if (scratch_registers.empty())
            return vm::register_t::empty();

        auto reg = scratch_registers.top();
        scratch_registers.pop();
        return reg;
    }

    void emit_context_t::free_variable(
            compiler::session& session,
            const std::string& name) {
        auto var = variable(name);
        if (var != nullptr) {
            var->make_dormant(session);
            variables.erase(name);
        }
    }

    variable_t* emit_context_t::variable(const std::string& name) {
        const auto it = variables.find(name);
        if (it == variables.end())
            return nullptr;
        return &it->second;
    }

    void emit_context_t::push_scratch_register(const vm::register_t& reg) {
        scratch_registers.push(reg);
    }

    variable_t* emit_context_t::variable_for_element(compiler::element* element) {
        if (element == nullptr)
            return nullptr;
        switch (element->element_type()) {
            case element_type_t::identifier: {
                auto identifier = dynamic_cast<compiler::identifier*>(element);
                return variable(identifier->symbol()->name());
            }
            case element_type_t::string_literal: {
                auto string_literal = dynamic_cast<compiler::string_literal*>(element);
                return variable(string_literal->label_name());
            }
            case element_type_t::identifier_reference: {
                auto identifier = dynamic_cast<compiler::identifier_reference*>(element)->identifier();
                return variable(identifier->symbol()->name());
            }
            default:
                return nullptr;
        }
    }

};