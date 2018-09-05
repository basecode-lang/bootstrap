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

#include <common/defer.h>
#include <compiler/session.h>
#include "type.h"
#include "element.h"
#include "program.h"
#include "identifier.h"
#include "symbol_element.h"
#include "binary_operator.h"
#include "integer_literal.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    binary_operator::binary_operator(
            compiler::module* module,
            block* parent_scope,
            operator_type_t type,
            element* lhs,
            element* rhs) : operator_base(module, parent_scope, element_type_t::binary_operator, type),
                            _lhs(lhs),
                            _rhs(rhs) {
    }

    bool binary_operator::on_infer_type(
            const compiler::session& session,
            infer_type_result_t& result) {
        switch (operator_type()) {
            case operator_type_t::add:
            case operator_type_t::modulo:
            case operator_type_t::divide:
            case operator_type_t::subtract:
            case operator_type_t::multiply:
            case operator_type_t::exponent:
            case operator_type_t::binary_or:
            case operator_type_t::binary_and:
            case operator_type_t::binary_xor:
            case operator_type_t::shift_left:
            case operator_type_t::shift_right:
            case operator_type_t::rotate_left:
            case operator_type_t::rotate_right: {
                return _lhs->infer_type(session, result);
            }
            case operator_type_t::dereference: {
                return _rhs->infer_type(session, result);
            }
            case operator_type_t::equals:
            case operator_type_t::less_than:
            case operator_type_t::not_equals:
            case operator_type_t::logical_or:
            case operator_type_t::logical_and:
            case operator_type_t::greater_than:
            case operator_type_t::less_than_or_equal:
            case operator_type_t::greater_than_or_equal: {
                result.inferred_type = session.scope_manager().find_type({.name = "bool"});
                return true;
            }
            default:
                return false;
        }
    }

    bool binary_operator::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        switch (operator_type()) {
            case operator_type_t::add:
            case operator_type_t::modulo:
            case operator_type_t::divide:
            case operator_type_t::subtract:
            case operator_type_t::multiply:
            case operator_type_t::exponent:
            case operator_type_t::binary_or:
            case operator_type_t::shift_left:
            case operator_type_t::binary_and:
            case operator_type_t::binary_xor:
            case operator_type_t::shift_right:
            case operator_type_t::rotate_left:
            case operator_type_t::rotate_right: {
                emit_arithmetic_operator(session);
                break;
            }
            case operator_type_t::equals:
            case operator_type_t::less_than:
            case operator_type_t::not_equals:
            case operator_type_t::logical_or:
            case operator_type_t::logical_and:
            case operator_type_t::greater_than:
            case operator_type_t::less_than_or_equal:
            case operator_type_t::greater_than_or_equal: {
                emit_relational_operator(session);
                break;
            }
            case operator_type_t::dereference: {
                auto block = assembler.current_block();
                block->comment("XXX: implement . dereference", 4);
                block->nop();
                break;
            }
            case operator_type_t::assignment: {
                auto var = session.variable_for_element(_lhs);
                if (var == nullptr) {
                    session.error(
                        _lhs,
                        "P051",
                        fmt::format("missing assembler variable for {}.", _lhs->label_name()),
                        _lhs->location());
                    return false;
                }

                var->make_live(session);
                defer({
                    var->make_dormant(session);
                });

                _lhs->emit(session);
                var->init(session);

                vm::register_t rhs_reg;
                rhs_reg.size = var->value_reg.reg.size;
                rhs_reg.type = var->value_reg.reg.type;

                if (!assembler.allocate_reg(rhs_reg)) {
                    session.error(
                        _rhs,
                        "P052",
                        "assembler registers exhausted.",
                        _rhs->location());
                    return false;
                }
                assembler.push_target_register(rhs_reg);
                _rhs->emit(session);
                var->write(session);
                assembler.pop_target_register();
                assembler.free_reg(rhs_reg);
                break;
            }
            default:
                break;
        }
        return true;
    }

    bool binary_operator::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        switch (operator_type()) {
            case operator_type_t::add: {
                break;
            }
            case operator_type_t::divide: {
                break;
            }
            case operator_type_t::modulo: {
                break;
            }
            case operator_type_t::equals: {
                break;
            }
            case operator_type_t::subtract: {
                break;
            }
            case operator_type_t::exponent: {
                break;
            }
            case operator_type_t::multiply: {
                break;
            }
            case operator_type_t::binary_or: {
                break;
            }
            case operator_type_t::less_than: {
                break;
            }
            case operator_type_t::not_equals: {
                break;
            }
            case operator_type_t::logical_or: {
                break;
            }
            case operator_type_t::binary_and: {
                break;
            }
            case operator_type_t::binary_xor: {
                break;
            }
            case operator_type_t::shift_left: {
                break;
            }
            case operator_type_t::logical_and: {
                break;
            }
            case operator_type_t::shift_right: {
                break;
            }
            case operator_type_t::rotate_left: {
                break;
            }
            case operator_type_t::dereference: {
                break;
            }
            case operator_type_t::rotate_right: {
                break;
            }
            case operator_type_t::greater_than: {
                break;
            }
            case operator_type_t::less_than_or_equal: {
                break;
            }
            case operator_type_t::greater_than_or_equal: {
                break;
            }
            default:
                break;
        }

        return true;
    }

    element* binary_operator::lhs() {
        return _lhs;
    }

    element* binary_operator::rhs() {
        return _rhs;
    }

    bool binary_operator::on_is_constant() const {
        return (_lhs != nullptr && _lhs->is_constant())
            && (_rhs != nullptr && _rhs->is_constant());
    }

    void binary_operator::lhs(compiler::element* element) {
        _lhs = element;
    }

    void binary_operator::rhs(compiler::element* element) {
        _rhs = element;
    }

    void binary_operator::on_owned_elements(element_list_t& list) {
        if (_lhs != nullptr)
            list.emplace_back(_lhs);
        if( _rhs != nullptr)
            list.emplace_back(_rhs);
    }

    void binary_operator::emit_relational_operator(compiler::session& session) {
        auto& assembler = session.assembler();

        auto lhs_reg = register_for(session, _lhs);
        auto rhs_reg = register_for(session, _rhs);

        if (!lhs_reg.valid || !rhs_reg.valid)
            return;

        assembler.push_target_register(lhs_reg.reg);
        _lhs->emit(session);
        assembler.pop_target_register();

        assembler.push_target_register(rhs_reg.reg);
        _rhs->emit(session);
        session.assembler().pop_target_register();

        switch (operator_type()) {
            case operator_type_t::equals: {
                break;
            }
            case operator_type_t::less_than: {
                break;
            }
            case operator_type_t::not_equals: {
                break;
            }
            case operator_type_t::logical_or: {
                break;
            }
            case operator_type_t::logical_and: {
                break;
            }
            case operator_type_t::greater_than: {
                break;
            }
            case operator_type_t::less_than_or_equal: {
                break;
            }
            case operator_type_t::greater_than_or_equal: {
                break;
            }
            default: {
                break;
            }
        }
    }

    void binary_operator::emit_arithmetic_operator(compiler::session& session) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();
        auto result_reg = assembler.current_target_register();

        auto lhs_reg = register_for(session, _lhs);
        auto rhs_reg = register_for(session, _rhs);

        if (!lhs_reg.valid || !rhs_reg.valid)
            return;

        vm::register_t target_reg {
            .type = vm::register_type_t::none
        };
        defer({
            if (target_reg.type != vm::register_type_t::none) {
                assembler.free_reg(target_reg);
            }
        });

        if (result_reg == nullptr) {
            result_reg = &target_reg;
            result_reg->size = lhs_reg.size();
            result_reg->type = lhs_reg.reg.type;
            if (!assembler.allocate_reg(*result_reg)) {

            }
        }

        assembler.push_target_register(lhs_reg.reg);
        _lhs->emit(session);
        assembler.pop_target_register();

        assembler.push_target_register(rhs_reg.reg);
        _rhs->emit(session);
        assembler.pop_target_register();

        switch (operator_type()) {
            case operator_type_t::add: {
                block->add_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::divide: {
                block->div_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::modulo: {
                block->mod_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::multiply: {
                block->mul_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::exponent: {
                block->pow_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::subtract: {
                block->sub_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::binary_or: {
                block->or_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::shift_left: {
                block->shl_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::binary_and: {
                block->and_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::binary_xor: {
                block->xor_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::rotate_left: {
                block->rol_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::shift_right: {
                block->shr_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::rotate_right: {
                block->ror_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            default:
                break;
        }
    }

};