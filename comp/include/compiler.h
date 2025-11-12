#pragma once

#include <format>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "code_buffer.h"
#include "instruction.h"
#include "symb_stack.h"

namespace lama::rv {

class Compiler {
public:
    std::optional<FrameInfo> current_frame{};
    SymbolicStack st{};
    CodeBuffer cb{};
    std::vector<char const*> strs{};
    size_t globals_count{};

    // Instruction address (in bytecode) to sumbolic stack size mapping
    std::unordered_map<size_t, size_t> done;
    std::unordered_map<size_t, size_t> todo;

    static std::string label_for_ip(size_t ip){
        return std::format("lamabc_{:#x}", ip);
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
        cb.emit_comment(std::format("stack height = {:d}", st.top));
    }

    Compiler(size_t globals)
        : globals_count(globals) {}

    std::string premain() {
        return
            R"(sd gp, saved_gp, t0
la gp, globals)";
    }

    std::string postmain() {
        return
            R"(srai a0, a0, 1
ld gp, saved_gp)";
    }

    std::string dump_asm() {
        return std::format(
            R"(.section .rodata
.section custom_data,"aw",@progbits
.fill 128, 8, 1
.data
saved_gp:
.dword 0
globals:
.fill {:d}, 8, 0
.text
.global main
{:s})",
            globals_count, cb.dump_asm()
        );
    }
};
}  // namespace lama::rv
