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

    class transmute  : public element {
    public:
        transmute(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::type_reference* type,
            compiler::element* expr);

        compiler::element* expression();

        compiler::type_reference* type();

        void expression(compiler::element* value);

        const common::source_location& type_location() const;

        void type_location(const common::source_location& loc);

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) override;

        compiler::element* on_clone(
            compiler::session& session,
            compiler::block* new_scope) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::element* _expression = nullptr;
        common::source_location _type_location;
        compiler::type_reference* _type_ref = nullptr;
    };

}

