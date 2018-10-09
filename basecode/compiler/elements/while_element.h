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

    class while_element : public element {
    public:
        while_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::binary_operator* predicate,
            compiler::block* body);

        compiler::block* body();

        compiler::binary_operator* predicate();

    protected:
        bool on_emit(compiler::session& session) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::block* _body = nullptr;
        compiler::binary_operator* _predicate = nullptr;
    };

};
