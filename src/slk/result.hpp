#pragma once

#include "core.hpp"

import std;

namespace slk {

enum class ErrorCode : u8 {
    NOT_FOUND,
    OUT_OF_MEMORY,
};

// note: no warning on discard, mark returning functions [[nodiscard]]
template<typename T>
using Result = std::expected<T, ErrorCode>;

using Status sb_warn_unused = Result<void>;

sb_force_inline constexpr std::unexpected<ErrorCode> fail(ErrorCode code) { return std::unexpected(code); }
consteval Status ok() { return {}; }

} // namespace slk

