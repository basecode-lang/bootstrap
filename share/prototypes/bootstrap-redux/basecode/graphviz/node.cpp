// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//       C O M P I L E R  P R O J E C T
//
// Copyright (C) 2019 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include "node.h"

namespace basecode::graphviz {

    node_t::node_t(
            memory::allocator_t* allocator,
            model_t* model,
            std::string_view name) : _name(name),
                                     _attributes(allocator, model, component_type_t::node) {
    }

    std::string_view node_t::name() const {
        return _name;
    }

    attribute_container_t& node_t::attributes() {
        return _attributes;
    }

}