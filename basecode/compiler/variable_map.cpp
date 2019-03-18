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

#include <vm/assembler.h>
#include <vm/basic_block.h>
#include "session.h"
#include "elements.h"
#include "element_map.h"
#include "variable_map.h"
#include "scope_manager.h"

namespace basecode::compiler {

    ///////////////////////////////////////////////////////////////////////////

    // Example stack frame for a procedure:
    //
    // +-------------+
    // | ...         |
    // | locals      | -offsets
    // | fp          | 0
    // | return addr | +offsets
    // | return slot |
    // | param 1     |
    // | param 2     |
    // +-------------+
    //

    ///////////////////////////////////////////////////////////////////////////

    // variable rules
    // -----------------------------------------------------------------------
    //
    // module: these variables are most similar to static variables in C/C++.
    //         however, unlike static variables, it's possible for other
    //         modules to access them through the module reference, e.g.
    //
    //         some_module :: module("some_module");
    //
    //         core::print("some_module = ${some_module::foo}");
    //
    //         module variables may live in any namespace. if you want to
    //         make a module variable "private", you can put it in a namespace
    //         you don't publish as part of the API.  (though, if people know it,
    //         they can still get to the variable).
    //
    //         these variables are reserved in the executable image. bss, ro_data, or data
    //         are the primary sections in play.  note: unlike some other languages,
    //         basecode does not "copy" values into the image; rather, the initialized
    //         values are persisted into the image.  if there is no initializer or
    //         the variable is explicitly uninitialized, only space is reserved in the image.
    //
    // locals: these live on the stack (negative offsets from FP) or
    //          registers if they're primitive root values.
    //
    // parameters: these live on the stack at positive offsets from FP
    //
    // return values: these live on the stack at positive offsets from FP
    //
    // temporary: these are always registers
    //
    //

    ///////////////////////////////////////////////////////////////////////////

    size_t variable_t::size_in_bytes() const {
        auto var = identifier;
        if (var != nullptr)
            return var->type_ref()->type()->size_in_bytes();
        return 0;
    }

    bool variable_t::flag(basecode::compiler::variable_t::flags_t f) const {
        return (state & f) == f;
    }

    void variable_t::flag(basecode::compiler::variable_t::flags_t f, bool value) {
        if (value)
            state |= f;
        else
            state &= ~f;
    }

    ///////////////////////////////////////////////////////////////////////////

    variable_map::variable_map(compiler::session& session) : _session(session) {
    }

    bool variable_map::use(
            vm::basic_block* basic_block,
            vm::assembler_named_ref_t* named_ref,
            bool load_on_use) {
        auto var = find(named_ref->name);
        if (var == nullptr)
            return false;

        var->flag(variable_t::flags_t::used, true);

        auto& assembler = _session.assembler();
        auto var_type = var->identifier->type_ref()->type();

        if (var->flag(variable_t::flags_t::must_init)) {
            uint64_t default_value = var_type->element_type() == element_type_t::rune_type ?
                                     common::rune_invalid :
                                     0;
            auto op_size = vm::op_size_for_byte_size(var_type->size_in_bytes());
            basic_block->comment("init", vm::comment_location_t::after_instruction);

            switch (var->type) {
                case variable_type_t::local:
                case variable_type_t::parameter:
                case variable_type_t::return_value: {
                    auto local_offset_ref = assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::offset,
                        var->label,
                        vm::op_sizes::word);

                    auto init = var->identifier->initializer();
                    if (init != nullptr) {
                        auto expr = init->expression();
                        if (expr != nullptr) {
                            if (!expr->as_integer(default_value))
                                return false;
                        }
                    }

                    basic_block->move(
                        vm::instruction_operand_t(named_ref),
                        vm::instruction_operand_t(default_value, op_size));
                    basic_block->store(
                        vm::instruction_operand_t::fp(),
                        vm::instruction_operand_t(named_ref),
                        vm::instruction_operand_t(local_offset_ref));
                    break;
                }
                case variable_type_t::module: {
                    auto module_var_ref = assembler.make_named_ref(
                        vm::assembler_named_ref_type_t::label,
                        var->label);
                    basic_block->move(
                        vm::instruction_operand_t(named_ref),
                        vm::instruction_operand_t(default_value, op_size));
                    basic_block->store(
                        vm::instruction_operand_t(module_var_ref),
                        vm::instruction_operand_t(named_ref),
                        vm::instruction_operand_t::offset(var->field_offset.value));
                    break;
                }
                case variable_type_t::temporary: {
                    basic_block->move(
                        vm::instruction_operand_t(named_ref),
                        vm::instruction_operand_t(default_value, op_size));
                    break;
                }
            }

            var->flag(variable_t::flags_t::must_init, false);
            var->flag(variable_t::flags_t::initialized, true);
        }

        if (!var->flag(variable_t::flags_t::filled)) {
            var->flag(variable_t::flags_t::filled, true);

            if (load_on_use) {
                basic_block->comment("filled", vm::comment_location_t::after_instruction);

                switch (var->type) {
                    case variable_type_t::local:
                    case variable_type_t::parameter:
                    case variable_type_t::return_value: {
                        auto offset_ref = assembler.make_named_ref(
                            vm::assembler_named_ref_type_t::offset,
                            var->label,
                            vm::op_sizes::word);
                        basic_block->load(
                            vm::instruction_operand_t(named_ref),
                            vm::instruction_operand_t::fp(),
                            vm::instruction_operand_t(offset_ref));
                        break;
                    }
                    case variable_type_t::module: {
                        auto source_ref = assembler.make_named_ref(
                            vm::assembler_named_ref_type_t::label,
                            var->label);
                        basic_block->load(
                            vm::instruction_operand_t(named_ref),
                            vm::instruction_operand_t(source_ref),
                            vm::instruction_operand_t::offset(var->field_offset.value));
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }
        }

        return true;
    }

    bool variable_map::build(
            compiler::block* block,
            compiler::procedure_type* proc_type) {
        reset();

        if (!find_referenced_module_variables(block))
            return false;

        if (!find_local_variables(block))
            return false;

        if (!find_return_variables(proc_type))
            return false;

        if (!find_parameter_variables(proc_type))
            return false;

        return true;
    }

    bool variable_map::assign(
            vm::basic_block* basic_block,
            emit_result_t& lhs,
            emit_result_t& rhs,
            bool requires_copy) {
        auto lhs_named_ref = *(lhs.operands.front().data<vm::assembler_named_ref_t*>());
        auto var = find(lhs_named_ref->name);
        if (var == nullptr)
            return false;

        if (!var->flag(variable_t::flags_t::filled)) {
            // XXX: should probably include an error message
            return false;
        }

        auto& assembler = _session.assembler();
        //auto is_lhs_pointer = lhs.type_result.inferred_type->is_pointer_type();

        basic_block->comment("spilled", vm::comment_location_t::after_instruction);
        basic_block->move(vm::instruction_operand_t(lhs_named_ref), rhs.operands.back());

        switch (var->type) {
            case variable_type_t::local:
            case variable_type_t::parameter:
            case variable_type_t::return_value: {
                auto local_offset_ref = assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::offset,
                    var->label,
                    vm::op_sizes::word);
                basic_block->store(
                    vm::instruction_operand_t::fp(),
                    vm::instruction_operand_t(lhs_named_ref),
                    vm::instruction_operand_t(local_offset_ref));
                break;
            }
            case variable_type_t::module: {
                auto module_var_ref = assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    var->label);
                basic_block->store(
                    vm::instruction_operand_t(module_var_ref),
                    vm::instruction_operand_t(lhs_named_ref),
                    vm::instruction_operand_t::offset(var->field_offset.value));
                break;
            }
            default: {
                break;
            }
        }

        return true;
    }

    void variable_map::reset() {
        _variables.clear();
    }

    bool variable_map::address_of(
            vm::basic_block* basic_block,
            emit_result_t& arg_result,
            vm::instruction_operand_t& temp_operand) {
        auto named_ref = *(arg_result.operands.front().data<vm::assembler_named_ref_t*>());

        auto var = find(named_ref->name);
        if (var == nullptr)
            return false;

        auto& assembler = _session.assembler();
        basic_block->comment("address_of", vm::comment_location_t::after_instruction);

        switch (var->type) {
            case variable_type_t::local:
            case variable_type_t::parameter:
            case variable_type_t::return_value: {
                auto local_offset_ref = assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::offset,
                    var->label,
                    vm::op_sizes::word);
                basic_block->move(
                    temp_operand,
                    vm::instruction_operand_t::fp(),
                    vm::instruction_operand_t(local_offset_ref));
                break;
            }
            case variable_type_t::module: {
                auto module_var_ref = assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    var->label);
                basic_block->move(
                    temp_operand,
                    vm::instruction_operand_t(module_var_ref),
                    vm::instruction_operand_t::offset(var->field_offset.value));
                break;
            }
            default: {
                break;
            }
        }

        return true;
    }

    bool variable_map::initialize() {
        create_sections();
        return group_module_variables_into_sections();
    }

    void variable_map::create_sections() {
        _module_variables.sections.insert(std::make_pair(vm::section_t::bss,     element_list_t()));
        _module_variables.sections.insert(std::make_pair(vm::section_t::data,    element_list_t()));
        _module_variables.sections.insert(std::make_pair(vm::section_t::ro_data, element_list_t()));
    }

    variable_list_t variable_map::temps() {
        variable_list_t list {};

        for (auto& kvp : _variables) {
            if (kvp.second.type == variable_type_t::temporary)
                list.emplace_back(&kvp.second);
        }

        return list;
    }

    variable_list_t variable_map::variables() {
        variable_list_t list {};

        for (auto& kvp : _variables)
            list.emplace_back(&kvp.second);

        return list;
    }

    variable_t* variable_map::find(const std::string& name) {
        auto it = _variables.find(name);
        if (it == std::end(_variables))
            return nullptr;
        return &it->second;
    }

    bool variable_map::group_module_variables_into_sections() {
        auto& scope_manager = _session.scope_manager();

        auto bss_list = _module_variables.variable_section(vm::section_t::bss);
        auto data_list = _module_variables.variable_section(vm::section_t::data);
        auto ro_list = _module_variables.variable_section(vm::section_t::ro_data);

        std::set<common::id_t> processed_identifiers {};

        const element_type_set_t parent_types {element_type_t::field};
        const element_type_set_t ignored_types {
            element_type_t::generic_type,
            element_type_t::namespace_type,
            element_type_t::module_reference,
        };

        auto identifier_refs = _session
            .elements()
            .find_by_type<compiler::identifier_reference>(element_type_t::identifier_reference);
        for (auto ref : identifier_refs) {
            auto var = ref->identifier();
            if (processed_identifiers.count(var->id()) > 0)
                continue;

            processed_identifiers.insert(var->id());

            if (scope_manager.within_local_scope(var->parent_scope()))
                continue;

            auto var_parent = var->parent_element();
            if (var_parent != nullptr
            &&  var_parent->is_parent_type_one_of(parent_types)) {
                continue;
            }

            auto var_type = var->type_ref()->type();
            if (var_type == nullptr) {
                // XXX: this is an error!
                return false;
            }

            if (var_type->is_type_one_of(ignored_types)) {
                continue;
            }

            auto init = var->initializer();
            if (init != nullptr) {
                switch (init->expression()->element_type()) {
                    case element_type_t::directive: {
                        auto directive = dynamic_cast<compiler::directive*>(init->expression());
                        if (directive->name() == "type")
                            continue;
                        break;
                    }
                    case element_type_t::proc_type:
                    case element_type_t::composite_type:
                    case element_type_t::type_reference:
                    case element_type_t::module_reference:
                        continue;
                    default:
                        break;
                }
            }

            auto has_initializers = var->is_initialized();
            auto is_constant = var->is_constant()
                && var_type->element_type() != element_type_t::tuple_type;

            if (is_constant) {
                ro_list->emplace_back(var);
            } else {
                _module_variables.identifiers.insert(var->id());

                if (!has_initializers) {
                    bss_list->emplace_back(var);
                } else {
                    data_list->emplace_back(var);
                }
            }
        }

        return true;
    }

    void variable_map::release_temp(temp_pool_entry_t* entry) {
        if (entry == nullptr)
            return;
        auto it = _temps.find(entry->id);
        if (it == std::end(_temps))
            return;
        entry->available = true;
    }

    identifier_by_section_t& variable_map::module_variables() {
        return _module_variables;
    }

    bool variable_map::find_local_variables(compiler::block* block) {
        auto& scope_manager = _session.scope_manager();

        int64_t offset = 0;
        element_id_set_t processed {};

        return scope_manager.visit_child_blocks(
            _session.result(),
            [&](compiler::block* scope) {
                if (scope->is_parent_type_one_of({element_type_t::proc_type}))
                    return true;

                for (auto ref_id : scope->references().as_list()) {
                    auto e = _session.elements().find(ref_id);
                    auto ref = dynamic_cast<compiler::identifier_reference*>(e);
                    if (ref == nullptr)
                        continue;

                    auto var = ref->identifier();
                    if (var->is_constant()
                    ||  processed.count(var->id()) > 0) {
                        continue;
                    }

                    auto type = var->type_ref()->type();
                    if (type->is_proc_type())
                        continue;

                    auto offset_result = ref->field_offset();
                    if (!var->parent_scope()->has_stack_frame()) {
                        if (offset_result.base_ref == nullptr
                        || !offset_result.base_ref->parent_scope()->has_stack_frame()) {
                            continue;
                        }
                    }

                    processed.insert(var->id());

                    offset += -type->size_in_bytes();

                    variable_t var_info {};
                    var_info.identifier = var;
                    var_info.frame_offset = offset;
                    var_info.label = var->label_name();
                    var_info.field_offset = offset_result;
                    var_info.type = variable_type_t::local;
                    if (var->is_initialized())
                        var_info.state |= variable_t::flags_t::must_init;
                    else
                        var_info.state = variable_t::flags_t::none;
                    var_info.number_class = type->number_class();

                    _variables.insert(std::make_pair(var->label_name(), var_info));
                }

                return true;
            },
            block);
    }

    temp_pool_entry_t* variable_map::retain_temp(number_class_t number_class) {
        auto temp = find_available_temp(number_class);
        if (temp != nullptr) {
            temp->available = false;
            return temp;
        }

        variable_t var_info {};
        var_info.number_class = number_class;
        var_info.type = variable_type_t::temporary;
        var_info.label = fmt::format("temp{}", _temps.size() + 1);

        temp_pool_entry_t entry {};
        entry.available = false;
        entry.id = common::id_pool::instance()->allocate();

        auto vit = _variables.insert(std::make_pair(var_info.label, var_info));
        entry.variable = &vit.first->second;

        auto it = _temps.insert(std::make_pair(entry.id, entry));
        return &it.first->second;
    }

    bool variable_map::find_referenced_module_variables(compiler::block* block) {
        auto& scope_manager = _session.scope_manager();

        const element_type_set_t excluded_types = {
            element_type_t::proc_type,
            element_type_t::module_type,
            element_type_t::namespace_type,
        };

        element_id_set_t processed {};

        return scope_manager.visit_child_blocks(
            _session.result(),
            [&](compiler::block* scope) {
                for (auto ref_id : scope->references().as_list()) {
                    auto e = _session.elements().find(ref_id);
                    auto ref = dynamic_cast<compiler::identifier_reference*>(e);
                    if (ref == nullptr)
                        continue;

                    auto var = ref->identifier();
                    auto type = var->type_ref()->type();
                    if (type->is_type_one_of(excluded_types)
                    ||  processed.count(var->id()) > 0) {
                        continue;
                    }

                    processed.insert(var->id());

                    if (var->usage() == identifier_usage_t::stack
                    ||  var->parent_scope()->has_stack_frame()) {
                        continue;
                    }

                    variable_t var_info {};
                    var_info.identifier = var;
                    var_info.label = var->label_name();
                    var_info.type = variable_type_t::module;
                    var_info.state = variable_t::flags_t::none;
                    var_info.field_offset = ref->field_offset();
                    var_info.number_class = type->number_class();

                    _variables.insert(std::make_pair(var->label_name(), var_info));
                }

                return true;
            },
            block);
    }

    bool variable_map::find_return_variables(compiler::procedure_type* proc_type) {
        if (proc_type == nullptr || proc_type->is_foreign())
            return true;

        auto return_type = proc_type->return_type();
        if (return_type == nullptr)
            return true;

        auto var = return_type->declaration()->identifier();

        variable_t var_info {};
        var_info.identifier = var;
        var_info.frame_offset = 0;
        var_info.label = var->label_name();
        var_info.state = variable_t::flags_t::none;
        var_info.type = variable_type_t::return_value;
        var_info.number_class = var->type_ref()->type()->number_class();

        _variables.insert(std::make_pair(var->label_name(), var_info));

        return true;
    }

    bool variable_map::find_parameter_variables(compiler::procedure_type* proc_type) {
        if (proc_type == nullptr || proc_type->is_foreign())
            return true;

        uint64_t offset = 0;

        auto fields = proc_type->parameters().as_list();
        for (auto fld : fields) {
            auto var = fld->identifier();

            variable_t var_info {};
            var_info.identifier = var;
            var_info.frame_offset = offset;
            var_info.label = var->label_name();
            var_info.state = variable_t::flags_t::none;
            var_info.type = variable_type_t::return_value;
            var_info.number_class = var->type_ref()->type()->number_class();

            _variables.insert(std::make_pair(var->label_name(), var_info));

            offset += 8;
        }

        return true;
    }

    temp_pool_entry_t* variable_map::find_available_temp(number_class_t number_class) {
        for (const auto& kvp : _temps) {
            if (kvp.second.available
            &&  kvp.second.variable->number_class == number_class) {
                return const_cast<temp_pool_entry_t*>(&kvp.second);
            }
        }
        return nullptr;
    }

}