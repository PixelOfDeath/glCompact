#pragma once
#include <cstdint> //C++11

//uint8_t is not listed here because it is often not supported in hardware and its usage can lead to extreme performance drops!

namespace glCompact {
    enum class IndexType : int32_t {
        UINT16 = 0x1403, //GL_UNSIGNED_SHORT,
        UINT32 = 0x1405  //GL_UNSIGNED_INT
    };
}
