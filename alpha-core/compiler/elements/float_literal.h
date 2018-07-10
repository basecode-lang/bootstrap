// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#pragma once

#include "element.h"

namespace basecode::compiler {

    class float_literal : public element {
    public:
        float_literal(
            element* parent,
            double value);

        double value() const;

    protected:
        bool on_emit(
            common::result& r,
            vm::assembler& assembler,
            const emit_context_t& context) override;

        bool on_is_constant() const override;

        bool on_as_float(double& value) const override;

        compiler::type* on_infer_type(const compiler::program* program) override;

    private:
        double _value;
    };

};

