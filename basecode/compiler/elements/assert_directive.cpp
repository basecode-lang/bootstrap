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
#include "assert_directive.h"

namespace basecode::compiler {

    assert_directive::assert_directive(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expression) : directive(module, parent_scope),
                                             _expression(expression) {
    }

    compiler::element* assert_directive::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        auto expr = _expression != nullptr ?
            _expression->clone<compiler::element>(session, new_scope) :
            nullptr;
        auto directive = session
            .builder()
            .make_directive(new_scope, type(), location(), {expr});
        return directive;
    }

    bool assert_directive::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        _expression = fold_result.element;
        return true;
    }

    directive_type_t assert_directive::type() const {
        return directive_type_t::assert;
    }

    bool assert_directive::on_evaluate(compiler::session& session) {
        bool value;
        if (_expression == nullptr
        || !_expression->as_bool(value)) {
            session.error(
                module(),
                "X000",
                "compile time assert requires constant boolean expression.",
                location());
            return false;
        }
        if (!value) {
            session.error(
                module(),
                "X000",
                "compile time assertion failed.",
                _expression->location());
            return false;
        }
        return true;
    }

    void assert_directive::on_owned_elements(element_list_t& list) {
        list.emplace_back(_expression);
    }

}