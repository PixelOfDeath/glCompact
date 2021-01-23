#pragma once
#include <cstdint> //C++11

namespace glCompact {
    struct SurfaceFormatDetail {
        const char* name;
        int32_t sizedFormat;
        int32_t components;        //e.g. GL_RED
        int32_t componentsTypes;   //e.g. GL_UNSIGNED_BYTE
        int32_t attachmentType;
        int16_t bitsPerPixelOrBlock;
        uint8_t blockSizeX;
        uint8_t blockSizeY;
        bool isRenderable                   : 1;
        bool isCompressed                   : 1;
        bool isSrgb                         : 1;
        bool imageSupport                   : 1;
        bool sparseSupport                  : 1;
      //bool   nonZeroBorder:1; maybe need this for some compressed formats

        bool isRgbaNormalizedIntegerOrFloat : 1;
        bool isRgbaInteger                  : 1;
        bool isDepth                        : 1;
        bool isStencil                      : 1;
        bool isSigned                       : 1;
    };
}
