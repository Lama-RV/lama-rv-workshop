#pragma once

#include <glog/logging.h>
#include <ostream>
#include "compiler.h"

namespace lama {

class Instruction {
public:
    virtual void print(std::ostream&) const = 0;
    virtual void emit_code(rv::Compiler*) const = 0;

    virtual bool is_terminator() const {
        return false;
    }

    virtual ~Instruction() = default;
};

inline std::ostream& operator<<(std::ostream& os, Instruction const& inst) {
    inst.print(os);
    return os;
}

}  // namespace lama
