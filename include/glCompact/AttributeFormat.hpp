/*
    glCompact
    Copyright (C) 2019-2020 Frederik Uje vom Hofe

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#pragma once
#include <cstdint> //C++11

namespace glCompact {
    struct AttributeFormatDetail {
        int32_t componentsType;        //Base type or complex type. E.g. GL_UNSIGNED_BYTE, or special type like GL_INT_2_10_10_10_REV
        int32_t componentsCountOrBGRA; //1, 2, 3, 4 or GL_BGRA
        bool    normalized;
        uint8_t byteSize;
    };

    class AttributeFormat {
            friend class AttributeLayout;
            friend class AttributeLayoutStates;
            friend class PipelineRasterization;
        public:
            enum FormatEnum : uint8_t {
                R8_UNORM = 1,
                R8_SNORM,
                R8_UINT,
                R8_SINT,
                R16_UNORM,
                R16_SNORM,
                R16_UINT,
                R16_SINT,
                R32_UNORM,
                R32_SNORM,
                R32_UINT,
                R32_SINT,
                R16_SFLOAT,
                R32_SFLOAT,
                R64_SFLOAT,

                R8G8_UNORM,
                R8G8_SNORM,
                R8G8_UINT,
                R8G8_SINT,
                R16G16_UNORM,
                R16G16_SNORM,
                R16G16_UINT,
                R16G16_SINT,
                R32G32_UNORM,
                R32G32_SNORM,
                R32G32_UINT,
                R32G32_SINT,
                R16G16_SFLOAT,
                R32G32_SFLOAT,
                R64G64_SFLOAT,

                //just prevent non 4byte aligned data, or 3 vector sized locations in general? But probably only the whole vertex block alignment matters?! (TODO: test test test)
                R8G8B8_UNORM,
                R8G8B8_SNORM,
                R8G8B8_UINT,
                R8G8B8_SINT,
                R16G16B16_UNORM,
                R16G16B16_SNORM,
                R16G16B16_UINT,
                R16G16B16_SINT,
                R32G32B32_UNORM,
                R32G32B32_SNORM,
                R32G32B32_UINT,
                R32G32B32_SINT,
                R16G16B16_SFLOAT,
                R32G32B32_SFLOAT,
                R64G64B64_SFLOAT,

                R8G8B8A8_UNORM,
                R8G8B8A8_SNORM,
                R8G8B8A8_UINT,
                R8G8B8A8_SINT,
                R16G16B16A16_UNORM,
                R16G16B16A16_SNORM,
                R16G16B16A16_UINT,
                R16G16B16A16_SINT,
                R32G32B32A32_UNORM,
                R32G32B32A32_SNORM,
                R32G32B32A32_UINT,
                R32G32B32A32_SINT,
                R16G16B16A16_SFLOAT,
                R32G32B32A32_SFLOAT,
                R64G64B64A64_SFLOAT,

                //BGRA comes in normalized only
                B8G8R8A8_UNORM,    //BGRA

                A2B10G10R10_UNORM, //RGBA GL_UNSIGNED_INT_2_10_10_10_REV
                A2B10G10R10_SNORM, //RGBA GL_INT_2_10_10_10_REV
                A2R10G10B10_UNORM, //BGRA GL_UNSIGNED_INT_2_10_10_10_REV
                A2R10G10B10_SNORM, //BGRA GL_INT_2_10_10_10_REV

                B10G11R11_UFLOAT   //GL_UNSIGNED_INT_10F_11F_11F_REV (Needs at last GL4.4)

                //GL_ARB_ES2_compatibility
                //GL_FIXED
            };
            AttributeFormat(){};
            AttributeFormat(FormatEnum formatEnum): formatEnum(formatEnum){}
            AttributeFormat& operator=(FormatEnum formatEnum){
                this->formatEnum = formatEnum;
                return *this;
            }
            inline bool operator==(const AttributeFormat& attributeFormat) const {
                return this->formatEnum == attributeFormat.formatEnum;
            }
            inline bool operator!=(const AttributeFormat& attributeFormat) const {
                return this->formatEnum != attributeFormat.formatEnum;
            }
            inline const AttributeFormatDetail* operator->() const {
                return &attributeFormatDetail[formatEnum];
            }
        private:
            static constexpr FormatEnum NONE = static_cast<FormatEnum>(0);
            FormatEnum formatEnum = NONE;
            static const AttributeFormatDetail attributeFormatDetail[];
    };
}
