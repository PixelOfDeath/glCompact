#include "glCompact/MemorySurfaceFormat.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/MemorySurfaceFormatDetail.hpp"

using namespace glCompact::gl;

namespace glCompact {
    //do we also need GL_SRGB, GL_SRGB_ALPHA for components and arangement?
    const MemorySurfaceFormatDetail memorySurfaceFormatDetailUncompressed[] = {
      // name                           sizedFormat (only relevant for compressed formats)
      // |                              |  componentsAndArrangement
      // |                              |  |                   componentsTypes                     bytePerPixelOrBlock
      // |                              |  |                   |                                   |    blockSizeX
      // |                              |  |                   |                                   |    |   blockSizeY
      // |                              |  |                   |                                   |    |   |    isRgbaNormalizedIntegerOrFloat
      // |                              |  |                   |                                   |    |   |    |  isRgbaInteger
      // |                              |  |                   |                                   |    |   |    |  |  isDepth
      // |                              |  |                   |                                   |    |   |    |  |  |  isStencil
      // |                              |  |                   |                                   |    |   |    |  |  |  |  isCompressed
      // |                              |  |                   |                                   |    |   |    |  |  |  |  |  isSigned
      // |                              |  |                   |                                   |    |   |    |  |  |  |  |  |
        {"R8_UNORM",                    0, GL_RED            , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R8_SNORM",                    0, GL_RED            , GL_BYTE                          ,  1,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R8_UINT",                     0, GL_RED_INTEGER    , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   0, 1, 0, 0, 0, 0},
        {"R8_SINT",                     0, GL_RED_INTEGER    , GL_BYTE                          ,  1,   1,  1,   0, 1, 0, 0, 0, 1},
        {"R16_UNORM",                   0, GL_RED            , GL_UNSIGNED_SHORT                ,  2,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R16_SNORM",                   0, GL_RED            , GL_SHORT                         ,  2,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R16_UINT",                    0, GL_RED_INTEGER    , GL_UNSIGNED_SHORT                ,  2,   1,  1,   0, 1, 0, 0, 0, 0},
        {"R16_SINT",                    0, GL_RED_INTEGER    , GL_SHORT                         ,  2,   1,  1,   0, 1, 0, 0, 0, 1},
        {"R32_UNORM",                   0, GL_RED            , GL_UNSIGNED_INT                  ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R32_SNORM",                   0, GL_RED            , GL_INT                           ,  4,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R32_UINT",                    0, GL_RED_INTEGER    , GL_UNSIGNED_INT                  ,  4,   1,  1,   0, 1, 0, 0, 0, 0},
        {"R32_SINT",                    0, GL_RED_INTEGER    , GL_INT                           ,  4,   1,  1,   0, 1, 0, 0, 0, 1},
        {"R16_SFLOAT",                  0, GL_RED            , GL_HALF_FLOAT                    ,  2,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R32_SFLOAT",                  0, GL_RED            , GL_FLOAT                         ,  4,   1,  1,   1, 0, 0, 0, 0, 1},

        {"G8_UNORM",                    0, GL_GREEN          , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   1, 0, 0, 0, 0, 0},
        {"G8_SNORM",                    0, GL_GREEN          , GL_BYTE                          ,  1,   1,  1,   1, 0, 0, 0, 0, 1},
        {"G8_UINT",                     0, GL_GREEN_INTEGER  , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   0, 1, 0, 0, 0, 0},
        {"G8_SINT",                     0, GL_GREEN_INTEGER  , GL_BYTE                          ,  1,   1,  1,   0, 1, 0, 0, 0, 1},
        {"G16_UNORM",                   0, GL_GREEN          , GL_UNSIGNED_SHORT                ,  2,   1,  1,   1, 0, 0, 0, 0, 0},
        {"G16_SNORM",                   0, GL_GREEN          , GL_SHORT                         ,  2,   1,  1,   1, 0, 0, 0, 0, 1},
        {"G16_UINT",                    0, GL_GREEN_INTEGER  , GL_UNSIGNED_SHORT                ,  2,   1,  1,   0, 1, 0, 0, 0, 0},
        {"G16_SINT",                    0, GL_GREEN_INTEGER  , GL_SHORT                         ,  2,   1,  1,   0, 1, 0, 0, 0, 1},
        {"G32_UNORM",                   0, GL_GREEN          , GL_UNSIGNED_INT                  ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
        {"G32_SNORM",                   0, GL_GREEN          , GL_INT                           ,  4,   1,  1,   1, 0, 0, 0, 0, 1},
        {"G32_UINT",                    0, GL_GREEN_INTEGER  , GL_UNSIGNED_INT                  ,  4,   1,  1,   0, 1, 0, 0, 0, 0},
        {"G32_SINT",                    0, GL_GREEN_INTEGER  , GL_INT                           ,  4,   1,  1,   0, 1, 0, 0, 0, 1},
        {"G16_SFLOAT",                  0, GL_GREEN          , GL_HALF_FLOAT                    ,  2,   1,  1,   1, 0, 0, 0, 0, 1},
        {"G32_SFLOAT",                  0, GL_GREEN          , GL_FLOAT                         ,  4,   1,  1,   1, 0, 0, 0, 0, 1},

        {"B8_UNORM",                    0, GL_BLUE           , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   1, 0, 0, 0, 0, 0},
        {"B8_SNORM",                    0, GL_BLUE           , GL_BYTE                          ,  1,   1,  1,   1, 0, 0, 0, 0, 1},
        {"B8_UINT",                     0, GL_BLUE_INTEGER   , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   0, 1, 0, 0, 0, 0},
        {"B8_SINT",                     0, GL_BLUE_INTEGER   , GL_BYTE                          ,  1,   1,  1,   0, 1, 0, 0, 0, 1},
        {"B16_UNORM",                   0, GL_BLUE           , GL_UNSIGNED_SHORT                ,  2,   1,  1,   1, 0, 0, 0, 0, 0},
        {"B16_SNORM",                   0, GL_BLUE           , GL_SHORT                         ,  2,   1,  1,   1, 0, 0, 0, 0, 1},
        {"B16_UINT",                    0, GL_BLUE_INTEGER   , GL_UNSIGNED_SHORT                ,  2,   1,  1,   0, 1, 0, 0, 0, 0},
        {"B16_SINT",                    0, GL_BLUE_INTEGER   , GL_SHORT                         ,  2,   1,  1,   0, 1, 0, 0, 0, 1},
        {"B32_UNORM",                   0, GL_BLUE           , GL_UNSIGNED_INT                  ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
        {"B32_SNORM",                   0, GL_BLUE           , GL_INT                           ,  4,   1,  1,   1, 0, 0, 0, 0, 1},
        {"B32_UINT",                    0, GL_BLUE_INTEGER   , GL_UNSIGNED_INT                  ,  4,   1,  1,   0, 1, 0, 0, 0, 0},
        {"B32_SINT",                    0, GL_BLUE_INTEGER   , GL_INT                           ,  4,   1,  1,   0, 1, 0, 0, 0, 1},
        {"B16_SFLOAT",                  0, GL_BLUE           , GL_HALF_FLOAT                    ,  2,   1,  1,   1, 0, 0, 0, 0, 1},
        {"B32_SFLOAT",                  0, GL_BLUE           , GL_FLOAT                         ,  4,   1,  1,   1, 0, 0, 0, 0, 1},

        {"A8_UNORM",                    0, GL_ALPHA          , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   1, 0, 0, 0, 0, 0},
        {"A8_SNORM",                    0, GL_ALPHA          , GL_BYTE                          ,  1,   1,  1,   1, 0, 0, 0, 0, 1},
        {"A8_UINT",                     0, GL_ALPHA_INTEGER  , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   0, 1, 0, 0, 0, 0},
        {"A8_SINT",                     0, GL_ALPHA_INTEGER  , GL_BYTE                          ,  1,   1,  1,   0, 1, 0, 0, 0, 1},
        {"A16_UNORM",                   0, GL_ALPHA          , GL_UNSIGNED_SHORT                ,  2,   1,  1,   1, 0, 0, 0, 0, 0},
        {"A16_SNORM",                   0, GL_ALPHA          , GL_SHORT                         ,  2,   1,  1,   1, 0, 0, 0, 0, 1},
        {"A16_UINT",                    0, GL_ALPHA_INTEGER  , GL_UNSIGNED_SHORT                ,  2,   1,  1,   0, 1, 0, 0, 0, 0},
        {"A16_SINT",                    0, GL_ALPHA_INTEGER  , GL_SHORT                         ,  2,   1,  1,   0, 1, 0, 0, 0, 1},
        {"A32_UNORM",                   0, GL_ALPHA          , GL_UNSIGNED_INT                  ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
        {"A32_SNORM",                   0, GL_ALPHA          , GL_INT                           ,  4,   1,  1,   1, 0, 0, 0, 0, 1},
        {"A32_UINT",                    0, GL_ALPHA_INTEGER  , GL_UNSIGNED_INT                  ,  4,   1,  1,   0, 1, 0, 0, 0, 0},
        {"A32_SINT",                    0, GL_ALPHA_INTEGER  , GL_INT                           ,  4,   1,  1,   0, 1, 0, 0, 0, 1},
        {"A16_SFLOAT",                  0, GL_ALPHA          , GL_HALF_FLOAT                    ,  2,   1,  1,   1, 0, 0, 0, 0, 1},
        {"A32_SFLOAT",                  0, GL_ALPHA          , GL_FLOAT                         ,  4,   1,  1,   1, 0, 0, 0, 0, 1},

        {"R8G8_UNORM",                  0, GL_RG             , GL_UNSIGNED_BYTE                 ,  2,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R8G8_SNORM",                  0, GL_RG             , GL_BYTE                          ,  2,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R8G8_UINT",                   0, GL_RG_INTEGER     , GL_UNSIGNED_BYTE                 ,  2,   1,  1,   0, 1, 0, 0, 0, 0},
        {"R8G8_SINT",                   0, GL_RG_INTEGER     , GL_BYTE                          ,  2,   1,  1,   0, 1, 0, 0, 0, 1},
        {"R16G16_UNORM",                0, GL_RG             , GL_UNSIGNED_SHORT                ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R16G16_SNORM",                0, GL_RG             , GL_SHORT                         ,  4,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R16G16_UINT",                 0, GL_RG_INTEGER     , GL_UNSIGNED_SHORT                ,  4,   1,  1,   0, 1, 0, 0, 0, 0},
        {"R16G16_SINT",                 0, GL_RG_INTEGER     , GL_SHORT                         ,  4,   1,  1,   0, 1, 0, 0, 0, 1},
        {"R32G32_UNORM",                0, GL_RG             , GL_UNSIGNED_INT                  ,  8,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R32G32_SNORM",                0, GL_RG             , GL_INT                           ,  8,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R32G32_UINT",                 0, GL_RG_INTEGER     , GL_UNSIGNED_INT                  ,  8,   1,  1,   0, 1, 0, 0, 0, 0},
        {"R32G32_SINT",                 0, GL_RG_INTEGER     , GL_INT                           ,  8,   1,  1,   0, 1, 0, 0, 0, 1},
        {"R16G16_SFLOAT",               0, GL_RG             , GL_HALF_FLOAT                    ,  4,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R32G32_SFLOAT",               0, GL_RG             , GL_FLOAT                         ,  8,   1,  1,   1, 0, 0, 0, 0, 1},

        {"R8G8B8_UNORM",                0, GL_RGB            , GL_UNSIGNED_BYTE                 ,  3,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R8G8B8_SNORM",                0, GL_RGB            , GL_BYTE                          ,  3,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R8G8B8_UINT",                 0, GL_RGB_INTEGER    , GL_UNSIGNED_BYTE                 ,  3,   1,  1,   0, 1, 0, 0, 0, 0},
        {"R8G8B8_SINT",                 0, GL_RGB_INTEGER    , GL_BYTE                          ,  3,   1,  1,   0, 1, 0, 0, 0, 1},
        {"R16G16B16_UNORM",             0, GL_RGB            , GL_UNSIGNED_SHORT                ,  6,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R16G16B16_SNORM",             0, GL_RGB            , GL_SHORT                         ,  6,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R16G16B16_UINT",              0, GL_RGB_INTEGER    , GL_UNSIGNED_SHORT                ,  6,   1,  1,   0, 1, 0, 0, 0, 0},
        {"R16G16B16_SINT",              0, GL_RGB_INTEGER    , GL_SHORT                         ,  6,   1,  1,   0, 1, 0, 0, 0, 1},
        {"R32G32B32_UNORM",             0, GL_RGB            , GL_UNSIGNED_INT                  , 12,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R32G32B32_SNORM",             0, GL_RGB            , GL_INT                           , 12,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R32G32B32_UINT",              0, GL_RGB_INTEGER    , GL_UNSIGNED_INT                  , 12,   1,  1,   0, 1, 0, 0, 0, 0},
        {"R32G32B32_SINT",              0, GL_RGB_INTEGER    , GL_INT                           , 12,   1,  1,   0, 1, 0, 0, 0, 1},
        {"R16G16B16_SFLOAT",            0, GL_RGB            , GL_HALF_FLOAT                    ,  6,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R32G32B32_SFLOAT",            0, GL_RGB            , GL_FLOAT                         , 12,   1,  1,   1, 0, 0, 0, 0, 1},

        {"B8G8R8_UNORM",                0, GL_BGR            , GL_UNSIGNED_BYTE                 ,  3,   1,  1,   1, 0, 0, 0, 0, 0},
        {"B8G8R8_SNORM",                0, GL_BGR            , GL_BYTE                          ,  3,   1,  1,   1, 0, 0, 0, 0, 1},
        {"B8G8R8_UINT",                 0, GL_BGR_INTEGER    , GL_UNSIGNED_BYTE                 ,  3,   1,  1,   0, 1, 0, 0, 0, 0},
        {"B8G8R8_SINT",                 0, GL_BGR_INTEGER    , GL_BYTE                          ,  3,   1,  1,   0, 1, 0, 0, 0, 1},
        {"B16G16R16_UNORM",             0, GL_BGR            , GL_UNSIGNED_SHORT                ,  6,   1,  1,   1, 0, 0, 0, 0, 0},
        {"B16G16R16_SNORM",             0, GL_BGR            , GL_SHORT                         ,  6,   1,  1,   1, 0, 0, 0, 0, 1},
        {"B16G16R16_UINT",              0, GL_BGR_INTEGER    , GL_UNSIGNED_SHORT                ,  6,   1,  1,   0, 1, 0, 0, 0, 0},
        {"B16G16R16_SINT",              0, GL_BGR_INTEGER    , GL_SHORT                         ,  6,   1,  1,   0, 1, 0, 0, 0, 1},
        {"B32G32R32_UNORM",             0, GL_BGR            , GL_UNSIGNED_INT                  , 12,   1,  1,   1, 0, 0, 0, 0, 0},
        {"B32G32R32_SNORM",             0, GL_BGR            , GL_INT                           , 12,   1,  1,   1, 0, 0, 0, 0, 1},
        {"B32G32R32_UINT",              0, GL_BGR_INTEGER    , GL_UNSIGNED_INT                  , 12,   1,  1,   0, 1, 0, 0, 0, 0},
        {"B32G32R32_SINT",              0, GL_BGR_INTEGER    , GL_INT                           , 12,   1,  1,   0, 1, 0, 0, 0, 1},
        {"B16G16R16_SFLOAT",            0, GL_BGR            , GL_HALF_FLOAT                    ,  6,   1,  1,   1, 0, 0, 0, 0, 1},
        {"B32G32R32_SFLOAT",            0, GL_BGR            , GL_FLOAT                         , 12,   1,  1,   1, 0, 0, 0, 0, 1},

        {"R8G8B8A8_UNORM",              0, GL_RGBA           , GL_UNSIGNED_BYTE                 ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R8G8B8A8_SNORM",              0, GL_RGBA           , GL_BYTE                          ,  4,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R8G8B8A8_UINT",               0, GL_RGBA_INTEGER   , GL_UNSIGNED_BYTE                 ,  4,   1,  1,   0, 1, 0, 0, 0, 0},
        {"R8G8B8A8_SINT",               0, GL_RGBA_INTEGER   , GL_BYTE                          ,  4,   1,  1,   0, 1, 0, 0, 0, 1},
        {"R16G16B16A16_UNORM",          0, GL_RGBA           , GL_UNSIGNED_SHORT                ,  8,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R16G16B16A16_SNORM",          0, GL_RGBA           , GL_SHORT                         ,  8,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R16G16B16A16_UINT",           0, GL_RGBA_INTEGER   , GL_UNSIGNED_SHORT                ,  8,   1,  1,   0, 1, 0, 0, 0, 0},
        {"R16G16B16A16_SINT",           0, GL_RGBA_INTEGER   , GL_SHORT                         ,  8,   1,  1,   0, 1, 0, 0, 0, 1},
        {"R32G32B32A32_UNORM",          0, GL_RGBA           , GL_UNSIGNED_INT                  , 16,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R32G32B32A32_SNORM",          0, GL_RGBA           , GL_INT                           , 16,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R32G32B32A32_UINT",           0, GL_RGBA_INTEGER   , GL_UNSIGNED_INT                  , 16,   1,  1,   0, 1, 0, 0, 0, 0},
        {"R32G32B32A32_SINT",           0, GL_RGBA_INTEGER   , GL_INT                           , 16,   1,  1,   0, 1, 0, 0, 0, 1},
        {"R16G16B16A16_SFLOAT",         0, GL_RGBA           , GL_HALF_FLOAT                    ,  8,   1,  1,   1, 0, 0, 0, 0, 1},
        {"R32G32B32A32_SFLOAT",         0, GL_RGBA           , GL_FLOAT                         , 16,   1,  1,   1, 0, 0, 0, 0, 1},

        {"B8G8R8A8_UNORM",              0, GL_BGRA           , GL_UNSIGNED_BYTE                 ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
        {"B8G8R8A8_SNORM",              0, GL_BGRA           , GL_BYTE                          ,  4,   1,  1,   1, 0, 0, 0, 0, 1},
        {"B8G8R8A8_UINT",               0, GL_BGRA_INTEGER   , GL_UNSIGNED_BYTE                 ,  4,   1,  1,   0, 1, 0, 0, 0, 0},
        {"B8G8R8A8_SINT",               0, GL_BGRA_INTEGER   , GL_BYTE                          ,  4,   1,  1,   0, 1, 0, 0, 0, 1},
        {"B16G16R16A16_UNORM",          0, GL_BGRA           , GL_UNSIGNED_SHORT                ,  8,   1,  1,   1, 0, 0, 0, 0, 0},
        {"B16G16R16A16_SNORM",          0, GL_BGRA           , GL_SHORT                         ,  8,   1,  1,   1, 0, 0, 0, 0, 1},
        {"B16G16R16A16_UINT",           0, GL_BGRA_INTEGER   , GL_UNSIGNED_SHORT                ,  8,   1,  1,   0, 1, 0, 0, 0, 0},
        {"B16G16R16A16_SINT",           0, GL_BGRA_INTEGER   , GL_SHORT                         ,  8,   1,  1,   0, 1, 0, 0, 0, 1},
        {"B32G32R32A32_UNORM",          0, GL_BGRA           , GL_UNSIGNED_INT                  , 16,   1,  1,   1, 0, 0, 0, 0, 0},
        {"B32G32R32A32_SNORM",          0, GL_BGRA           , GL_INT                           , 16,   1,  1,   1, 0, 0, 0, 0, 1},
        {"B32G32R32A32_UINT",           0, GL_BGRA_INTEGER   , GL_UNSIGNED_INT                  , 16,   1,  1,   0, 1, 0, 0, 0, 0},
        {"B32G32R32A32_SINT",           0, GL_BGRA_INTEGER   , GL_INT                           , 16,   1,  1,   0, 1, 0, 0, 0, 1},
        {"B16G16R16A16_SFLOAT",         0, GL_BGRA           , GL_HALF_FLOAT                    ,  8,   1,  1,   1, 0, 0, 0, 0, 1},
        {"B32G32R32A32_SFLOAT",         0, GL_BGRA           , GL_FLOAT                         , 16,   1,  1,   1, 0, 0, 0, 0, 1},

        {"R3G3B2_UNORM",                0, GL_RGB            , GL_UNSIGNED_BYTE_3_3_2           ,  1,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R3G3B2_UINT",                 0, GL_RGB_INTEGER    , GL_UNSIGNED_BYTE_3_3_2           ,  1,   1,  1,   0, 1, 0, 0, 0, 0},
        {"B2G3R3_UNORM",                0, GL_RGB            , GL_UNSIGNED_BYTE_2_3_3_REV       ,  1,   1,  1,   1, 0, 0, 0, 0, 0},
        {"B2G3R3_UINT",                 0, GL_RGB_INTEGER    , GL_UNSIGNED_BYTE_2_3_3_REV       ,  1,   1,  1,   0, 1, 0, 0, 0, 0},

        {"R5G6B5_UNORM_PACK16",         0, GL_RGB            , GL_UNSIGNED_SHORT_5_6_5          ,  2,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R5G6B5_UINT_PACK16",          0, GL_RGB_INTEGER    , GL_UNSIGNED_SHORT_5_6_5          ,  2,   1,  1,   0, 1, 0, 0, 0, 0},
        {"B5G6R5_UNORM_PACK16",         0, GL_RGB            , GL_UNSIGNED_SHORT_5_6_5_REV      ,  2,   1,  1,   1, 0, 0, 0, 0, 0},
        {"B5G6R5_UINT_PACK16",          0, GL_RGB_INTEGER    , GL_UNSIGNED_SHORT_5_6_5_REV      ,  2,   1,  1,   0, 1, 0, 0, 0, 0},

        {"R4G4B4A4_UNORM_PACK16",       0, GL_RGBA           , GL_UNSIGNED_SHORT_4_4_4_4        ,  2,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R4G4B4A4_UINT_PACK16",        0, GL_RGBA_INTEGER   , GL_UNSIGNED_SHORT_4_4_4_4        ,  2,   1,  1,   0, 1, 0, 0, 0, 0},
        {"B4G4R4A4_UNORM_PACK16",       0, GL_BGRA           , GL_UNSIGNED_SHORT_4_4_4_4        ,  2,   1,  1,   1, 0, 0, 0, 0, 0},
        {"B4G4R4A4_UINT_PACK16",        0, GL_BGRA_INTEGER   , GL_UNSIGNED_SHORT_4_4_4_4        ,  2,   1,  1,   0, 1, 0, 0, 0, 0},
        {"A4B4G4R4_UNORM_PACK16",       0, GL_RGBA           , GL_UNSIGNED_SHORT_4_4_4_4_REV    ,  2,   1,  1,   1, 0, 0, 0, 0, 0},
        {"A4B4G4R4_UINT_PACK16",        0, GL_RGBA_INTEGER   , GL_UNSIGNED_SHORT_4_4_4_4_REV    ,  2,   1,  1,   0, 1, 0, 0, 0, 0},
        {"A4R4G4B4_UNORM_PACK16",       0, GL_BGRA           , GL_UNSIGNED_SHORT_4_4_4_4_REV    ,  2,   1,  1,   1, 0, 0, 0, 0, 0},
        {"A4R4G4B4_UINT_PACK16",        0, GL_BGRA_INTEGER   , GL_UNSIGNED_SHORT_4_4_4_4_REV    ,  2,   1,  1,   0, 1, 0, 0, 0, 0},

        {"R5G5B5A1_UNORM_PACK16",       0, GL_RGBA           , GL_UNSIGNED_SHORT_5_5_5_1        ,  2,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R5G5B5A1_UINT_PACK16",        0, GL_RGBA_INTEGER   , GL_UNSIGNED_SHORT_5_5_5_1        ,  2,   1,  1,   0, 1, 0, 0, 0, 0},
        {"B5G5R5A1_UNORM_PACK16",       0, GL_BGRA           , GL_UNSIGNED_SHORT_5_5_5_1        ,  2,   1,  1,   1, 0, 0, 0, 0, 0},
        {"B5G5R5A1_UINT_PACK16",        0, GL_BGRA_INTEGER   , GL_UNSIGNED_SHORT_5_5_5_1        ,  2,   1,  1,   0, 1, 0, 0, 0, 0},
        {"A1B5G5R5_UNORM_PACK16",       0, GL_RGBA           , GL_UNSIGNED_SHORT_1_5_5_5_REV    ,  2,   1,  1,   1, 0, 0, 0, 0, 0},
        {"A1B5G5R5_UINT_PACK16",        0, GL_RGBA_INTEGER   , GL_UNSIGNED_SHORT_1_5_5_5_REV    ,  2,   1,  1,   0, 1, 0, 0, 0, 0},
        {"A1R5G5B5_UNORM_PACK16",       0, GL_BGRA           , GL_UNSIGNED_SHORT_1_5_5_5_REV    ,  2,   1,  1,   1, 0, 0, 0, 0, 0},
        {"A1R5G5B5_UINT_PACK16",        0, GL_BGRA_INTEGER   , GL_UNSIGNED_SHORT_1_5_5_5_REV    ,  2,   1,  1,   0, 1, 0, 0, 0, 0},

    //GL_UNSIGNED_INT_8_8_8_8 is already covered by {GL_RGBA, GL_UNSIGNED_BYTE}
      //{"R8G8B8A8_UNORM",              0, GL_RGBA           , GL_UNSIGNED_INT_8_8_8_8          ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
      //{"B8G8R8A8_UNORM",              0, GL_BGRA           , GL_UNSIGNED_INT_8_8_8_8          ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
        {"A8B8G8R8_UNORM_PACK32",       0, GL_RGBA           , GL_UNSIGNED_INT_8_8_8_8_REV      ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
        {"A8B8G8R8_UINT_PACK32",        0, GL_RGBA_INTEGER   , GL_UNSIGNED_INT_8_8_8_8_REV      ,  4,   1,  1,   0, 1, 0, 0, 0, 0},
        {"A8R8G8B8_UNORM_PACK32",       0, GL_BGRA           , GL_UNSIGNED_INT_8_8_8_8_REV      ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
        {"A8R8G8B8_UINT_PACK32",        0, GL_BGRA_INTEGER   , GL_UNSIGNED_INT_8_8_8_8_REV      ,  4,   1,  1,   0, 1, 0, 0, 0, 0},

        {"R10G10B10A2_UNORM_PACK32",    0, GL_RGBA           , GL_UNSIGNED_INT_10_10_10_2       ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
        {"R10G10B10A2_UINT_PACK32",     0, GL_RGBA_INTEGER   , GL_UNSIGNED_INT_10_10_10_2       ,  4,   1,  1,   0, 1, 0, 0, 0, 0},
        {"B10G10R10A2_UNORM_PACK32",    0, GL_BGRA           , GL_UNSIGNED_INT_10_10_10_2       ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
        {"B10G10R10A2_UINT_PACK32",     0, GL_BGRA_INTEGER   , GL_UNSIGNED_INT_10_10_10_2       ,  4,   1,  1,   0, 1, 0, 0, 0, 0},
        {"A2B10G10R10_UNORM_PACK32",    0, GL_RGBA           , GL_UNSIGNED_INT_2_10_10_10_REV   ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
        {"A2B10G10R10_UINT_PACK32",     0, GL_RGBA_INTEGER   , GL_UNSIGNED_INT_2_10_10_10_REV   ,  4,   1,  1,   0, 1, 0, 0, 0, 0},
        {"A2R10G10B10_UNORM_PACK32",    0, GL_BGRA           , GL_UNSIGNED_INT_2_10_10_10_REV   ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
        {"A2R10G10B10_UINT_PACK32",     0, GL_BGRA_INTEGER   , GL_UNSIGNED_INT_2_10_10_10_REV   ,  4,   1,  1,   0, 1, 0, 0, 0, 0},

        //Win7 AMD blob driver accepts this for integer images, but that is not standard conform and e.g. mesa rejects it
        {"B10G11R11_UFLOAT",            0, GL_RGB            , GL_UNSIGNED_INT_10F_11F_11F_REV  ,  4,   1,  1,   1, 0, 0, 0, 0, 0},
        {"E5B9G9R9_UFLOAT",             0, GL_RGB            , GL_UNSIGNED_INT_5_9_9_9_REV      ,  4,   1,  1,   1, 0, 0, 0, 0, 0},

        //depth
        {"D8_UNORM",                    0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE                 ,  1,   1,  1,   0, 0, 1, 0, 0, 0},
        {"D16_UNORM",                   0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT                ,  2,   1,  1,   0, 0, 1, 0, 0, 0},
        {"D32_UNORM",                   0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT                  ,  4,   1,  1,   0, 0, 1, 0, 0, 0},
        {"D32_SFLOAT",                  0, GL_DEPTH_COMPONENT, GL_FLOAT                         ,  4,   1,  1,   0, 0, 1, 0, 0, 1},

        //depth stencil
        {"D24_UNORM_S8_UINT",           0, GL_DEPTH_STENCIL  , GL_UNSIGNED_INT_24_8             ,  4,   1,  1,   0, 0, 1, 1, 0, 0},
        {"D32_SFLOAT_X24_S8_UINT",      0, GL_DEPTH_STENCIL  , GL_FLOAT_32_UNSIGNED_INT_24_8_REV,  8,   1,  1,   0, 0, 1, 1, 0, 1}, //Only the depth value can have a sign, so the isSigned bit references it

        //stencil
        //Vulkan only supports 8 bit stencil formats. For simplicity and bug hit prevention we also only support that one!
      //{"S1_UINT",                     0, GL_STENCIL_INDEX  , GL_STENCIL_INDEX1                },
      //{"S4_UINT",                     0, GL_STENCIL_INDEX  , GL_STENCIL_INDEX4                },
        {"S8_UINT",                     0, GL_STENCIL_INDEX  , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   0, 0, 0, 1, 0, 0},
      //{"S16_UINT",                    0, GL_STENCIL_INDEX  , GL_STENCIL_INDEX16               },

    };

    const MemorySurfaceFormatDetail memorySurfaceFormatDetailCompressed[] = {
      // name                           sizedFormat (To check compatibility with SurfaceFormat)                                                 bytePerPixelOrBlock
      // |                              |                                                                                                       |    blockSizeX
      // |                              |                                                                                                       |    |   blockSizeX
      // |                              |                                                                                                       |    |   |    isRgbaNormalizedIntegerOrFloat
      // |                              |                                                                                                       |    |   |    |  isRgbaInteger
      // |                              |                                                                                                       |    |   |    |  |  isDepth
      // |                              |                                                                                                       |    |   |    |  |  |  isStencil
      // |                              |                                                                                                       |    |   |    |  |  |  |  isCompressed
      // |                              |                                                                                                       |    |   |    |  |  |  |  |  isSigned
      // |                              |                                                                                                       |    |   |    |  |  |  |  |  |
        //S3 Texture Compression (S3TC/DXT)
        {"BC1_RGB_UNORM_BLOCK",         GL_COMPRESSED_RGB_S3TC_DXT1_EXT,                0, 0,                                                   8,   4,  4,   1, 0, 0, 0, 1, 0},
        {"BC1_RGBA_UNORM_BLOCK",        GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,               0, 0,                                                   8,   4,  4,   1, 0, 0, 0, 1, 0},
        {"BC2_UNORM_BLOCK",             GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,               0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 0},
        {"BC3_UNORM_BLOCK",             GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,               0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 0},
        //GL_EXT_texture_compression_s3tc_srgb (Not core)
        {"BC1_RGB_SRGB_BLOCK",          GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,               0, 0,                                                   8,   4,  4,   1, 0, 0, 0, 1, 0},
        {"BC1_RGBA_SRGB_BLOCK",         GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,         0, 0,                                                   8,   4,  4,   1, 0, 0, 0, 1, 0},
        {"BC2_SRGB_BLOCK",              GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,         0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 0},
        {"BC3_SRGB_BLOCK",              GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,         0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 0},

        //Red Green Texture Compression (RGTC) (compressed like the alpha chanel in S3TC)
        //EXT_texture_compression_rgtc
        {"BC4_UNORM_BLOCK",             GL_COMPRESSED_RED_RGTC1,                        0, 0,                                                   8,   4,  4,   1, 0, 0, 0, 1, 0},
        {"BC4_SNORM_BLOCK",             GL_COMPRESSED_SIGNED_RED_RGTC1,                 0, 0,                                                   8,   4,  4,   1, 0, 0, 0, 1, 1},
        {"BC5_UNORM_BLOCK",             GL_COMPRESSED_RG_RGTC2,                         0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 0},
        {"BC5_SNORM_BLOCK",             GL_COMPRESSED_SIGNED_RG_RGTC2,                  0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 1},

        //BPTC
        {"BC6H_UFLOAT_BLOCK",           GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,          0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 0},
        {"BC6H_SFLOAT_BLOCK",           GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT,            0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 1},
        {"BC7_UNORM_BLOCK",             GL_COMPRESSED_RGBA_BPTC_UNORM,                  0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 0},
        {"BC7_SRGB_BLOCK",              GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,            0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 0},

        //Ericsson Texture Compression (ETC2/EAC)
        //Also core since 4.3 (No extension name)
        //apparently core of opengl, but no real hardware support on desktop GL (remove from list?)
        {"ETC2_R8G8B8_UNORM_BLOCK",     GL_COMPRESSED_RGB8_ETC2,                        0, 0,                                                   8,   4,  4,   1, 0, 0, 0, 1, 0},
        {"ETC2_R8G8B8_SRGB_BLOCK",      GL_COMPRESSED_SRGB8_ETC2,                       0, 0,                                                   8,   4,  4,   1, 0, 0, 0, 1, 0},
        {"ETC2_R8G8B8A1_UNORM_BLOCK",   GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,    0, 0,                                                   8,   4,  4,   1, 0, 0, 0, 1, 0},
        {"ETC2_R8G8B8A1_SRGB_BLOCK",    GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,   0, 0,                                                   8,   4,  4,   1, 0, 0, 0, 1, 0},
        {"ETC2_R8G8B8A8_UNORM_BLOCK",   GL_COMPRESSED_RGBA8_ETC2_EAC,                   0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 0},
        {"ETC2_R8G8B8A8_SRGB_BLOCK",    GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,            0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 0},
        {"EAC_R11_UNORM_BLOCK",         GL_COMPRESSED_R11_EAC,                          0, 0,                                                   8,   4,  4,   1, 0, 0, 0, 1, 0},
        {"EAC_R11_SNORM_BLOCK",         GL_COMPRESSED_SIGNED_R11_EAC,                   0, 0,                                                   8,   4,  4,   1, 0, 0, 0, 1, 1},
        {"EAC_R11G11_UNORM_BLOCK",      GL_COMPRESSED_RG11_EAC,                         0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 0},
        {"EAC_R11G11_SNORM_BLOCK",      GL_COMPRESSED_SIGNED_RG11_EAC,                  0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 1},

        //Adaptable Scalable Texture Compression (ASTC)
        {"ASTC_4x4_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_4x4,                    0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 0},
        {"ASTC_4x4_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4,            0, 0,                                                  16,   4,  4,   1, 0, 0, 0, 1, 0},
        {"ASTC_5x4_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_5x4,                    0, 0,                                                  16,   5,  4,   1, 0, 0, 0, 1, 0},
        {"ASTC_5x4_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4,            0, 0,                                                  16,   5,  4,   1, 0, 0, 0, 1, 0},
        {"ASTC_5x5_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_5x5,                    0, 0,                                                  16,   5,  5,   1, 0, 0, 0, 1, 0},
        {"ASTC_5x5_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5,            0, 0,                                                  16,   5,  5,   1, 0, 0, 0, 1, 0},
        {"ASTC_6x5_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_6x5,                    0, 0,                                                  16,   6,  5,   1, 0, 0, 0, 1, 0},
        {"ASTC_6x5_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5,            0, 0,                                                  16,   6,  5,   1, 0, 0, 0, 1, 0},
        {"ASTC_6x6_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_6x6,                    0, 0,                                                  16,   6,  6,   1, 0, 0, 0, 1, 0},
        {"ASTC_6x6_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6,            0, 0,                                                  16,   6,  6,   1, 0, 0, 0, 1, 0},
        {"ASTC_8x5_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_8x5,                    0, 0,                                                  16,   8,  5,   1, 0, 0, 0, 1, 0},
        {"ASTC_8x5_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5,            0, 0,                                                  16,   8,  5,   1, 0, 0, 0, 1, 0},
        {"ASTC_8x6_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_8x6,                    0, 0,                                                  16,   8,  6,   1, 0, 0, 0, 1, 0},
        {"ASTC_8x6_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6,            0, 0,                                                  16,   8,  6,   1, 0, 0, 0, 1, 0},
        {"ASTC_8x8_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_8x8,                    0, 0,                                                  16,   8,  8,   1, 0, 0, 0, 1, 0},
        {"ASTC_8x8_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8,            0, 0,                                                  16,   8,  8,   1, 0, 0, 0, 1, 0},
        {"ASTC_10x5_UNORM_BLOCK",       GL_COMPRESSED_RGBA_ASTC_10x5,                   0, 0,                                                  16,  10,  5,   1, 0, 0, 0, 1, 0},
        {"ASTC_10x5_SRGB_BLOCK",        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5,           0, 0,                                                  16,  10,  5,   1, 0, 0, 0, 1, 0},
        {"ASTC_10x6_UNORM_BLOCK",       GL_COMPRESSED_RGBA_ASTC_10x6,                   0, 0,                                                  16,  10,  6,   1, 0, 0, 0, 1, 0},
        {"ASTC_10x6_SRGB_BLOCK",        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6,           0, 0,                                                  16,  10,  6,   1, 0, 0, 0, 1, 0},
        {"ASTC_10x8_UNORM_BLOCK",       GL_COMPRESSED_RGBA_ASTC_10x8,                   0, 0,                                                  16,  10,  8,   1, 0, 0, 0, 1, 0},
        {"ASTC_10x8_SRGB_BLOCK",        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8,           0, 0,                                                  16,  10,  8,   1, 0, 0, 0, 1, 0},
        {"ASTC_10x10_UNORM_BLOCK",      GL_COMPRESSED_RGBA_ASTC_10x10,                  0, 0,                                                  16,  10, 10,   1, 0, 0, 0, 1, 0},
        {"ASTC_10x10_SRGB_BLOCK",       GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10,          0, 0,                                                  16,  10, 10,   1, 0, 0, 0, 1, 0},
        {"ASTC_12x10_UNORM_BLOCK",      GL_COMPRESSED_RGBA_ASTC_12x10,                  0, 0,                                                  16,  12, 10,   1, 0, 0, 0, 1, 0},
        {"ASTC_12x10_SRGB_BLOCK",       GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10,          0, 0,                                                  16,  12, 10,   1, 0, 0, 0, 1, 0},
        {"ASTC_12x12_UNORM_BLOCK",      GL_COMPRESSED_RGBA_ASTC_12x12,                  0, 0,                                                  16,  12, 12,   1, 0, 0, 0, 1, 0},
        {"ASTC_12x12_SRGB_BLOCK",       GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12,          0, 0,                                                  16,  12, 12,   1, 0, 0, 0, 1, 0},
    };

    const MemorySurfaceFormatDetail* MemorySurfaceFormat::operator->() const {
        if (formatEnum < 1000) {
            return &memorySurfaceFormatDetailUncompressed[formatEnum];
        } else {
            return &memorySurfaceFormatDetailCompressed  [formatEnum - 1000];
        }
    }
}
