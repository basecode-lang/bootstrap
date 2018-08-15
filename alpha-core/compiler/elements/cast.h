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

#include "element.h"

namespace basecode::compiler {

    class cast : public element {
    public:
        cast(
            compiler::module* module,
            block* parent_scope,
            compiler::type* type,
            element* expr);

        compiler::type* type();

        element* expression();

    protected:
        bool on_emit(
            common::result& r,
            emit_context_t& context) override;

        void on_owned_elements(element_list_t& list) override;

        compiler::type* on_infer_type(const compiler::program* program) override;

    private:
        element* _expression = nullptr;
        compiler::type* _type = nullptr;
    };

};

