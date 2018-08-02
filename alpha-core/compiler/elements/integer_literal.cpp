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

#include <vm/instruction_block.h>
#include "program.h"
#include "numeric_type.h"
#include "integer_literal.h"

namespace basecode::compiler {

    integer_literal::integer_literal(
            block* parent_scope,
            uint64_t value) : element(parent_scope, element_type_t::integer_literal),
                              _value(value) {
    }

    bool integer_literal::on_emit(
            common::result& r,
            emit_context_t& context) {
        auto instruction_block = context.assembler->current_block();
        auto target_reg = context.assembler->current_target_register();
        auto inferred_type = infer_type(context.program);
        instruction_block->move_constant_to_ireg(
            vm::op_size_for_byte_size(inferred_type->size_in_bytes()),
            target_reg->reg.i,
            _value);
        return true;
    }

    uint64_t integer_literal::value() const {
        return _value;
    }

    bool integer_literal::on_is_constant() const {
        return true;
    }

    bool integer_literal::on_as_integer(uint64_t& value) const {
        value = _value;
        return true;
    }

    compiler::type* integer_literal::on_infer_type(const compiler::program* program) {
        return program->find_type({
            .name = numeric_type::narrow_to_value(_value)
        });
    }

};