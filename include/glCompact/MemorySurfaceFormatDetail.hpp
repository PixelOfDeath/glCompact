#pragma once
#include <cstdint> //C++11

namespace glCompact {
    struct MemorySurfaceFormatDetail {
        const char*const name;
        int32_t sizedFormat; //To check if compressed formats are the same as SurfaceFormat and support raw memory copy
        int32_t componentsAndArrangement;
        int32_t componentsTypes;
        uint8_t bytePerPixelOrBlock;
        uint8_t blockSizeX;
        uint8_t blockSizeY;

        bool isRgbaNormalizedIntegerOrFloat : 1;
        bool isRgbaInteger                  : 1;
        bool isDepth                        : 1;
        bool isStencil                      : 1;
        bool isCompressed                   : 1;
        bool isSigned                       : 1;
    };
}
