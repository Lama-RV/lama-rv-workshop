#pragma once
#include <string>
#include <vector>

#include "symb_stack.h"
namespace lama::rv {

    class CodeBuffer {
        private:
        std::vector<std::string> _code;
        public:
        CodeBuffer() {}

        std::string dump_asm() {
            std::string s;
            s.reserve(1 << 14);
            for (const std::string& str : _code) {
                s += str + "\n";
            }
            return s;
        }

        using SymbolicLocation = SymbolicStack::Loc;


    };
    
}