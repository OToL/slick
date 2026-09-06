#pragma once

import std;

namespace slk {

template <std::size_t N, typename T>
std::span<T, N> makeFixSubSpan(std::span<T> val, std::size_t offset = 0)
{
    return std::span<T, N>{val.subspan(offset, N)};
}

}

