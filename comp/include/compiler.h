#pragma once

#include <format>
#include <optional>
#include <string>
#include <vector>
#include "code_buffer.h"
#include "symb_stack.h"

namespace lama::rv {

class Compiler {
public:
    std::optional<FrameInfo> current_frame{};
    SymbolicStack st{};
    CodeBuffer cb{};
    std::vector<char const*> strs{};
    size_t globals_count{};

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
