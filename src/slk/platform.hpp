#pragma once

// Target platform specifics e.g. compiler intrinsics abstration, architecture, identifier, etc.

#if defined(__clang__)
#    define sb_force_inline [[clang::always_inline]]
#else
#    define sb_force_inline
#endif

