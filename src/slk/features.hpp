#include "core.hpp"

#define SB_FEATURE_INCLUDE_INLINE SB_FEATURE_ENABLED

#if sb_feature_enabled(INCLUDE_INLINE)
#   define sb_inline_hpp(path) path
#   define sb_inline_cpp(path) <slk/stub.hpp>
#   define sb_inline inline
#else
#   define sb_inline_hpp(path) <slk/stub.hpp>
#   define sb_inline_cpp(path) path
#   define sb_inline 
#endif

#if defined(__clang__)
#    define sb_force_inline [[clang::always_inline]]
#else
#    define sb_force_inline
#endif

