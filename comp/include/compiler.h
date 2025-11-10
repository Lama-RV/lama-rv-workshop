#pragma once

#include "symb_stack.h"
#include "code_buffer.h"
#include <optional>
#include <string>
#include <vector>
#include <format>

namespace lama::rv {
    
    class Compiler {
    public:
        std::optional<FrameInfo> current_frame{};
        SymbolicStack st{};
        CodeBuffer cb{};
        std::vector<const char *> strs{};
        size_t globals_count{};

        Compiler(size_t globals) : globals_count(globals) {}

        std::string premain() {
            return "la gp, globals";
        }

        std::string dump_asm() {
            constexpr const char *fmt_string = 
R"(.section .rodata
.section custom_data,"aw",@progbits
.fill 128, 8, 1
.data
globals: 
.fill {}, 8, 0
.text
.global main
{})";
            return std::format(fmt_string, globals_count, cb.dump_asm());
        }
    };
}