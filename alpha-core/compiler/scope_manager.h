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

#include <cstdint>
#include <functional>
#include <parser/ast.h>
#include "compiler_types.h"

namespace basecode::compiler {

    using block_stack_t = std::stack<compiler::block*>;
    using module_stack_t = std::stack<compiler::module*>;
    using interned_string_literal_list_t = std::unordered_map<std::string, string_literal_list_t>;

    using block_visitor_callable = std::function<bool (compiler::block*)>;
    using scope_visitor_callable = std::function<compiler::element* (compiler::block*)>;
    using element_visitor_callable = std::function<compiler::element* (compiler::element*)>;
    using namespace_visitor_callable = std::function<compiler::element* (compiler::block*)>;

    class scope_manager {
    public:
        explicit scope_manager(compiler::session& session);

        bool visit_blocks(
            common::result& r,
            const block_visitor_callable& callable,
            compiler::block* root_block = nullptr);

        compiler::type* find_type(
            const qualified_symbol_t& symbol,
            compiler::block* scope = nullptr) const;

        bool find_identifier_type(
            compiler::session& session,
            type_find_result_t& result,
            const syntax::ast_node_shared_ptr& type_node,
            compiler::block* parent_scope = nullptr);

        compiler::block* pop_scope();

        element* walk_parent_scopes(
            compiler::block* scope,
            const scope_visitor_callable& callable) const;

        module_stack_t& module_stack();

        element* walk_parent_elements(
            compiler::element* element,
            const element_visitor_callable& callable) const;

        element* walk_qualified_symbol(
            const qualified_symbol_t& symbol,
            compiler::block* scope,
            const namespace_visitor_callable& callable) const;

        compiler::block* push_new_block(
            compiler::session& session,
            element_type_t type = element_type_t::block);

        compiler::type* find_array_type(
            compiler::type* entry_type,
            size_t size,
            compiler::block* scope = nullptr);

        compiler::type* find_pointer_type(
            compiler::type* base_type,
            compiler::block* scope = nullptr);

        block_stack_t& top_level_stack();

        compiler::module* current_module();

        compiler::block* current_top_level();

        compiler::block* current_scope() const;

        void push_scope(compiler::block* block);

        compiler::identifier* find_identifier(
            const qualified_symbol_t& symbol,
            compiler::block* scope = nullptr) const;

        void add_type_to_scope(compiler::type* type);

        identifier_list_t& identifiers_with_unknown_types();

        interned_string_literal_list_t& interned_string_literals();

        identifier_reference_list_t& unresolved_identifier_references();

        compiler::module* find_module(compiler::element* element) const;

        bool within_procedure_scope(compiler::block* parent_scope = nullptr) const;

    private:
        compiler::session& _session;
        block_stack_t _scope_stack {};
        module_stack_t _module_stack {};
        block_stack_t _top_level_stack {};
        identifier_list_t _identifiers_with_unknown_types {};
        interned_string_literal_list_t _interned_string_literals {};
        identifier_reference_list_t _unresolved_identifier_references {};
    };

};
