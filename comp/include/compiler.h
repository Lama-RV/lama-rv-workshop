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
    struct {
        std::unordered_map<size_t, size_t> expected, actual;
    } stack_hieghts;

    void add_expected_stack_height(size_t offset, size_t expected) {
        cb.emit_comment(std::format("Expectings stack height {:d} at {:#x}", expected, offset));
        auto& map = stack_hieghts.expected;
        if (map.contains(offset)) {
            // DCHECK_EQ(map.at(offset), expected) << std::format("Offset = {:#x}", offset);
        } else {
            map[offset] = expected;
        }
    }

    Compiler(size_t globals)
        : globals_count(globals) {}

    std::string premain() {
        return "la gp, globals";
    }

    std::string dump_asm() {
        return std::format(
            R"(.section .rodata
.section custom_data,"aw",@progbits
.fill 128, 8, 1
.data
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
