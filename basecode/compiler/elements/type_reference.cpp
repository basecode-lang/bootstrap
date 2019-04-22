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
#include <compiler/element_builder.h>
#include "type.h"
#include "numeric_type.h"
#include "pointer_type.h"
#include "unknown_type.h"
#include "type_reference.h"

namespace basecode::compiler {

    type_reference::type_reference(
            compiler::module* module,
            block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::type* type) : element(module, parent_scope, element_type_t::type_reference),
                                    _symbol(symbol),
                                    _type(type) {
    }

    bool type_reference::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.types.emplace_back(_type, this);
        return true;
    }

    bool type_reference::is_void() const {
        if (_type == nullptr
        ||  _type->element_type() != element_type_t::numeric_type)
            return false;

        auto numeric_type = dynamic_cast<compiler::numeric_type*>(_type);
        return numeric_type != nullptr && numeric_type->size_in_bytes() == 0;
    }

    bool type_reference::resolved() const {
        return _type != nullptr;
    }

    compiler::type* type_reference::type() {
        return _type;
    }

    bool type_reference::is_proc_type() const {
        return _type != nullptr && _type->is_proc_type();
    }

    compiler::element* type_reference::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session.builder().make_type_reference(
            new_scope,
            _symbol,
            _type);
    }

    bool type_reference::is_array_type() const {
        return _type != nullptr
               && _type->element_type() == element_type_t::array_type;
    }

    bool type_reference::on_is_constant() const {
        return true;
    }

    bool type_reference::is_pointer_type() const {
        return _type != nullptr && _type->is_pointer_type();
    }

    bool type_reference::is_unknown_type() const {
        return _type != nullptr
            && _type->is_unknown_type();
    }

    symbol_element* type_reference::symbol() const {
        if (_type == nullptr)
            return nullptr;
        return _type->symbol();
    }

    bool type_reference::is_composite_type() const {
        return _type != nullptr && _type->is_composite_type();
    }

    void type_reference::type(compiler::type* value) {
        _type = value;
    }

    const qualified_symbol_t& type_reference::symbol_override() const {
        return _symbol;
    }

    void type_reference::symbol_override(const qualified_symbol_t& value) {
        _symbol = value;
    }

    bool type_reference::extract_unknown_type(extract_unknown_type_result_t& result) {
        if (is_pointer_type()) {
            result.is_pointer = true;
            result.pointer = dynamic_cast<compiler::pointer_type*>(_type);
            result.unknown = dynamic_cast<compiler::unknown_type*>(result.pointer->base_type_ref()->type());
        } else {
            result.unknown = dynamic_cast<compiler::unknown_type*>(_type);
        }
        return result.unknown != nullptr;
    }

}