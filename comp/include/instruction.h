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
        LOG(FATAL) << "emit_code unimplemented for " << name();
    };

    virtual bool is_terminator() const {
        return false;
    }

    virtual ~Instruction() = default;
};

}  // namespace lama
