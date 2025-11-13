#pragma once
#include <glog/logging.h>
#include <array>
#include <cstddef>
#include <string>

namespace lama::rv {
class SymbolicStack {
public:
    enum class LocType { Register, Memory };

    struct Loc {
        LocType type;
        size_t number;
    };

    constexpr static auto regs = std::to_array<size_t>({9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 6, 7, 28, 29});

    int top;
    SymbolicStack()
        : top(0) {}

    Loc alloc() {
        if (top >= regs.size()) {
            return Loc{.type = LocType::Memory, .number = top++ - regs.size()};
        } else {
            return Loc{.type = LocType::Register, .number = regs[top++]};
        }
    }

    void push(Loc) {
        top++;
    }

    size_t spilled_count() {
        return (top > regs.size()) ? top - regs.size() : 0;
    }

    Loc peek() {
        DCHECK_GT(top, 0) << "peek at empty symbolic stack";
        if (top - 1 >= regs.size()) {
            return Loc{.type = LocType::Memory, .number = top - 1 - regs.size()};
        } else {
            return Loc{.type = LocType::Register, .number = regs[top - 1]};
        }
    }

    Loc pop() {
        DCHECK_GT(top, 0) << "pop from empty symbolic stack";
        if (--top >= regs.size()) {
            return Loc{.type = LocType::Memory, .number = top - regs.size()};
        } else {
            return Loc{.type = LocType::Register, .number = regs[top]};
        }
    }
};

struct FrameInfo {
    std::string function_name;
    size_t locals_count;
    size_t args_count;
};
}  // namespace lama::rv
