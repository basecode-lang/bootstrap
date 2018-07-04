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

#include <vm/assembler.h>
#include "type.h"
#include "element.h"
#include "identifier.h"

namespace basecode::compiler {

    class block : public element {
    public:
        block(
            block* parent,
            element_type_t type = element_type_t::block);

        bool emit(
            common::result& r,
            vm::assembler& assembler);

        bool define_data(
            common::result& r,
            vm::assembler& assembler);

        type_map_t& types();

        block_list_t& blocks();

        comment_list_t& comments();

        statement_list_t& statements();

        identifier_map_t& identifiers();

    private:
        void add_symbols(
            common::result& r,
            vm::segment_t* segment,
            const identifier_list_t& list);

    private:
        type_map_t _types {};
        block_list_t _blocks {};
        comment_list_t _comments {};
        statement_list_t _statements {};
        identifier_map_t _identifiers {};
    };

};