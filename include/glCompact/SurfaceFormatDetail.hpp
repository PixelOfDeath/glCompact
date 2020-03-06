/*
    This file is part of glCompact.
    Copyright (C) 2019-2020 Frederik Uje vom Hofe

    glCompact is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    glCompact is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
*/
///\cond HIDDEN_FROM_DOXYGEN
#pragma once
#include <cstdint> //C++11

namespace glCompact {
    struct SurfaceFormatDetail {
        const char*const name;
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
///\endcond
