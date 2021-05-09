#include "glCompact/SurfaceFormat.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/SurfaceFormatDetail.hpp"
#include "glCompact/MemorySurfaceFormatDetail.hpp"
#include "glCompact/threadContext_.hpp"
#include <stdexcept>
#include <string>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    const SurfaceFormatDetail SurfaceFormat::detail() const {
        switch (formatEnum) {
          //                                           name                           sizedFormat                                    attachmentType                  bitsPerPixel
          //                                           |                             |                                               |                               |    blockSizeX
          //                                           |                             |                                               |                               |    |   blockSizeY
          //                                           |                             |                                               |                               |    |   |    isRenderable
          //                                           |                             |                                               |                               |    |   |    |  isCompressed
          //                                           |                             |                                               |                               |    |   |    |  |  isSrgb
          //                                           |                             |                                               |                               |    |   |    |  |  |  imageSupport
          //                                           |                             |                                               |                               |    |   |    |  |  |  |  sparseSupport
          //                                           |                             |                                               |                               |    |   |    |  |  |  |  |   isRgbaNormalizedIntegerOrFloat
          //                                           |                             |                                               |                               |    |   |    |  |  |  |  |   |  isRgbaInteger
          //                                           |                             |                                               |                               |    |   |    |  |  |  |  |   |  |  isDepth
          //                                           |                             |                                               |                               |    |   |    |  |  |  |  |   |  |  |  isStencil
          //                                           |                             |                                               |                               |    |   |    |  |  |  |  |   |  |  |  |  isSigned
          //                                           |                             |                                               |                               |    |   |    |  |  |  |  |   |  |  |  |  |
            case R8_UNORM                   : return {"R8_UNORM",                    GL_R8,                                          GL_COLOR_ATTACHMENT0,           8,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 0};
            case R8_SNORM                   : return {"R8_SNORM",                    GL_R8_SNORM,                                    GL_COLOR_ATTACHMENT0,           8,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 1};
            case R8_UINT                    : return {"R8_UINT",                     GL_R8UI,                                        GL_COLOR_ATTACHMENT0,           8,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 0};
            case R8_SINT                    : return {"R8_SINT",                     GL_R8I,                                         GL_COLOR_ATTACHMENT0,           8,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 1};
            case R16_UNORM                  : return {"R16_UNORM",                   GL_R16,                                         GL_COLOR_ATTACHMENT0,          16,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 0};
            case R16_SNORM                  : return {"R16_SNORM",                   GL_R16_SNORM,                                   GL_COLOR_ATTACHMENT0,          16,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 1};
            case R16_UINT                   : return {"R16_UINT",                    GL_R16UI,                                       GL_COLOR_ATTACHMENT0,          16,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 0};
            case R16_SINT                   : return {"R16_SINT",                    GL_R16I,                                        GL_COLOR_ATTACHMENT0,          16,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 1};
            case R32_UINT                   : return {"R32_UINT",                    GL_R32UI,                                       GL_COLOR_ATTACHMENT0,          16,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 0};
            case R32_SINT                   : return {"R32_SINT",                    GL_R32I,                                        GL_COLOR_ATTACHMENT0,          32,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 1};
            case R16_SFLOAT                 : return {"R16_SFLOAT",                  GL_R16F,                                        GL_COLOR_ATTACHMENT0,          16,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 1};
            case R32_SFLOAT                 : return {"R32_SFLOAT",                  GL_R32F,                                        GL_COLOR_ATTACHMENT0,          32,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 1};

            case R8G8_UNORM                 : return {"R8G8_UNORM",                  GL_RG8,                                         GL_COLOR_ATTACHMENT0,          16,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 0};
            case R8G8_SNORM                 : return {"R8G8_SNORM",                  GL_RG8_SNORM,                                   GL_COLOR_ATTACHMENT0,          16,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 1};
            case R8G8_UINT                  : return {"R8G8_UINT",                   GL_RG8UI,                                       GL_COLOR_ATTACHMENT0,          16,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 0};
            case R8G8_SINT                  : return {"R8G8_SINT",                   GL_RG8I,                                        GL_COLOR_ATTACHMENT0,          16,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 1};
            case R16G16_UNORM               : return {"R16G16_UNORM",                GL_RG16,                                        GL_COLOR_ATTACHMENT0,          32,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 0};
            case R16G16_SNORM               : return {"R16G16_SNORM",                GL_RG16_SNORM,                                  GL_COLOR_ATTACHMENT0,          32,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 1};
            case R16G16_UINT                : return {"R16G16_UINT",                 GL_RG16UI,                                      GL_COLOR_ATTACHMENT0,          32,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 0};
            case R16G16_SINT                : return {"R16G16_SINT",                 GL_RG16I,                                       GL_COLOR_ATTACHMENT0,          32,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 1};
            case R32G32_UINT                : return {"R32G32_UINT",                 GL_RG32UI,                                      GL_COLOR_ATTACHMENT0,          64,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 0};
            case R32G32_SINT                : return {"R32G32_SINT",                 GL_RG32I,                                       GL_COLOR_ATTACHMENT0,          64,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 1};
            case R16G16_SFLOAT              : return {"R16G16_SFLOAT",               GL_RG16F,                                       GL_COLOR_ATTACHMENT0,          32,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 1};
            case R32G32_SFLOAT              : return {"R32G32_SFLOAT",               GL_RG32F,                                       GL_COLOR_ATTACHMENT0,          64,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 1};

          //NOTE: also not garantied to be color renderable
          //case R8G8B8_UNORM               : return {"R8G8B8_UNORM",                GL_RGB8,                                        0,                             24,   1,  1,   0, 0, 0, 0, 0,  1, 0, 0, 0, 0};
          //case R8G8B8_SNORM               : return {"R8G8B8_SNORM",                GL_RGB8_SNORM,                                  0,                             24,   1,  1,   0, 0, 0, 0, 0,  1, 0, 0, 0, 1};
          //case R8G8B8_UINT                : return {"R8G8B8_UINT",                 GL_RGB8UI,                                      0,                             24,   1,  1,   0, 0, 0, 0, 0,  0, 1, 0, 0, 0};
          //case R8G8B8_SINT                : return {"R8G8B8_SINT",                 GL_RGB8I,                                       0,                             24,   1,  1,   0, 0, 0, 0, 0,  0, 1, 0, 0, 1};
          //case R16G16B16_UNORM            : return {"R16G16B16_UNORM",             GL_RGB16,                                       0,                             48,   1,  1,   0, 0, 0, 0, 0,  1, 0, 0, 0, 0};
          //case R16G16B16_SNORM            : return {"R16G16B16_SNORM",             GL_RGB16_SNORM,                                 0,                             48,   1,  1,   0, 0, 0, 0, 0,  1, 0, 0, 0, 1};
          //case R16G16B16_UINT             : return {"R16G16B16_UINT",              GL_RGB16UI,                                     0,                             48,   1,  1,   0, 0, 0, 0, 0,  0, 1, 0, 0, 0};
          //case R16G16B16_SINT             : return {"R16G16B16_SINT",              GL_RGB16I,                                      0,                             48,   1,  1,   0, 0, 0, 0, 0,  0, 1, 0, 0, 1};
          //case R32G32B32_UNORM            : return {"R32G32B32_UNORM",             GL_RGB32UI,                                     0,                             96,   1,  1,   0, 0, 0, 0, 0,  0, 1, 0, 0, 0};
          //case R32G32B32_SNORM            : return {"R32G32B32_SNORM",             GL_RGB32I,                                      0,                             96,   1,  1,   0, 0, 0, 0, 0,  0, 1, 0, 0, 1};
          //case R16G16B16_SFLOAT           : return {"R16G16B16_SFLOAT",            GL_RGB16F,                                      0,                             48,   1,  1,   0, 0, 0, 0, 0,  1, 0, 0, 0, 1};
          //case R32G32B32_SFLOAT           : return {"R32G32B32_SFLOAT",            GL_RGB32F,                                      0,                             96,   1,  1,   0, 0, 0, 0, 0,  1, 0, 0, 0, 1};

            case R8G8B8A8_UNORM             : return {"R8G8B8A8_UNORM",              GL_RGBA8,                                       GL_COLOR_ATTACHMENT0,          32,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 0};
            case R8G8B8A8_SNORM             : return {"R8G8B8A8_SNORM",              GL_RGBA8_SNORM,                                 GL_COLOR_ATTACHMENT0,          32,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 1};
            case R8G8B8A8_UINT              : return {"R8G8B8A8_UINT",               GL_RGBA8UI,                                     GL_COLOR_ATTACHMENT0,          32,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 0};
            case R8G8B8A8_SINT              : return {"R8G8B8A8_SINT",               GL_RGBA8I,                                      GL_COLOR_ATTACHMENT0,          32,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 1};
            case R16G16B16A16_UNORM         : return {"R16G16B16A16_UNORM",          GL_RGBA16,                                      GL_COLOR_ATTACHMENT0,          64,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 0};
            case R16G16B16A16_SNORM         : return {"R16G16B16A16_SNORM",          GL_RGBA16_SNORM,                                GL_COLOR_ATTACHMENT0,          64,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 1};
            case R16G16B16A16_UINT          : return {"R16G16B16A16_UINT",           GL_RGBA16UI,                                    GL_COLOR_ATTACHMENT0,          64,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 0};
            case R16G16B16A16_SINT          : return {"R16G16B16A16_SINT",           GL_RGBA16I,                                     GL_COLOR_ATTACHMENT0,          64,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 1};
            case R32G32B32A32_UINT          : return {"R32G32B32A32_UINT",           GL_RGBA32UI,                                    GL_COLOR_ATTACHMENT0,         128,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 0};
            case R32G32B32A32_SINT          : return {"R32G32B32A32_SINT",           GL_RGBA32I,                                     GL_COLOR_ATTACHMENT0,         128,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 1};
            case R16G16B16A16_SFLOAT        : return {"R16G16B16A16_SFLOAT",         GL_RGBA16F,                                     GL_COLOR_ATTACHMENT0,          64,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 1};
            case R32G32B32A32_SFLOAT        : return {"R32G32B32A32_SFLOAT",         GL_RGBA32F,                                     GL_COLOR_ATTACHMENT0,         128,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 1};

            case B10G11R11_UFLOAT_PACK32    : return {"B10G11R11_UFLOAT_PACK32",     GL_R11F_G11F_B10F,                              GL_COLOR_ATTACHMENT0,          32,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 0};
            case R10G10B10A2_UNORM_PACK32   : return {"R10G10B10A2_UNORM_PACK32",    GL_RGB10_A2,                                    GL_COLOR_ATTACHMENT0,          32,   1,  1,   1, 0, 0, 1, 1,  1, 0, 0, 0, 0};
            case R10G10B10A2_UINT_PACK32    : return {"R10G10B10A2_UINT_PACK32",     GL_RGB10_A2UI,                                  GL_COLOR_ATTACHMENT0,          32,   1,  1,   1, 0, 0, 1, 1,  0, 1, 0, 0, 0};

            case R5G5B5A1_UNORM_PACK16      : return {"R5G5B5A1_UNORM_PACK16",       GL_RGB5_A1,                                     GL_COLOR_ATTACHMENT0,          16,   1,  1,   1, 0, 0, 0, 0,  1, 0, 0, 0, 0};
            case R5G6B5_UNORM_PACK16        : return {"R5G6B5_UNORM_PACK16",         GL_RGB565,                                      GL_COLOR_ATTACHMENT0,          16,   1,  1,   1, 0, 0, 0, 0,  1, 0, 0, 0, 0};
            case R4G4B4A4_UNORM_PACK16      : return {"R4G4B4A4_UNORM_PACK16",       GL_RGBA4,                                       GL_COLOR_ATTACHMENT0,          16,   1,  1,   1, 0, 0, 0, 0,  1, 0, 0, 0, 0};

            case E5B9G9R9_UFLOAT_PACK32     : return {"E5B9G9R9_UFLOAT_PACK32",      GL_RGB9_E5,                                     0,                             32,   1,  1,   0, 0, 0, 0, 1,  1, 0, 0, 0, 0}; //unsigned RGB9 values with shared 5 bit exponent

            case R3G3B2_UNORM_PACK8         : return {"R3G3B2_UNORM_PACK8",          GL_R3_G3_B2,                                    0,                              8,   1,  1,   0, 0, 0, 0, 0,  1, 0, 0, 0, 0};
            case R4G4B4_UNORM               : return {"R4G4B4_UNORM",                GL_RGB4,                                        0,                             12,   1,  1,   0, 0, 0, 0, 0,  1, 0, 0, 0, 0};
            case R5G5B5_UNORM               : return {"R5G5B5_UNORM",                GL_RGB5,                                        0,                             15,   1,  1,   0, 0, 0, 0, 0,  1, 0, 0, 0, 0};
            case R10G10B10_UNORM            : return {"R10G10B10_UNORM",             GL_RGB10,                                       0,                             30,   1,  1,   0, 0, 0, 0, 0,  1, 0, 0, 0, 0};
            case R12G12B12_UNORM            : return {"R12G12B12_UNORM",             GL_RGB12,                                       0,                             36,   1,  1,   0, 0, 0, 0, 0,  1, 0, 0, 0, 0};
            case R2G2B2A2_UNORM_PACK8       : return {"R2G2B2A2_UNORM_PACK8",        GL_RGBA2,                                       0,                              8,   1,  1,   0, 0, 0, 0, 0,  1, 0, 0, 0, 0};
            case R12G12B12A12_UNORM         : return {"R12G12B12A12_UNORM",          GL_RGBA12,                                      0,                             48,   1,  1,   0, 0, 0, 0, 0,  1, 0, 0, 0, 0};

            case R8G8B8_SRGB                : return {"R8G8B8_SRGB",                 GL_SRGB8,                                       GL_COLOR_ATTACHMENT0,          24,   1,  1,   1, 0, 1, 1, 1,  1, 0, 0, 0, 0};
            case R8G8B8A8_SRGB              : return {"R8G8B8A8_SRGB",               GL_SRGB8_ALPHA8,                                GL_COLOR_ATTACHMENT0,          32,   1,  1,   1, 0, 1, 1, 1,  1, 0, 0, 0, 0};

          //depth
            case D16_UNORM                  : return {"D16_UNORM",                   GL_DEPTH_COMPONENT16,                           GL_DEPTH_ATTACHMENT,           16,   1,  1,   1, 0, 0, 1, 0,  0, 0, 1, 0, 0};
            case D24_UNORM                  : return {"D24_UNORM",                   GL_DEPTH_COMPONENT24,                           GL_DEPTH_ATTACHMENT,           24,   1,  1,   1, 0, 0, 1, 0,  0, 0, 1, 0, 0};
            case D32_UNORM                  : return {"D32_UNORM",                   GL_DEPTH_COMPONENT32,                           GL_DEPTH_ATTACHMENT,           32,   1,  1,   1, 0, 0, 1, 0,  0, 0, 1, 0, 0};
            case D32_SFLOAT                 : return {"D32_SFLOAT",                  GL_DEPTH_COMPONENT32F,                          GL_DEPTH_ATTACHMENT,           32,   1,  1,   1, 0, 0, 1, 0,  0, 0, 1, 0, 1};

          //depth stencil
          //could we remove this combo formats? NO! Most drivers do not even support a FBO with seperate depth/stencil! :(
          //ARB_ES3_compatibility (Core since 4.3, but only since 2012-09-16; BUG 9418) requires all stencel only formats to be supported
            case D24_UNORM_S8_UINT          : return {"D24_UNORM_S8_UINT",           GL_DEPTH24_STENCIL8,                            GL_DEPTH_STENCIL_ATTACHMENT,   32,   1,  1,   1, 0, 0, 1, 0,  0, 0, 1, 1, 0};
            case D32_SFLOAT_S8_UINT         : return {"D32_SFLOAT_S8_UINT",          GL_DEPTH32F_STENCIL8,                           GL_DEPTH_STENCIL_ATTACHMENT,   64,   1,  1,   1, 0, 0, 1, 0,  0, 0, 1, 1, 0};

          //extension.GL_NV_depth_buffer_float
          //case                            : return {"",                            GL_DEPTH_COMPONENT32F_NV,                       GL_DEPTH_COMPONENT,            32,  1, 0, 1, 0,  0, 0, 1, 0}; //GL_UNSIGNED_INT_24_8_NV
          //case                            : return {"",                            GL_DEPTH32F_STENCIL8_NV,                        GL_DEPTH_STENCIL,              64,  1, 0, 1, 0,  0, 0, 1, 1}; //GL_FLOAT_32_UNSIGNED_INT_24_8_REV_NV, only rev exist

          //stencil index
          //ARB_ES3_compatibility (Core since 4.3, but this formats are only required since 2012-09-16; BUG 9418)
          //PIXELTYPE for GL_STENCIL_INDEXX is guessed so far
          //Even most modern drivers do not support FBOs with seperate depth/stencil format. So this formats are only usefull if the FBO needs no depth values.
          //case S1_UINT                    : return {"S1_UINT",                     GL_STENCIL_INDEX8,                              GL_STENCIL_ATTACHMENT,          1,   1,  1,   1, 0, 0, 0, 0,  0, 0, 0, 1, 0};
          //case S4_UINT                    : return {"S4_UINT",                     GL_STENCIL_INDEX8,                              GL_STENCIL_ATTACHMENT,          4,   1,  1,   1, 0, 0, 0, 0,  0, 0, 0, 1, 0};
            case S8_UINT                    : return {"S8_UINT",                     GL_STENCIL_INDEX8,                              GL_STENCIL_ATTACHMENT,          8,   1,  1,   1, 0, 0, 0, 0,  0, 0, 0, 1, 0};
          //case S16_UINT                   : return {"S16_UINT",                    GL_STENCIL_INDEX16,                             GL_STENCIL_ATTACHMENT,         16,   1,  1,   1, 0, 0, 0, 0,  0, 0, 0, 1, 0};

          //Compressed Formats
          //TODO: Do EAC, ETC2, or RGTC formats a non-zero border value?
          //                                           name                          sizedFormat                                     attachmentType
          //                                           |                             |                                               |                               bitsPerPixel
          //                                           |                             |                                               |                               |    blockSizeX
          //                                           |                             |                                               |                               |    |   blockSizeY
          //                                           |                             |                                               |                               |    |   |    isRenderable
          //                                           |                             |                                               |                               |    |   |    |  isCompressed
          //                                           |                             |                                               |                               |    |   |    |  |  isSrgb
          //                                           |                             |                                               |                               |    |   |    |  |  |  imageSupport
          //                                           |                             |                                               |                               |    |   |    |  |  |  |  sparseSupport
          //                                           |                             |                                               |                               |    |   |    |  |  |  |  |   isRgbaNormalizedIntegerOrFloat
          //                                           |                             |                                               |                               |    |   |    |  |  |  |  |   |  isRgbaInteger
          //                                           |                             |                                               |                               |    |   |    |  |  |  |  |   |  |  isDepth
          //                                           |                             |                                               |                               |    |   |    |  |  |  |  |   |  |  |  isStencil
          //                                           |                             |                                               |                               |    |   |    |  |  |  |  |   |  |  |  |  isSigned
          //                                           |                             |                                               |                               |    |   |    |  |  |  |  |   |  |  |  |  |
          //S3 Texture Compression (S3TC/DXT)
            case BC1_RGB_UNORM_BLOCK        : return {"BC1_RGB_UNORM_BLOCK",         GL_COMPRESSED_RGB_S3TC_DXT1_EXT,                0,                             64,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case BC1_RGBA_UNORM_BLOCK       : return {"BC1_RGBA_UNORM_BLOCK",        GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,               0,                             64,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case BC2_UNORM_BLOCK            : return {"BC2_UNORM_BLOCK",             GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,               0,                            128,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case BC3_UNORM_BLOCK            : return {"BC3_UNORM_BLOCK",             GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,               0,                            128,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
          //GL_EXT_texture_compression_s3tc_srgb (Not core)
            case BC1_RGB_SRGB_BLOCK         : return {"BC1_RGB_SRGB_BLOCK",          GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,               0,                             64,   4,  4,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case BC1_RGBA_SRGB_BLOCK        : return {"BC1_RGBA_SRGB_BLOCK",         GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,         0,                             64,   4,  4,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case BC2_SRGB_BLOCK             : return {"BC2_SRGB_BLOCK",              GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,         0,                            128,   4,  4,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case BC3_SRGB_BLOCK             : return {"BC3_SRGB_BLOCK",              GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,         0,                            128,   4,  4,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};

          //Red Green Texture Compression (RGTC) (compressed like the alpha chanel in S3TC)
          //EXT_texture_compression_rgtc
            case BC4_UNORM_BLOCK            : return {"BC4_UNORM_BLOCK",             GL_COMPRESSED_RED_RGTC1,                        0,                             64,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case BC4_SNORM_BLOCK            : return {"BC4_SNORM_BLOCK",             GL_COMPRESSED_SIGNED_RED_RGTC1,                 0,                             64,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 1};
            case BC5_UNORM_BLOCK            : return {"BC5_UNORM_BLOCK",             GL_COMPRESSED_RG_RGTC2,                         0,                            128,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case BC5_SNORM_BLOCK            : return {"BC5_SNORM_BLOCK",             GL_COMPRESSED_SIGNED_RG_RGTC2,                  0,                            128,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 1};

          //BPTC
            case BC6H_UFLOAT_BLOCK          : return {"BC6H_UFLOAT_BLOCK",           GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,          0,                            128,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case BC6H_SFLOAT_BLOCK          : return {"BC6H_SFLOAT_BLOCK",           GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT,            0,                            128,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 1};
            case BC7_UNORM_BLOCK            : return {"BC7_UNORM_BLOCK",             GL_COMPRESSED_RGBA_BPTC_UNORM,                  0,                            128,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case BC7_SRGB_BLOCK             : return {"BC7_SRGB_BLOCK",              GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,            0,                            128,   4,  4,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};

          //ETC1: GL_COMPRESSED_ETC1_RGB8_OES 64bit block

          //Ericsson Texture Compression (ETC2/EAC)
          //Also core since 4.3 (No extension name)
          //apparently core of opengl, but no real hardware support on desktop GL (remove from list?)
            case ETC2_R8G8B8_UNORM_BLOCK    : return {"ETC2_R8G8B8_UNORM_BLOCK",     GL_COMPRESSED_RGB8_ETC2,                        0,                             64,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ETC2_R8G8B8_SRGB_BLOCK     : return {"ETC2_R8G8B8_SRGB_BLOCK",      GL_COMPRESSED_SRGB8_ETC2,                       0,                             64,   4,  4,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case ETC2_R8G8B8A1_UNORM_BLOCK  : return {"ETC2_R8G8B8A1_UNORM_BLOCK",   GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,    0,                             64,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ETC2_R8G8B8A1_SRGB_BLOCK   : return {"ETC2_R8G8B8A1_SRGB_BLOCK",    GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,   0,                             64,   4,  4,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case ETC2_R8G8B8A8_UNORM_BLOCK  : return {"ETC2_R8G8B8A8_UNORM_BLOCK",   GL_COMPRESSED_RGBA8_ETC2_EAC,                   0,                            128,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ETC2_R8G8B8A8_SRGB_BLOCK   : return {"ETC2_R8G8B8A8_SRGB_BLOCK",    GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,            0,                            128,   4,  4,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case EAC_R11_UNORM_BLOCK        : return {"EAC_R11_UNORM_BLOCK",         GL_COMPRESSED_R11_EAC,                          0,                             64,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case EAC_R11_SNORM_BLOCK        : return {"EAC_R11_SNORM_BLOCK",         GL_COMPRESSED_SIGNED_R11_EAC,                   0,                             64,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 1};
            case EAC_R11G11_UNORM_BLOCK     : return {"EAC_R11G11_UNORM_BLOCK",      GL_COMPRESSED_RG11_EAC,                         0,                            128,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case EAC_R11G11_SNORM_BLOCK     : return {"EAC_R11G11_SNORM_BLOCK",      GL_COMPRESSED_SIGNED_RG11_EAC,                  0,                            128,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 1};

          //Adaptable Scalable Texture Compression (ASTC)
            case ASTC_4x4_UNORM_BLOCK       : return {"ASTC_4x4_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_4x4,                    0,                            128,   4,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_4x4_SRGB_BLOCK        : return {"ASTC_4x4_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4,            0,                            128,   4,  4,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_5x4_UNORM_BLOCK       : return {"ASTC_5x4_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_5x4,                    0,                            128,   5,  4,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_5x4_SRGB_BLOCK        : return {"ASTC_5x4_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4,            0,                            128,   5,  4,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_5x5_UNORM_BLOCK       : return {"ASTC_5x5_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_5x5,                    0,                            128,   5,  5,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_5x5_SRGB_BLOCK        : return {"ASTC_5x5_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5,            0,                            128,   5,  5,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_6x5_UNORM_BLOCK       : return {"ASTC_6x5_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_6x5,                    0,                            128,   6,  5,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_6x5_SRGB_BLOCK        : return {"ASTC_6x5_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5,            0,                            128,   6,  5,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_6x6_UNORM_BLOCK       : return {"ASTC_6x6_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_6x6,                    0,                            128,   6,  6,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_6x6_SRGB_BLOCK        : return {"ASTC_6x6_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6,            0,                            128,   6,  6,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_8x5_UNORM_BLOCK       : return {"ASTC_8x5_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_8x5,                    0,                            128,   8,  5,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_8x5_SRGB_BLOCK        : return {"ASTC_8x5_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5,            0,                            128,   8,  5,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_8x6_UNORM_BLOCK       : return {"ASTC_8x6_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_8x6,                    0,                            128,   8,  6,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_8x6_SRGB_BLOCK        : return {"ASTC_8x6_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6,            0,                            128,   8,  6,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_8x8_UNORM_BLOCK       : return {"ASTC_8x8_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_8x8,                    0,                            128,   8,  8,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_8x8_SRGB_BLOCK        : return {"ASTC_8x8_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8,            0,                            128,   8,  8,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_10x5_UNORM_BLOCK      : return {"ASTC_10x5_UNORM_BLOCK",       GL_COMPRESSED_RGBA_ASTC_10x5,                   0,                            128,  10,  5,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_10x5_SRGB_BLOCK       : return {"ASTC_10x5_SRGB_BLOCK",        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5,           0,                            128,  10,  5,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_10x6_UNORM_BLOCK      : return {"ASTC_10x6_UNORM_BLOCK",       GL_COMPRESSED_RGBA_ASTC_10x6,                   0,                            128,  10,  6,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_10x6_SRGB_BLOCK       : return {"ASTC_10x6_SRGB_BLOCK",        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6,           0,                            128,  10,  6,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_10x8_UNORM_BLOCK      : return {"ASTC_10x8_UNORM_BLOCK",       GL_COMPRESSED_RGBA_ASTC_10x8,                   0,                            128,  10,  8,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_10x8_SRGB_BLOCK       : return {"ASTC_10x8_SRGB_BLOCK",        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8,           0,                            128,  10,  8,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_10x10_UNORM_BLOCK     : return {"ASTC_10x10_UNORM_BLOCK",      GL_COMPRESSED_RGBA_ASTC_10x10,                  0,                            128,  10, 10,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_10x10_SRGB_BLOCK      : return {"ASTC_10x10_SRGB_BLOCK",       GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10,          0,                            128,  10, 10,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_12x10_UNORM_BLOCK     : return {"ASTC_12x10_UNORM_BLOCK",      GL_COMPRESSED_RGBA_ASTC_12x10,                  0,                            128,  12, 10,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_12x10_SRGB_BLOCK      : return {"ASTC_12x10_SRGB_BLOCK",       GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10,          0,                            128,  12, 10,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_12x12_UNORM_BLOCK     : return {"ASTC_12x12_UNORM_BLOCK",      GL_COMPRESSED_RGBA_ASTC_12x12,                  0,                            128,  12, 12,   0, 1, 0, 0, 0,  1, 0, 0, 0, 0};
            case ASTC_12x12_SRGB_BLOCK      : return {"ASTC_12x12_SRGB_BLOCK",       GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12,          0,                            128,  12, 12,   0, 1, 1, 0, 0,  1, 0, 0, 0, 0};
        }
                                              return {"INVALID_SurfaceFormat",       0,                                              0,                              0,   0,  0,   0, 0, 0, 0, 0,  0, 0, 0, 0, 0};
    }

    bool SurfaceFormat::isCopyConvertibleToThisMemorySurfaceFormat(MemorySurfaceFormat memorySurfaceFormat) const {
        //If MemorySurfaceFormat is compressed, SurfaceFormat must be the exact same format!
        if (memorySurfaceFormat.detail().isCompressed && (memorySurfaceFormat.detail().sizedFormat != (*this).detail().sizedFormat)) return false;
        if ((*this).detail().isRgbaInteger                  &&  memorySurfaceFormat.detail().isRgbaInteger)                          return true;
        if ((*this).detail().isRgbaNormalizedIntegerOrFloat &&  memorySurfaceFormat.detail().isRgbaNormalizedIntegerOrFloat)         return true;
        if ((*this).detail().isDepth                        &&  memorySurfaceFormat.detail().isDepth)                                return true;
        if ((*this).detail().isStencil                      &&  memorySurfaceFormat.detail().isStencil)                              return true;
        return false;
    }

    void SurfaceFormat::throwIfNotCopyConvertibleToThisMemorySurfaceFormat(MemorySurfaceFormat memorySurfaceFormat) const {
        if (!isCopyConvertibleToThisMemorySurfaceFormat(memorySurfaceFormat))
            throw runtime_error("Can't copy convert between SurfaceFormat(" + string((*this).detail().name) + " and MemorySurfaceFormat(" + string(memorySurfaceFormat.detail().name) + ")");
    }
}
