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

namespace glCompact {
    struct MemorySurfaceFormatDetail;

    class MemorySurfaceFormat {
        public:
            //- f64 does not exist as surface format so there also is not memory surface format for it.
            enum FormatEnum {
                R8_UNORM,
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

                G8_UNORM,
                G8_SNORM,
                G8_UINT,
                G8_SINT,
                G16_UNORM,
                G16_SNORM,
                G16_UINT,
                G16_SINT,
                G32_UNORM,
                G32_SNORM,
                G32_UINT,
                G32_SINT,
                G16_SFLOAT,
                G32_SFLOAT,

                B8_UNORM,
                B8_SNORM,
                B8_UINT,
                B8_SINT,
                B16_UNORM,
                B16_SNORM,
                B16_UINT,
                B16_SINT,
                B32_UNORM,
                B32_SNORM,
                B32_UINT,
                B32_SINT,
                B16_SFLOAT,
                B32_SFLOAT,

                A8_UNORM,
                A8_SNORM,
                A8_UINT,
                A8_SINT,
                A16_UNORM,
                A16_SNORM,
                A16_UINT,
                A16_SINT,
                A32_UNORM,
                A32_SNORM,
                A32_UINT,
                A32_SINT,
                A16_SFLOAT,
                A32_SFLOAT,

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

                B8G8R8_UNORM,
                B8G8R8_SNORM,
                B8G8R8_UINT,
                B8G8R8_SINT,
                B16G16R16_UNORM,
                B16G16R16_SNORM,
                B16G16R16_UINT,
                B16G16R16_SINT,
                B32G32R32_UNORM,
                B32G32R32_SNORM,
                B32G32R32_UINT,
                B32G32R32_SINT,
                B16G16R16_SFLOAT,
                B32G32R32_SFLOAT,

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

                B8G8R8A8_UNORM,
                B8G8R8A8_SNORM,
                B8G8R8A8_UINT,
                B8G8R8A8_SINT,
                B16G16R16A16_UNORM,
                B16G16R16A16_SNORM,
                B16G16R16A16_UINT,
                B16G16R16A16_SINT,
                B32G32R32A32_UNORM,
                B32G32R32A32_SNORM,
                B32G32R32A32_UINT,
                B32G32R32A32_SINT,
                B16G16R16A16_SFLOAT,
                B32G32R32A32_SFLOAT,

                R3G3B2_UNORM,
                R3G3B2_UINT,
                B2G3R3_UNORM,
                B2G3R3_UINT,

                R5G6B5_UNORM_PACK16,
                R5G6B5_UINT_PACK16,
                B5G6R5_UNORM_PACK16,
                B5G6R5_UINT_PACK16,

                R4G4B4A4_UNORM_PACK16,
                R4G4B4A4_UINT_PACK16,
                B4G4R4A4_UNORM_PACK16,
                B4G4R4A4_UINT_PACK16,
                A4B4G4R4_UNORM_PACK16,
                A4B4G4R4_UINT_PACK16,
                A4R4G4B4_UNORM_PACK16,
                A4R4G4B4_UINT_PACK16,

                R5G5B5A1_UNORM_PACK16,
                R5G5B5A1_UINT_PACK16,
                B5G5R5A1_UNORM_PACK16,
                B5G5R5A1_UINT_PACK16,
                A1B5G5R5_UNORM_PACK16,
                A1B5G5R5_UINT_PACK16,
                A1R5G5B5_UNORM_PACK16,
                A1R5G5B5_UINT_PACK16,

                A8B8G8R8_UNORM_PACK32,
                A8B8G8R8_UINT_PACK32,
                A8R8G8B8_UNORM_PACK32,
                A8R8G8B8_UINT_PACK32,

                R10G10B10A2_UNORM_PACK32,
                R10G10B10A2_UINT_PACK32,
                B10G10R10A2_UNORM_PACK32,
                B10G10R10A2_UINT_PACK32,
                A2B10G10R10_UNORM_PACK32,
                A2B10G10R10_UINT_PACK32,
                A2R10G10B10_UNORM_PACK32,
                A2R10G10B10_UINT_PACK32,

                B10G11R11_UFLOAT, //10 bit UFLOAT = E5M5, 11 bit UFLOAT = E5M6
                E5B9G9R9_UFLOAT,  //shared 5 bit exponent

                //depth
                D16_UNORM,
                D24_UNORM,
                D32_UNORM,
                D32_SFLOAT,

                //depth stencil
                D24_UNORM_S8_UINT,
                //Only the memory format defines unused bits! The ImageFormat definition does not contain this part, because most GPUs do allocate two sepperate buffers and have no unused bits!
                D32_SFLOAT_X24_S8_UINT,

                //stencil
              //S1_UINT,
              //S4_UINT,
                S8_UINT,
              //S16_UINT,


                //Compressed formats
                //S3 Texture Compression (S3TC)
                BC1_RGB_UNORM_BLOCK = 1000,
                BC1_RGBA_UNORM_BLOCK,
                BC2_UNORM_BLOCK,
                BC3_UNORM_BLOCK,
                BC1_RGB_SRGB_BLOCK,
                BC1_RGBA_SRGB_BLOCK,
                BC2_SRGB_BLOCK,
                BC3_SRGB_BLOCK,

                //Red Green Texture Compression (RGTC)
                BC4_UNORM_BLOCK,
                BC4_SNORM_BLOCK,
                BC5_UNORM_BLOCK,
                BC5_SNORM_BLOCK,

                //BPTC
                BC6H_UFLOAT_BLOCK,
                BC6H_SFLOAT_BLOCK,
                BC7_UNORM_BLOCK,
                BC7_SRGB_BLOCK,

                //Ericsson Texture Compression (ETC2/EAC)
                ETC2_R8G8B8_UNORM_BLOCK,
                ETC2_R8G8B8_SRGB_BLOCK,
                ETC2_R8G8B8A1_UNORM_BLOCK,
                ETC2_R8G8B8A1_SRGB_BLOCK,
                ETC2_R8G8B8A8_UNORM_BLOCK,
                ETC2_R8G8B8A8_SRGB_BLOCK,
                EAC_R11_UNORM_BLOCK,
                EAC_R11_SNORM_BLOCK,
                EAC_R11G11_UNORM_BLOCK,
                EAC_R11G11_SNORM_BLOCK,

                //Adaptable Scalable Texture Compression (ASTC)
                ASTC_4x4_UNORM_BLOCK,
                ASTC_4x4_SRGB_BLOCK,
                ASTC_5x4_UNORM_BLOCK,
                ASTC_5x4_SRGB_BLOCK,
                ASTC_5x5_UNORM_BLOCK,
                ASTC_5x5_SRGB_BLOCK,
                ASTC_6x5_UNORM_BLOCK,
                ASTC_6x5_SRGB_BLOCK,
                ASTC_6x6_UNORM_BLOCK,
                ASTC_6x6_SRGB_BLOCK,
                ASTC_8x5_UNORM_BLOCK,
                ASTC_8x5_SRGB_BLOCK,
                ASTC_8x6_UNORM_BLOCK,
                ASTC_8x6_SRGB_BLOCK,
                ASTC_8x8_UNORM_BLOCK,
                ASTC_8x8_SRGB_BLOCK,
                ASTC_10x5_UNORM_BLOCK,
                ASTC_10x5_SRGB_BLOCK,
                ASTC_10x6_UNORM_BLOCK,
                ASTC_10x6_SRGB_BLOCK,
                ASTC_10x8_UNORM_BLOCK,
                ASTC_10x8_SRGB_BLOCK,
                ASTC_10x10_UNORM_BLOCK,
                ASTC_10x10_SRGB_BLOCK,
                ASTC_12x10_UNORM_BLOCK,
                ASTC_12x10_SRGB_BLOCK,
                ASTC_12x12_UNORM_BLOCK,
                ASTC_12x12_SRGB_BLOCK,
            };

            MemorySurfaceFormat(FormatEnum formatEnum): formatEnum(formatEnum){}
            MemorySurfaceFormat& operator=(FormatEnum formatEnum){
                this->formatEnum = formatEnum;
                return *this;
            }
            const MemorySurfaceFormatDetail* operator->() const;
        private:
            FormatEnum formatEnum;
    };
}
