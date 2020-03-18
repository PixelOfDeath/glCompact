///@file
#pragma once
#include <cstdint> //C++11

namespace glCompact {
    enum class FaceSelection : int32_t {
        front        = 0x0404, //GL_FRONT,
        back         = 0x0405, //GL_BACK,
        frontAndBack = 0x0408  //GL_FRONT_AND_BACK
    };
}
