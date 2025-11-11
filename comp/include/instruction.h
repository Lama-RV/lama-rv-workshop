#pragma once

#include <glog/logging.h>
#include <cstddef>
#include <limits>
#include "compiler.h"

namespace lama {
class Instruction {
public:
    virtual char const* name() const = 0;
    virtual void emit_code(rv::Compiler*) const {
        LOG(ERROR) << "emit_code unimplemented for " << name();
    };
    virtual ~Instruction() = default;
};

}  // namespace lama
