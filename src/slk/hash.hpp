#pragma once

#include "core.hpp"
#include <slk/std/bits.hpp>

#include <rapidhash/rapidhash.h>

import std;

namespace slk {

struct [[nodiscard]] Hash64 {
    u64 m_value = 0;

    constexpr b8 isValid() const {
        return m_value != 0;
    }

    // order-sensitive: a.combined(b) != b.combined(a), and a.combined(b, c) == a.combined(b).combined(c)
    constexpr Hash64 combined(std::same_as<Hash64> auto... vals) const requires (sizeof...(vals) > 0) {
        u64 val = m_value;
        ((val = scrambleBits(std::rotl(val, 23) + vals.m_value + 0x9e3779b97f4a7c15)), ...);
        return {val};
    }

    constexpr b8 operator==(Hash64 const&) const = default;

    static constexpr Hash64 make(void const* data, size_t len) {
        return {rapidhash(data, len)};
    }

    static constexpr Hash64 make(std::string_view str) {
        if consteval {
            std::vector<u8> const bytes(str.begin(), str.end()); // chars cannot be read as bytes during constant evaluation
            return {rapidhash(bytes.data(), bytes.size())};
        } else {
            return {rapidhash(str.data(), str.size())};
        }
    }

    // same as first.combined(rest...), e.g. Hash64::combine(mesh_id, material_id)
    static constexpr Hash64 combine(Hash64 first, std::same_as<Hash64> auto... rest) requires (sizeof...(rest) > 0) {
        return first.combined(rest...);
    }
};

namespace literals {
    consteval Hash64 operator""_h64(char const* data, size_t size) {
        return Hash64::make(std::string_view{data, size}); // string_view: the void* overload is not constexpr-usable with chars
    }

} // namespace literals

} // namespace slk

template <>
struct std::hash<slk::Hash64> {
    size_t operator()(slk::Hash64 h) const noexcept {
        return h.m_value;
    } // already well distributed
};
