///@file
#pragma once
#include <cstdint> //C++11

namespace glCompact {
    enum class CompareOperator : int32_t {
      //never          = 0x0200, //GL_NEVER //remove it because it has no use!?
        less           = 0x0201, //GL_LESS
        equal          = 0x0202, //GL_EQUAL
        lessOrEqual    = 0x0203, //GL_LEQUAL
        greater        = 0x0204, //GL_GREATER
        unequal        = 0x0205, //GL_NOTEQUAL
        greaterOrEqual = 0x0206, //GL_GEQUAL
        disabled       = 0x0207  //GL_ALWAYS
    };
}
