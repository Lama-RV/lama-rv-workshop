#pragma once

#include "compiler.h"

namespace lama {
class Instruction {
public:
    virtual void emit_code(rv::Compiler *) {}
    virtual ~Instruction() = default;
};

}