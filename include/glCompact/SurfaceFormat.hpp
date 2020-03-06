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
#pragma once
#include <cstdint> //C++11

namespace glCompact {
    struct SurfaceFormatDetail;

    class SurfaceFormat {
            friend class SurfaceInterface;
            friend class TextureInterface;
            friend class Frame;
            friend class PipelineInterface;
            friend class RenderBuffer2d;
            friend class RenderBuffer2dMultisample;
        public:
            enum FormatEnum {
                R8_UNORM = 1,
                R8_SNORM,
                R8_UINT,
                R8_SINT,
                R16_UNORM,
                R16_SNORM,
                R16_UINT,
                R16_SINT,
                R32_UINT,
                R32_SINT,
                R16_SFLOAT,
                R32_SFLOAT,

                R8G8_UNORM,
                R8G8_SNORM,
                R8G8_UINT,
                R8G8_SINT,
                R16G16_UNORM,
                R16G16_SNORM,
                R16G16_UINT,
                R16G16_SINT,
                R32G32_UINT,
                R32G32_SINT,
                R16G16_SFLOAT,
                R32G32_SFLOAT,

              //R8G8B8_UNORM,
              //R8G8B8_SNORM,
              //R8G8B8_UINT,
              //R8G8B8_SINT,
              //R16G16B16_UNORM,
              //R16G16B16_SNORM,
              //R16G16B16_UINT,
              //R16G16B16_SINT,
              //R32G32B32_UNORM,
              //R32G32B32_SNORM,
              //R16G16B16_SFLOAT,
              //R32G32B32_SFLOAT,

                R8G8B8A8_UNORM,
                R8G8B8A8_SNORM,
                R8G8B8A8_UINT,
                R8G8B8A8_SINT,
                R16G16B16A16_UNORM,
                R16G16B16A16_SNORM,
                R16G16B16A16_UINT,
                R16G16B16A16_SINT,
                R32G32B32A32_UINT,
                R32G32B32A32_SINT,
                R16G16B16A16_SFLOAT,
                R32G32B32A32_SFLOAT,

                //special color formats usable for texture and image
                R11G11B10_UFLOAT,
                R10G10B10A2_UNORM,
                R10G10B10A2_UINT,

                //texture sample only formats (not usable as image for compute shader read/write)
                //TODO: throw this formats out? Non has decent real memory aligenment size.
                //Maybe only take this two formats?: GL_RGB9_E5, GL_RGB5_A1

                R9G9B9E5_UFLOAT_PACK32, //GL_RGB9_E5, //single 5 bit exponent for all 3 values, no sign; vk only has E5B9G9R9_UFLOAT_PACK32 format
                R5G5B5A1_UNORM_PACK16,  //GL_RGB5_A1

                R5G6B5_UNORM_PACK16,    //GL_RGB565
              //R3G3B2_UNORM_PACK8.     //GL_R3_G3_B2
              //R4G4B4_UNORM,           //GL_RGB4
              //R5G5B5_UNORM,           //GL_RGB5
              //R10G10B10_UNORM,        //GL_RGB10
              //R12G12B12_UNORM,        //GL_RGB12
              //R2G2B2A2_UNORM_PACK8,   //GL_RGBA2
                R4G4B4A4_UNORM_PACK16,  //GL_RGBA4
              //R12G12B12A12_UNORM,     //GL_RGBA12

                //sRGB formats
                R8G8B8_SRGB,            //GL_SRGB8,
                R8G8B8A8_SRGB,          //GL_SRGB8_ALPHA8

                //depth
                D16_UNORM,
                D24_UNORM,
                D32_UNORM,
                D32_SFLOAT,

                //depth stencil (Many drivers only support FBOs with depth and stencil via this combinaton formats!)
                D24_UNORM_S8_UINT,
                D32_SFLOAT_S8_UINT,

                //stencil index (sadly only supported as core relatively late in OpenGL), VK only supports 8bit stencil formats. Maybe copy for simplicity/bug hit prevention?
              //S1_UINT,
              //S4_UINT,
                S8_UINT,
              //S16_UINT,

                //Compressed Formats
                //Except ASTC, all compressed formats use blocks of 4x4 pixels

                //S3 Texture Compression (S3TC)
                //GL_EXT_texture_compression_s3tc (Not core, ubiquitous extension)
                //GL_EXT_texture_compression_dxt1
                //GL_NV_texture_compression_vtc (VTC 3D texture compression) is S3TC for 3d textures
                //The original S3TC comes in 6 variants (ignoring sRGB), OpenGL only implements 4 of them!
                //BC1 (DXT1) without alpha
                //BC1 (DXT1) 1-bit alpha        <-               (RGB values pre-multiplied with alpha, an alpha bit 0 means RGB is black!)
                //BC2 (DXT2) Explicit alpha     <- NOT in OpenGL (RGB values pre-multiplied with alpha value)
                //BC2 (DXT3) Explicit alpha
                //BC3 (DXT4) Interpolated alpha <- NOT in OpenGL (RGB values pre-multiplied with alpha value)
                //BC3 (DXT5) Interpolated alpha
                BC1_RGB_UNORM_BLOCK = 1000, //BC1 (DXT1)
                BC1_RGBA_UNORM_BLOCK,       //BC1 (DXT1) with alpha bit
                BC2_UNORM_BLOCK,            //BC2 (DXT3)
                BC3_UNORM_BLOCK,            //BC3 (DXT5)
                //GL_EXT_texture_compression_s3tc_srgb (Not core)
                BC1_RGB_SRGB_BLOCK,         //BC1 (DXT1)
                BC1_RGBA_SRGB_BLOCK,        //BC1 (DXT1) with alpha bit
                BC2_SRGB_BLOCK,             //BC2 (DXT3)
                BC3_SRGB_BLOCK,             //BC3 (DXT5)

                //Red Green Texture Compression (RGTC)
                //GL_ARB_texture_compression_rgtc (Core since 3.0)
                //RG formats have sepperate interpolation, so they can be used for e.g. normals without artefacting
                BC4_UNORM_BLOCK,            //unsigned R
                BC4_SNORM_BLOCK,            //signed   R
                BC5_UNORM_BLOCK,            //unsigned RG
                BC5_SNORM_BLOCK,            //signed   RG

                //BPTC
                //GL_EXT_texture_compression_bptc
                //GL_ARB_texture_compression_bptc (Core since 4.2)
                BC6H_UFLOAT_BLOCK,
                BC6H_SFLOAT_BLOCK,
                BC7_UNORM_BLOCK,
                BC7_SRGB_BLOCK,

                //Ericsson Texture Compression (ETC2/EAC)
                //ETC2 is basically ETC1, but making use of some previous unused bit paterns for new modes
                ETC2_R8G8B8_UNORM_BLOCK,    //RGB ETC2
                ETC2_R8G8B8_SRGB_BLOCK,     //RGB ETC2 with sRGB encoding
                ETC2_R8G8B8A1_UNORM_BLOCK,  //RGB ETC2 with punch-through alpha
                ETC2_R8G8B8A1_SRGB_BLOCK,   //RGB ETC2 with punch-through alpha and sRGB
                ETC2_R8G8B8A8_UNORM_BLOCK,  //RGBA ETC2
                ETC2_R8G8B8A8_SRGB_BLOCK,   //RGBA ETC2 with sRGB encoding
                EAC_R11_UNORM_BLOCK,        //Unsigned R11 EAC
                EAC_R11_SNORM_BLOCK,        //Signed R11 EAC
                EAC_R11G11_UNORM_BLOCK,     //Unsigned RG11 EAC
                EAC_R11G11_SNORM_BLOCK,     //Signed RG11 EAC

                //Adaptable Scalable Texture Compression (ASTC)
                //GL_KHR_texture_compression_astc_hdr (high dynamic range) (Core since 4.3)
                //GL_KHR_texture_compression_astc_ldr (low dynamic range) and GL_KHR_texture_compression_astc_sliced_3d
                //GL_OES_texture_compression_astc
                //
                //GL_EXT_texture_compression_astc_decode_mode,
                //GL_EXT_texture_compression_astc_decode_mode_rgb9e5 (adds extra decode precision format GL_RGB9_E5)
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

                //GL_EXT_texture_compression_latc
                //GL_NV_texture_compression_latc (legacy)
                //Old LUMINANCE compressed formats - not going to implement this one
            };

            SurfaceFormat(): formatEnum(static_cast<FormatEnum>(0)){};
            SurfaceFormat(FormatEnum formatEnum): formatEnum(formatEnum){}
            SurfaceFormat& operator=(FormatEnum formatEnum){
                this->formatEnum = formatEnum;
                return *this;
            }
            /*inline bool operator!=(const SurfaceFormat& if1, const SurfaceFormat& if2){
                return if1.formatEnum != if2.formatEnum;
            }*/
            inline bool operator!=(const SurfaceFormat& surfaceFormat){
                return this->formatEnum != surfaceFormat.formatEnum;
            }
        private:
            FormatEnum formatEnum;
            const SurfaceFormatDetail* operator->() const;
    };
}
