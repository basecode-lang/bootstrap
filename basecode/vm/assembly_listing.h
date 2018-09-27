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

#include "vm_types.h"

namespace basecode::vm {

    class assembly_listing {
    public:
        assembly_listing();

        void reset();

        void write(FILE* file);

        listing_source_file_t* current_source_file();

        void add_source_file(const boost::filesystem::path& path);

        void select_source_file(const boost::filesystem::path& path);

    private:
        listing_source_file_t* _current_source_file = nullptr;
        std::unordered_map<std::string, listing_source_file_t> _source_files {};
    };

};

