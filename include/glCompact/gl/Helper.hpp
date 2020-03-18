#pragma once
#include <cstdint> //C++11
#include <string>

namespace glCompact {
    namespace gl {
        std::string typeToGlslAndCNameString (int32_t type);
        std::string typeTopConstantNameString(int32_t type);
        int32_t typeToBaseType               (int32_t type);
        int32_t typeToBaseTypeCount          (int32_t type);
        int32_t typeSize                     (int32_t type);
        bool typeIsSampler                   (int32_t type);
        bool typeIsImage                     (int32_t type);
        std::string errorToString            (int32_t error);
    }
}
