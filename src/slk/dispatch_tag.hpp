#pragma once

#include "std/concepts.hpp"

#define sb_declare_dispatch_tag(tag) \
    struct tag##Tag {};

namespace slk {

template <slk::EnumerationType auto V>
struct ETag { };

} // namespace slk
