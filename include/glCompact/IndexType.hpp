#pragma once
#include <cstdint> //C++11

namespace glCompact {
    enum class IndexType : int32_t {
        UINT16 = 0x1403, //GL_UNSIGNED_SHORT,
        UINT32 = 0x1405  //GL_UNSIGNED_INT
    };
}
