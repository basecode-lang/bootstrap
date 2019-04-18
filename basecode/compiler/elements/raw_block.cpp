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
#include "raw_block.h"

namespace basecode::compiler {

    raw_block::raw_block(
            compiler::module* module,
            block* parent_scope,
            const std::string_view& value) : element(module, parent_scope, element_type_t::raw_block),
                                             _value(value) {
    }

    compiler::element* raw_block::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session.builder().make_raw_block(new_scope, _value);
    }

    std::string_view raw_block::value() const {
        return _value;
    }

}