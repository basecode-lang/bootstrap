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

#include <map>
#include <stack>
#include <string>
#include <vector>
#include <boost/any.hpp>
#include "terp.h"
#include "label.h"
#include "stack_frame.h"
#include "assembly_listing.h"
#include "register_allocator.h"

namespace basecode::vm {

    enum class instruction_block_type_t {
        basic,
        procedure
    };

    enum class section_t : uint8_t {
        bss = 1,
        ro_data,
        data,
        text
    };

    inline static std::string section_name(section_t type) {
        switch (type) {
            case section_t::bss:    return "bss";
            case section_t::ro_data:return "ro_data";
            case section_t::data:   return "data";
            case section_t::text:   return "text";
        }
        return "unknown";
    }

    enum data_definition_type_t : uint8_t {
        initialized = 1,
        uninitialized
    };

    struct align_t {
        uint8_t size = 0;
    };

    struct data_definition_t {
        op_sizes size;
        data_definition_type_t type;
        std::vector<uint64_t> values {};
    };

    enum class block_entry_type_t : uint8_t {
        section = 1,
        memo,
        align,
        instruction,
        data_definition,
    };

    struct block_entry_t {
        block_entry_t() : _data({}),
                          _type(block_entry_type_t::memo) {
        }

        block_entry_t(const align_t& align) : _data(boost::any(align)),
                                              _type(block_entry_type_t::align) {
        }

        block_entry_t(const section_t& section) : _data(boost::any(section)),
                                                  _type(block_entry_type_t::section) {
        }

        block_entry_t(const instruction_t& instruction) : _data(boost::any(instruction)),
                                                          _type(block_entry_type_t::instruction) {
        }

        block_entry_t(const data_definition_t& data) : _data(boost::any(data)),
                                                       _type(block_entry_type_t::data_definition) {
        }

        template <typename T>
        T* data() {
            if (_data.empty())
                return nullptr;
            try {
                return boost::any_cast<T>(&_data);
            } catch (const boost::bad_any_cast& e) {
                return nullptr;
            }
        }

        uint64_t address() const {
            return _address;
        }

        void address(uint64_t value) {
            _address = value;
            for (auto label : _labels)
                label->address(value);
        }

        uint16_t blank_lines() const {
            return _blank_lines;
        }

        void label(vm::label* label) {
            _labels.push_back(label);
        }

        block_entry_type_t type() const {
            return _type;
        }

        void blank_lines(uint16_t count) {
            _blank_lines += count;
        }

        void comment(const std::string& value) {
            _comments.push_back(value);
        }

        const std::vector<vm::label*>& labels() const {
            return _labels;
        }

        const std::vector<std::string>& comments() const {
            return _comments;
        }

    private:
        boost::any _data;
        uint64_t _address = 0;
        block_entry_type_t _type;
        uint16_t _blank_lines = 0;
        std::vector<vm::label*> _labels {};
        std::vector<std::string> _comments {};
    };

    class instruction_block {
    public:
        using block_predicate_visitor_callable = std::function<bool (instruction_block*)>;

        instruction_block(
            instruction_block* parent,
            instruction_block_type_t type);

        virtual ~instruction_block();

    // block support
    public:
        void memo();

        void disassemble();

        void clear_blocks();

        void clear_entries();

        instruction_block* parent();

        stack_frame_t* stack_frame();

        block_entry_t* current_entry();

        listing_source_file_t* source_file();

        instruction_block_type_t type() const;

        std::vector<block_entry_t>& entries();

        void add_block(instruction_block* block);

        label* find_label(const std::string& name);

        void remove_block(instruction_block* block);

        std::vector<label_ref_t*> label_references();

        vm::label* make_label(const std::string& name);

        void source_file(listing_source_file_t* value);

        const std::vector<instruction_block*>& blocks() const;

        label_ref_t* find_unresolved_label_up(common::id_t id);

        bool walk_blocks(const block_predicate_visitor_callable& callable);

        template <typename T>
        T* find_in_blocks(const std::function<T* (instruction_block*)>& callable) {
            std::stack<instruction_block*> block_stack {};
            block_stack.push(this);
            while (!block_stack.empty()) {
                auto block = block_stack.top();
                auto found = callable(block);
                if (found != nullptr)
                    return found;
                block_stack.pop();
                for (auto child_block : block->blocks())
                    block_stack.push(child_block);
            }
            return nullptr;
        }

    // data definitions
    public:
        void align(uint8_t size);

        void section(section_t type);

        void reserve_byte(size_t count);

        void reserve_word(size_t count);

        void reserve_dword(size_t count);

        void reserve_qword(size_t count);

        void string(const std::string& value);

        void bytes(const std::vector<uint8_t>& values);

        void words(const std::vector<uint16_t>& values);

        void dwords(const std::vector<uint32_t>& values);

        void qwords(const std::vector<uint64_t>& values);

        // instructions
    public:
        void rts();

        void dup();

        void nop();

        void exit();

        // interrupts and traps
        void swi(uint8_t index);

        void trap(uint8_t index);

        // cmp variations
        void cmp(
            op_sizes size,
            registers_t lhs_reg,
            registers_t rhs_reg);

        // not variations
        void not_op(
            op_sizes size,
            registers_t dest_reg,
            registers_t src_reg);

        // neg variations
        void neg(
            op_sizes size,
            registers_t dest_reg,
            registers_t src_reg);

        // load variations
        void load_to_ireg(
            op_sizes size,
            registers_t dest_reg,
            registers_t address_reg,
            int64_t offset = 0);

        // store variations
        void store_from_ireg(
            op_sizes size,
            registers_t address_reg,
            registers_t src_reg,
            int64_t offset = 0);

        // move variations
        void move_constant_to_ireg(
            op_sizes size,
            registers_t dest_reg,
            uint64_t immediate);

        void move_ireg_to_ireg(
            registers_t dest_reg,
            registers_t src_reg);

        void move_label_to_ireg(
            registers_t dest_reg,
            const std::string& label_name);

        void move_label_to_ireg_with_offset(
            registers_t dest_reg,
            const std::string& label_name,
            int64_t offset);

        // setxx
        void setz(registers_t dest_reg);

        void setnz(registers_t dest_reg);

        // branches
        void bne(const std::string& label_name);

        void beq(const std::string& label_name);

        // inc variations
        void inc(op_sizes size, registers_t reg);

        // dec variations
        void dec(op_sizes size, registers_t reg);

        // mul variations
        void mul_ireg_by_ireg(
            op_sizes size,
            registers_t dest_reg,
            registers_t multiplicand_reg,
            registers_t multiplier_reg);

        // or variations
        void or_ireg_by_ireg(
            op_sizes size,
            registers_t dest_reg,
            registers_t lhs_reg,
            registers_t rhs_reg);

        // xor variations
        void xor_ireg_by_ireg(
            op_sizes size,
            registers_t dest_reg,
            registers_t lhs_reg,
            registers_t rhs_reg);

        // and variations
        void and_ireg_by_ireg(
            op_sizes size,
            registers_t dest_reg,
            registers_t lhs_reg,
            registers_t rhs_reg);

        // shl variations
        void shl_ireg_by_ireg(
            op_sizes size,
            registers_t dest_reg,
            registers_t lhs_reg,
            registers_t rhs_reg);

        // shr variations
        void shr_ireg_by_ireg(
            op_sizes size,
            registers_t dest_reg,
            registers_t lhs_reg,
            registers_t rhs_reg);

        // rol variations
        void rol_ireg_by_ireg(
            op_sizes size,
            registers_t dest_reg,
            registers_t lhs_reg,
            registers_t rhs_reg);

        // ror variations
        void ror_ireg_by_ireg(
            op_sizes size,
            registers_t dest_reg,
            registers_t lhs_reg,
            registers_t rhs_reg);

        // add variations
        void add_ireg_by_ireg(
            op_sizes size,
            registers_t dest_reg,
            registers_t augend_reg,
            registers_t addened_reg);

        // sub variations
        void sub_ireg_by_ireg(
            op_sizes size,
            registers_t dest_reg,
            registers_t minuend_reg,
            registers_t subtrahend_reg);

        void sub_ireg_by_immediate(
            op_sizes size,
            registers_t dest_reg,
            registers_t minuend_reg,
            uint64_t subtrahend_immediate);

        // div variations
        void div_ireg_by_ireg(
            op_sizes size,
            registers_t dest_reg,
            registers_t dividend_reg,
            registers_t divisor_reg);

        // mod variations
        void mod_ireg_by_ireg(
            op_sizes size,
            registers_t dest_reg,
            registers_t dividend_reg,
            registers_t divisor_reg);

        // swap variations
        void swap_ireg_with_ireg(
            op_sizes size,
            registers_t dest_reg,
            registers_t src_reg);

        // test mask for zero and branch
        void test_mask_branch_if_zero(
            op_sizes size,
            registers_t value_reg,
            registers_t mask_reg,
            registers_t address_reg);

        // test mask for non-zero and branch
        void test_mask_branch_if_not_zero(
            op_sizes size,
            registers_t value_reg,
            registers_t mask_reg,
            registers_t address_reg);

        // push variations
        void push_u8(uint8_t value);

        void push_u16(uint16_t value);

        void push_u32(uint32_t value);

        void push_u64(uint64_t value);

        void push(op_sizes size, registers_t reg);

        // pop variations
        void pop(op_sizes size, registers_t reg);

        // calls & jumps
        void jump_indirect(registers_t reg);

        void call(const std::string& proc_name);

        void call_foreign(uint64_t proc_address);

        void jump_direct(const std::string& label_name);

    private:
        void make_shl_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t value_reg,
            registers_t amount_reg);

        void make_rol_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t value_reg,
            registers_t amount_reg);

        void make_shr_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t value_reg,
            registers_t amount_reg);

        void make_ror_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t value_reg,
            registers_t amount_reg);

        void make_and_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t value_reg,
            registers_t mask_reg);

        void make_xor_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t value_reg,
            registers_t mask_reg);

        void make_or_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t value_reg,
            registers_t mask_reg);

        void make_mod_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t dividend_reg,
            registers_t divisor_reg);

        void make_div_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t dividend_reg,
            registers_t divisor_reg);

        void make_mul_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t multiplicand_reg,
            registers_t multiplier_reg);

        void make_cmp_instruction(
            op_sizes size,
            registers_t lhs_reg,
            registers_t rhs_reg);

        void make_not_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t src_reg);

        void make_neg_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t src_reg);

        void make_inc_instruction(
            op_sizes size,
            registers_t reg);

        void make_dec_instruction(
            op_sizes size,
            registers_t reg);

        void make_load_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t address_reg,
            int64_t offset);

        void make_store_instruction(
            op_sizes size,
            registers_t address_reg,
            registers_t src_reg,
            int64_t offset);

        void make_swap_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t src_reg);

        void make_pop_instruction(
            op_sizes size,
            registers_t dest_reg);

        void make_move_instruction(
            op_sizes size,
            registers_t dest_reg,
            uint64_t value);

        void make_move_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t src_reg);

        void make_add_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t augend_reg,
            registers_t addend_reg);

        void make_sub_instruction(
            op_sizes size,
            registers_t dest_reg,
            registers_t minuend_reg,
            registers_t subtrahend_reg);

        void make_sub_instruction_immediate(
            op_sizes size,
            registers_t dest_reg,
            registers_t minuend_reg,
            uint64_t subtrahend_immediate);

        void make_integer_constant_push_instruction(
            op_sizes size,
            uint64_t value);

        void make_block_entry(const align_t& section);

        void make_block_entry(const section_t& section);

        void make_block_entry(const instruction_t& inst);

        void make_block_entry(const data_definition_t& data);

        void make_push_instruction(op_sizes size, registers_t reg);

    private:
        void disassemble(instruction_block* block);

        vm::label* find_label_up(const std::string& label_name);

        label_ref_t* make_unresolved_label_ref(const std::string& label_name);

    private:
        stack_frame_t _stack_frame;
        instruction_block_type_t _type;
        instruction_block* _parent = nullptr;
        std::vector<block_entry_t> _entries {};
        std::vector<instruction_block*> _blocks {};
        vm::listing_source_file_t* _source_file = nullptr;
        std::unordered_map<std::string, vm::label*> _labels {};
        std::unordered_map<common::id_t, label_ref_t> _unresolved_labels {};
        std::unordered_map<std::string, common::id_t> _label_to_unresolved_ids {};
    };

};

