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

    class import : public element {
    public:
        import(
            block* parent_scope,
            element* expr,
            element* from_expr);

        element* expression();

        element* from_expression();

    protected:
        void on_owned_elements(element_list_t& list) override;

    private:
        element* _expression = nullptr;
        element* _from_expression = nullptr;
    };

};

