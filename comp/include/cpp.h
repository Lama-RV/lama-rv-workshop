#pragma once

#include <type_traits>

template <class Enum>
constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept {
    return static_cast<std::underlying_type_t<Enum>>(e);
};

template <class... Ts>
struct overloads : Ts... {
    using Ts::operator()...;
};
