#pragma once

#include <format>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>
#include "code_buffer.h"
#include "cpp.h"
#include "register.h"
#include "symb_stack.h"

namespace lama::rv {

#define DEBUG_COMMENTS 0

class Compiler {
public:
    std::string_view filename;
    SymbolicStack st{};
    CodeBuffer cb;
    std::vector<std::string_view> strs{};
    size_t globals_count{};

    // Instruction address (in bytecode) to symbolic stack size mapping
    std::unordered_map<size_t, size_t> done;
    std::unordered_map<size_t, size_t> todo;

    std::vector<size_t> closure_offsets;

    static std::string label_for_ip(size_t ip) {
        return std::format(".lbc_{:#x}", ip);
    }

    void add_jump_target(size_t offset, size_t stack_height) {
        auto as_done = done.find(offset);
        if (as_done == done.end()) {
            cb.emit_comment(std::format("Expectings stack height {:d} at {:#x}", stack_height, offset));
            auto [as_todo, is_new] = todo.emplace(offset, stack_height);
            DCHECK_EQ(as_todo->second, stack_height) << std::format("offset = {:#x}", offset);
        } else {
            DCHECK_EQ(as_done->second, stack_height) << std::format("offset = {:#x}", offset);
        }
    }

    void inst_begin(size_t offset) {
        size_t const stack_height = st.top;
        auto [as_done, inserted] = done.emplace(offset, stack_height);
        DCHECK_EQ(as_done->second, stack_height) << std::format("offset = {:#x}", offset);
        auto const label = label_for_ip(offset);
        if (inserted) {
            cb.emit_label(label);
        } else {
            cb.emit_comment(label);
        }
    }

    bool should_emit(size_t offset) const {
        auto emitted = done.find(offset);
        if (emitted == done.end()) {
            return true;
        }
        DCHECK_EQ(emitted->second, st.top) << std::format("offset = {:#x}", offset);
        return false;
    }

    void debug_stack_height() {
#if DEBUG_COMMENTS
        cb.emit_comment(std::format("stack height = {:d}", st.top));
#endif
    }

    void compile_call(
        std::variant<std::string, size_t, rv::Register> callee,
        size_t argc,
        std::optional<int64_t> opt_arg = std::nullopt
    ) {
        size_t add_arg = opt_arg.has_value();
        argc += add_arg;
        size_t alignment = (st.current_frame->locals_count + st.spilled_count() + (argc > 8 ? argc - 8 : 0)) & 1;
        // Skip spilled registers
        cb.emit_addi(rv::Register::sp(), rv::Register::sp(), -st.spilled_count() * rv::WORD_SIZE);
        // Save ra
        cb.emit_sd(rv::Register::ra(), rv::Register::sp(), -rv::WORD_SIZE);
        cb.emit_addi(rv::Register::sp(), rv::Register::sp(), -rv::WORD_SIZE);
        // Save temp registers
        rv::Register::temp_apply([this](rv::Register const& r, int) {
            cb.emit_sd(r, rv::Register::sp(), -rv::WORD_SIZE);
            cb.emit_addi(rv::Register::sp(), rv::Register::sp(), -rv::WORD_SIZE);
        });
        // Save arguments
        rv::Register::arg_apply([this](rv::Register const& r, int) {
            cb.emit_sd(r, rv::Register::sp(), -rv::WORD_SIZE);
            cb.emit_addi(rv::Register::sp(), rv::Register::sp(), -rv::WORD_SIZE);
        });
        // Align sp to 16 bytes
        if (alignment) {
            cb.emit_addi(rv::Register::sp(), rv::Register::sp(), -rv::WORD_SIZE);
        }
        // Store extra arguments on stack
        for (auto _ : std::views::iota(0ul, argc) | std::views::drop(8)) {
            cb.apply(st.pop(), [&](Register const& r) {
                cb.emit_sd(r, rv::Register::sp(), -rv::WORD_SIZE);
                cb.emit_addi(rv::Register::sp(), rv::Register::sp(), -rv::WORD_SIZE);
            });
        }
        if (add_arg)
            cb.emit_li(rv::Register::arg(0), *opt_arg);
        for (auto i : std::views::iota(add_arg, argc) | std::views::take(8 - add_arg) | std::views::reverse) {
            cb.symb_emit_mv(rv::Register::arg(i), st.pop());
        }
        if (callee.index() == 2) {
            cb.symb_emit_mv(rv::Register::closurep(), st.pop());
        }
        // Call function
        std::visit(
            overloads{
                [this](std::string name) { cb.emit_call(name); },
                [this](size_t offset) {
                    add_jump_target(offset, 0);
                    cb.emit_call(label_for_ip(offset));
                },
                [this](rv::Register reg) { cb.emit_jalr(reg); }
            },
            callee
        );
        // Drop extra arguments from stack
        if (argc > 8) {
            cb.emit_addi(rv::Register::sp(), rv::Register::sp(), (argc - 8) * rv::WORD_SIZE);
        }
        if (alignment) {
            cb.emit_addi(rv::Register::sp(), rv::Register::sp(), rv::WORD_SIZE);
        }
        cb.symb_emit_mv(st.alloc(), rv::Register::arg(0));
        // Restore arguments
        for (auto i : std::views::iota(0ul, 8ul) | std::views::reverse) {
            cb.emit_ld(rv::Register::arg(i), rv::Register::sp(), 0);
            cb.emit_addi(rv::Register::sp(), rv::Register::sp(), rv::WORD_SIZE);
        };
        // Restore temp registers
        cb.emit_addi(rv::Register::sp(), rv::Register::sp(), 8 * rv::WORD_SIZE);
        rv::Register::temp_apply([this](rv::Register const& r, int i) {
            cb.emit_ld(r, rv::Register::sp(), -(i + 1) * rv::WORD_SIZE);
        });
        // Restore ra
        cb.emit_ld(rv::Register::ra(), rv::Register::sp(), -rv::WORD_SIZE);
        cb.emit_addi(rv::Register::sp(), rv::Register::sp(), st.spilled_count() * rv::WORD_SIZE);
    }

    Compiler(std::string_view file, std::ostream& out, size_t globals, std::vector<std::string_view>&& strings)
        : filename(file)
        , cb(out)
        , strs(strings)
        , globals_count(globals) {}

    void header() {
        std::string string_tab;
        for (int i = 0; i < strs.size(); ++i) {
            string_tab.append(std::format("string_{}: .asciz \"{}\"\n", i, strs[i]));
        }
        cb.emit(std::format(
            R"(.section .rodata
.section custom_data,"aw",@progbits
.fill 128, 8, 1
.data
globals:
.fill {:d}, 8, 0
.align 8
fname: .asciz "{}"
{}
.text
.global main)",
            globals_count, filename, string_tab
        ));
    }

    constexpr std::string main_prologue() {
        return R"(
sd fp, __gc_stack_bottom, t0
la t0, globals)";
    }

    constexpr std::string main_epilogue() {
        return R"(srai a0, a0, 1)";
    }

    void footer() {
        std::string arr;
        for (auto it = closure_offsets.begin(); it != closure_offsets.end(); ++it) {
            if (it != closure_offsets.begin()) {
                arr.append(",");
            }
            arr.append(label_for_ip(*it));
        }
        cb.emit(std::format("closure_offsets: .dword {}", arr));
    }
};
}  // namespace lama::rv
