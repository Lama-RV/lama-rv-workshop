#pragma once
#include <glog/logging.h>
#include <cstddef>
#include <format>

namespace lama::rv {

constexpr int WORD_SIZE = 8;

struct Register {
    size_t regno;

public:

    static constexpr Register zero() {
        return {0};
    }
    static constexpr Register temp1() {
        return {30};
    }
    static constexpr Register ra() {
        return {1};
    }
    static constexpr Register gp() {
        return {5};
    }
    static constexpr Register sp() {
        return {2};
    }
    static constexpr Register fp() {
        return {8};
    }
    static constexpr Register closurep() {
        return {5};
    }

    static Register arg(size_t argno) {
        DCHECK_LT(argno, 8) << "argument register number out of range";
        return {(size_t)(10 + argno)};
    }

    static void saved_apply(auto const& f) {
        int i = 0;
        for (size_t r : {8, 9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27}) {
            f(Register{r}, ++i);
        }
    }

    static void temp_apply(auto const& f) {
        int i = 0;
        for (size_t r : {5, 6, 7, 28, 29, 30, 31}) {
            f(Register{r}, ++i);
        }
    }

    static void arg_apply(auto const& f) {
        int i = 0;
        for (size_t r : {10, 11, 12, 13, 14, 15, 16, 17}) {
            f(Register{r}, ++i);
        }
    }

    char const* to_string() {
        constexpr static char const* reg_names[] = {"zero", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2",
                                                    "fp",   "s1", "a0",  "a1",  "a2", "a3", "a4", "a5",
                                                    "a6",   "a7", "s2",  "s3",  "s4", "s5", "s6", "s7",
                                                    "s8",   "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
        return reg_names[regno];
    }
};

}  // namespace lama::rv

template <>
struct std::formatter<lama::rv::Register> : std::formatter<std::string> {
    auto format(lama::rv::Register p, format_context& ctx) const {
        return formatter<string>::format(p.to_string(), ctx);
    }
};
