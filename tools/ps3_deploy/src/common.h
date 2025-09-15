#pragma once

#include <core/base_types.hpp>

// TODO(ebeau): Change to constexpr
#define PS3LOAD_VERSION_MAYOR 0
#define PS3LOAD_VERSION_MINOR 5
#define LD_TCP_PORT 4299

struct MsgHeader
{
    slk::u32 compressed_size;
    slk::u32 uncomressed_size;
    slk::u16 arg_cnt;
    slk::u8 major_version;
    slk::u8 minor_version;
};

// TODO(ebeau): move to cpp
static_assert(sizeof(MsgHeader) == 12, "MsgHeader struct size mismatch");
