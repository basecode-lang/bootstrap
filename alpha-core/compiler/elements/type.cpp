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
#include <vm/instruction_block.h>
#include "type.h"
#include "field.h"
#include "identifier.h"
#include "symbol_element.h"

namespace basecode::compiler {

    type::type(
        compiler::module* module,
        block* parent_scope,
        element_type_t type,
        compiler::symbol_element* symbol) : element(module, parent_scope, type),
                                            _symbol(symbol) {
    }

    bool type::packed() const {
        return _packed;
    }

    bool type::emit_finalizer(
            compiler::session& session,
            compiler::identifier* var) {
        return on_emit_finalizer(session, var);
    }

    bool type::emit_initializer(
            compiler::session& session,
            compiler::identifier* var) {
        return on_emit_initializer(session, var);
    }

    bool type::on_emit_finalizer(
            compiler::session& session,
            compiler::identifier* var) {
        return true;
    }

    bool type::on_emit_initializer(
            compiler::session& session,
            compiler::identifier* var) {
        return true;
    }

    void type::packed(bool value) {
        _packed = value;
    }

    size_t type::alignment() const {
        return _alignment;
    }

    void type::alignment(size_t value) {
        _alignment = value;
    }

    size_t type::size_in_bytes() const {
        return _size_in_bytes;
    }

    bool type::is_composite_type() const {
        return false;
    }

    void type::size_in_bytes(size_t value) {
        _size_in_bytes = value;
    }

    bool type::type_check(compiler::type* other) {
        return on_type_check(other);
    }

    type_access_model_t type::access_model() const {
        return on_access_model();
    }

    type_number_class_t type::number_class() const {
        return on_number_class();
    }

    compiler::symbol_element* type::symbol() const {
        return _symbol;
    }

    bool type::on_type_check(compiler::type* other) {
        return false;
    }

    bool type::initialize(compiler::session& session) {
        return on_initialize(session);
    }

    type_access_model_t type::on_access_model() const {
        return type_access_model_t::none;
    }

    type_number_class_t type::on_number_class() const {
        return type_number_class_t::none;
    }

    void type::symbol(compiler::symbol_element* value) {
        _symbol = value;
    }

    void type::on_owned_elements(element_list_t& list) {
        if (_symbol != nullptr)
            list.emplace_back(_symbol);
    }

    bool type::on_initialize(compiler::session& session) {
        return true;
    }

    bool type::emit_type_info(compiler::session& session) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto type_name = name();
        auto type_name_len = static_cast<uint32_t>(type_name.length());
        auto label_name = fmt::format("_ti_{}", _symbol->name());

        block->blank_line();
        block->comment(fmt::format("type: {}", type_name), 0);
        block->label(assembler.make_label(label_name));

        block->dwords({type_name_len});
        block->dwords({type_name_len});
        block->qwords({assembler.make_label_ref(fmt::format(
            "_ti_name_lit_{}_data",
            symbol()->name()))});

        if (!on_emit_type_info(session))
            return false;

        session.type_info_label(
            this,
            assembler.make_label_ref(label_name));

        return true;
    }

    std::string type::name(const std::string& alias) const {
        if (!alias.empty())
            return alias;
        return _symbol != nullptr ? _symbol->name() : "unknown";
    }

    bool type::on_emit_type_info(compiler::session& session) {
        return true;
    }

};