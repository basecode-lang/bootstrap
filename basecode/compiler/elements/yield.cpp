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
#include "yield.h"

namespace basecode::compiler {

    yield::yield(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expression) : element(module, parent_scope, element_type_t::yield),
                                             _expression(expression) {
    }

    compiler::element* yield::expression() {
        return _expression;
    }

    void yield::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

}