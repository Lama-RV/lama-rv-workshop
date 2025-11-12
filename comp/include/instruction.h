#pragma once

#include <glog/logging.h>
#include <cstddef>
#include <limits>
#include "compiler.h"

namespace lama {
class Instruction {
public:
    virtual char const* name() const = 0;

    enum class IsTerminator {
        No,
        Yes,
    };

    // True = terminator
    // False = not a terminator, keep going.
    virtual IsTerminator emit_code(rv::Compiler*) const {
        LOG(ERROR) << "emit_code unimplemented for " << name();
        return IsTerminator::No;
    };

    virtual ~Instruction() = default;
};

}  // namespace lama
