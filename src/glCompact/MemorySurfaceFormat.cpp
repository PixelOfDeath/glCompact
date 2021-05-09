#include "glCompact/MemorySurfaceFormat.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/MemorySurfaceFormatDetail.hpp"

using namespace glCompact::gl;

namespace glCompact {
    //do we also need GL_SRGB, GL_SRGB_ALPHA for components and arangement?

    const MemorySurfaceFormatDetail MemorySurfaceFormat::detail() const {
        switch (formatEnum) {
          //                                           name                           sizedFormat (To check compressed format compatibility with SurfaceFormat)
          //                                           |                              |                                               componentsAndArrangement
          //                                           |                              |                                               |                   componentsTypes                     bytePerPixelOrBlock
          //                                           |                              |                                               |                   |                                   |    blockSizeX
          //                                           |                              |                                               |                   |                                   |    |   blockSizeY
          //                                           |                              |                                               |                   |                                   |    |   |    isRgbaNormalizedIntegerOrFloat
          //                                           |                              |                                               |                   |                                   |    |   |    |  isRgbaInteger
          //                                           |                              |                                               |                   |                                   |    |   |    |  |  isDepth
          //                                           |                              |                                               |                   |                                   |    |   |    |  |  |  isStencil
          //                                           |                              |                                               |                   |                                   |    |   |    |  |  |  |  isCompressed
          //                                           |                              |                                               |                   |                                   |    |   |    |  |  |  |  |  isSigned
          //                                           |                              |                                               |                   |                                   |    |   |    |  |  |  |  |  |
            case R8_UNORM                   : return {"R8_UNORM",                    0,                                              GL_RED            , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   1, 0, 0, 0, 0, 0};
            case R8_SNORM                   : return {"R8_SNORM",                    0,                                              GL_RED            , GL_BYTE                          ,  1,   1,  1,   1, 0, 0, 0, 0, 1};
            case R8_UINT                    : return {"R8_UINT",                     0,                                              GL_RED_INTEGER    , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   0, 1, 0, 0, 0, 0};
            case R8_SINT                    : return {"R8_SINT",                     0,                                              GL_RED_INTEGER    , GL_BYTE                          ,  1,   1,  1,   0, 1, 0, 0, 0, 1};
            case R16_UNORM                  : return {"R16_UNORM",                   0,                                              GL_RED            , GL_UNSIGNED_SHORT                ,  2,   1,  1,   1, 0, 0, 0, 0, 0};
            case R16_SNORM                  : return {"R16_SNORM",                   0,                                              GL_RED            , GL_SHORT                         ,  2,   1,  1,   1, 0, 0, 0, 0, 1};
            case R16_UINT                   : return {"R16_UINT",                    0,                                              GL_RED_INTEGER    , GL_UNSIGNED_SHORT                ,  2,   1,  1,   0, 1, 0, 0, 0, 0};
            case R16_SINT                   : return {"R16_SINT",                    0,                                              GL_RED_INTEGER    , GL_SHORT                         ,  2,   1,  1,   0, 1, 0, 0, 0, 1};
            case R32_UNORM                  : return {"R32_UNORM",                   0,                                              GL_RED            , GL_UNSIGNED_INT                  ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
            case R32_SNORM                  : return {"R32_SNORM",                   0,                                              GL_RED            , GL_INT                           ,  4,   1,  1,   1, 0, 0, 0, 0, 1};
            case R32_UINT                   : return {"R32_UINT",                    0,                                              GL_RED_INTEGER    , GL_UNSIGNED_INT                  ,  4,   1,  1,   0, 1, 0, 0, 0, 0};
            case R32_SINT                   : return {"R32_SINT",                    0,                                              GL_RED_INTEGER    , GL_INT                           ,  4,   1,  1,   0, 1, 0, 0, 0, 1};
            case R16_SFLOAT                 : return {"R16_SFLOAT",                  0,                                              GL_RED            , GL_HALF_FLOAT                    ,  2,   1,  1,   1, 0, 0, 0, 0, 1};
            case R32_SFLOAT                 : return {"R32_SFLOAT",                  0,                                              GL_RED            , GL_FLOAT                         ,  4,   1,  1,   1, 0, 0, 0, 0, 1};

            case G8_UNORM                   : return {"G8_UNORM",                    0,                                              GL_GREEN          , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   1, 0, 0, 0, 0, 0};
            case G8_SNORM                   : return {"G8_SNORM",                    0,                                              GL_GREEN          , GL_BYTE                          ,  1,   1,  1,   1, 0, 0, 0, 0, 1};
            case G8_UINT                    : return {"G8_UINT",                     0,                                              GL_GREEN_INTEGER  , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   0, 1, 0, 0, 0, 0};
            case G8_SINT                    : return {"G8_SINT",                     0,                                              GL_GREEN_INTEGER  , GL_BYTE                          ,  1,   1,  1,   0, 1, 0, 0, 0, 1};
            case G16_UNORM                  : return {"G16_UNORM",                   0,                                              GL_GREEN          , GL_UNSIGNED_SHORT                ,  2,   1,  1,   1, 0, 0, 0, 0, 0};
            case G16_SNORM                  : return {"G16_SNORM",                   0,                                              GL_GREEN          , GL_SHORT                         ,  2,   1,  1,   1, 0, 0, 0, 0, 1};
            case G16_UINT                   : return {"G16_UINT",                    0,                                              GL_GREEN_INTEGER  , GL_UNSIGNED_SHORT                ,  2,   1,  1,   0, 1, 0, 0, 0, 0};
            case G16_SINT                   : return {"G16_SINT",                    0,                                              GL_GREEN_INTEGER  , GL_SHORT                         ,  2,   1,  1,   0, 1, 0, 0, 0, 1};
            case G32_UNORM                  : return {"G32_UNORM",                   0,                                              GL_GREEN          , GL_UNSIGNED_INT                  ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
            case G32_SNORM                  : return {"G32_SNORM",                   0,                                              GL_GREEN          , GL_INT                           ,  4,   1,  1,   1, 0, 0, 0, 0, 1};
            case G32_UINT                   : return {"G32_UINT",                    0,                                              GL_GREEN_INTEGER  , GL_UNSIGNED_INT                  ,  4,   1,  1,   0, 1, 0, 0, 0, 0};
            case G32_SINT                   : return {"G32_SINT",                    0,                                              GL_GREEN_INTEGER  , GL_INT                           ,  4,   1,  1,   0, 1, 0, 0, 0, 1};
            case G16_SFLOAT                 : return {"G16_SFLOAT",                  0,                                              GL_GREEN          , GL_HALF_FLOAT                    ,  2,   1,  1,   1, 0, 0, 0, 0, 1};
            case G32_SFLOAT                 : return {"G32_SFLOAT",                  0,                                              GL_GREEN          , GL_FLOAT                         ,  4,   1,  1,   1, 0, 0, 0, 0, 1};

            case B8_UNORM                   : return {"B8_UNORM",                    0,                                              GL_BLUE           , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   1, 0, 0, 0, 0, 0};
            case B8_SNORM                   : return {"B8_SNORM",                    0,                                              GL_BLUE           , GL_BYTE                          ,  1,   1,  1,   1, 0, 0, 0, 0, 1};
            case B8_UINT                    : return {"B8_UINT",                     0,                                              GL_BLUE_INTEGER   , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   0, 1, 0, 0, 0, 0};
            case B8_SINT                    : return {"B8_SINT",                     0,                                              GL_BLUE_INTEGER   , GL_BYTE                          ,  1,   1,  1,   0, 1, 0, 0, 0, 1};
            case B16_UNORM                  : return {"B16_UNORM",                   0,                                              GL_BLUE           , GL_UNSIGNED_SHORT                ,  2,   1,  1,   1, 0, 0, 0, 0, 0};
            case B16_SNORM                  : return {"B16_SNORM",                   0,                                              GL_BLUE           , GL_SHORT                         ,  2,   1,  1,   1, 0, 0, 0, 0, 1};
            case B16_UINT                   : return {"B16_UINT",                    0,                                              GL_BLUE_INTEGER   , GL_UNSIGNED_SHORT                ,  2,   1,  1,   0, 1, 0, 0, 0, 0};
            case B16_SINT                   : return {"B16_SINT",                    0,                                              GL_BLUE_INTEGER   , GL_SHORT                         ,  2,   1,  1,   0, 1, 0, 0, 0, 1};
            case B32_UNORM                  : return {"B32_UNORM",                   0,                                              GL_BLUE           , GL_UNSIGNED_INT                  ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
            case B32_SNORM                  : return {"B32_SNORM",                   0,                                              GL_BLUE           , GL_INT                           ,  4,   1,  1,   1, 0, 0, 0, 0, 1};
            case B32_UINT                   : return {"B32_UINT",                    0,                                              GL_BLUE_INTEGER   , GL_UNSIGNED_INT                  ,  4,   1,  1,   0, 1, 0, 0, 0, 0};
            case B32_SINT                   : return {"B32_SINT",                    0,                                              GL_BLUE_INTEGER   , GL_INT                           ,  4,   1,  1,   0, 1, 0, 0, 0, 1};
            case B16_SFLOAT                 : return {"B16_SFLOAT",                  0,                                              GL_BLUE           , GL_HALF_FLOAT                    ,  2,   1,  1,   1, 0, 0, 0, 0, 1};
            case B32_SFLOAT                 : return {"B32_SFLOAT",                  0,                                              GL_BLUE           , GL_FLOAT                         ,  4,   1,  1,   1, 0, 0, 0, 0, 1};

            case A8_UNORM                   : return {"A8_UNORM",                    0,                                              GL_ALPHA          , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   1, 0, 0, 0, 0, 0};
            case A8_SNORM                   : return {"A8_SNORM",                    0,                                              GL_ALPHA          , GL_BYTE                          ,  1,   1,  1,   1, 0, 0, 0, 0, 1};
            case A8_UINT                    : return {"A8_UINT",                     0,                                              GL_ALPHA_INTEGER  , GL_UNSIGNED_BYTE                 ,  1,   1,  1,   0, 1, 0, 0, 0, 0};
            case A8_SINT                    : return {"A8_SINT",                     0,                                              GL_ALPHA_INTEGER  , GL_BYTE                          ,  1,   1,  1,   0, 1, 0, 0, 0, 1};
            case A16_UNORM                  : return {"A16_UNORM",                   0,                                              GL_ALPHA          , GL_UNSIGNED_SHORT                ,  2,   1,  1,   1, 0, 0, 0, 0, 0};
            case A16_SNORM                  : return {"A16_SNORM",                   0,                                              GL_ALPHA          , GL_SHORT                         ,  2,   1,  1,   1, 0, 0, 0, 0, 1};
            case A16_UINT                   : return {"A16_UINT",                    0,                                              GL_ALPHA_INTEGER  , GL_UNSIGNED_SHORT                ,  2,   1,  1,   0, 1, 0, 0, 0, 0};
            case A16_SINT                   : return {"A16_SINT",                    0,                                              GL_ALPHA_INTEGER  , GL_SHORT                         ,  2,   1,  1,   0, 1, 0, 0, 0, 1};
            case A32_UNORM                  : return {"A32_UNORM",                   0,                                              GL_ALPHA          , GL_UNSIGNED_INT                  ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
            case A32_SNORM                  : return {"A32_SNORM",                   0,                                              GL_ALPHA          , GL_INT                           ,  4,   1,  1,   1, 0, 0, 0, 0, 1};
            case A32_UINT                   : return {"A32_UINT",                    0,                                              GL_ALPHA_INTEGER  , GL_UNSIGNED_INT                  ,  4,   1,  1,   0, 1, 0, 0, 0, 0};
            case A32_SINT                   : return {"A32_SINT",                    0,                                              GL_ALPHA_INTEGER  , GL_INT                           ,  4,   1,  1,   0, 1, 0, 0, 0, 1};
            case A16_SFLOAT                 : return {"A16_SFLOAT",                  0,                                              GL_ALPHA          , GL_HALF_FLOAT                    ,  2,   1,  1,   1, 0, 0, 0, 0, 1};
            case A32_SFLOAT                 : return {"A32_SFLOAT",                  0,                                              GL_ALPHA          , GL_FLOAT                         ,  4,   1,  1,   1, 0, 0, 0, 0, 1};

            case R8G8_UNORM                 : return {"R8G8_UNORM",                  0,                                              GL_RG             , GL_UNSIGNED_BYTE                 ,  2,   1,  1,   1, 0, 0, 0, 0, 0};
            case R8G8_SNORM                 : return {"R8G8_SNORM",                  0,                                              GL_RG             , GL_BYTE                          ,  2,   1,  1,   1, 0, 0, 0, 0, 1};
            case R8G8_UINT                  : return {"R8G8_UINT",                   0,                                              GL_RG_INTEGER     , GL_UNSIGNED_BYTE                 ,  2,   1,  1,   0, 1, 0, 0, 0, 0};
            case R8G8_SINT                  : return {"R8G8_SINT",                   0,                                              GL_RG_INTEGER     , GL_BYTE                          ,  2,   1,  1,   0, 1, 0, 0, 0, 1};
            case R16G16_UNORM               : return {"R16G16_UNORM",                0,                                              GL_RG             , GL_UNSIGNED_SHORT                ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
            case R16G16_SNORM               : return {"R16G16_SNORM",                0,                                              GL_RG             , GL_SHORT                         ,  4,   1,  1,   1, 0, 0, 0, 0, 1};
            case R16G16_UINT                : return {"R16G16_UINT",                 0,                                              GL_RG_INTEGER     , GL_UNSIGNED_SHORT                ,  4,   1,  1,   0, 1, 0, 0, 0, 0};
            case R16G16_SINT                : return {"R16G16_SINT",                 0,                                              GL_RG_INTEGER     , GL_SHORT                         ,  4,   1,  1,   0, 1, 0, 0, 0, 1};
            case R32G32_UNORM               : return {"R32G32_UNORM",                0,                                              GL_RG             , GL_UNSIGNED_INT                  ,  8,   1,  1,   1, 0, 0, 0, 0, 0};
            case R32G32_SNORM               : return {"R32G32_SNORM",                0,                                              GL_RG             , GL_INT                           ,  8,   1,  1,   1, 0, 0, 0, 0, 1};
            case R32G32_UINT                : return {"R32G32_UINT",                 0,                                              GL_RG_INTEGER     , GL_UNSIGNED_INT                  ,  8,   1,  1,   0, 1, 0, 0, 0, 0};
            case R32G32_SINT                : return {"R32G32_SINT",                 0,                                              GL_RG_INTEGER     , GL_INT                           ,  8,   1,  1,   0, 1, 0, 0, 0, 1};
            case R16G16_SFLOAT              : return {"R16G16_SFLOAT",               0,                                              GL_RG             , GL_HALF_FLOAT                    ,  4,   1,  1,   1, 0, 0, 0, 0, 1};
            case R32G32_SFLOAT              : return {"R32G32_SFLOAT",               0,                                              GL_RG             , GL_FLOAT                         ,  8,   1,  1,   1, 0, 0, 0, 0, 1};

            case R8G8B8_UNORM               : return {"R8G8B8_UNORM",                0,                                              GL_RGB            , GL_UNSIGNED_BYTE                 ,  3,   1,  1,   1, 0, 0, 0, 0, 0};
            case R8G8B8_SNORM               : return {"R8G8B8_SNORM",                0,                                              GL_RGB            , GL_BYTE                          ,  3,   1,  1,   1, 0, 0, 0, 0, 1};
            case R8G8B8_UINT                : return {"R8G8B8_UINT",                 0,                                              GL_RGB_INTEGER    , GL_UNSIGNED_BYTE                 ,  3,   1,  1,   0, 1, 0, 0, 0, 0};
            case R8G8B8_SINT                : return {"R8G8B8_SINT",                 0,                                              GL_RGB_INTEGER    , GL_BYTE                          ,  3,   1,  1,   0, 1, 0, 0, 0, 1};
            case R16G16B16_UNORM            : return {"R16G16B16_UNORM",             0,                                              GL_RGB            , GL_UNSIGNED_SHORT                ,  6,   1,  1,   1, 0, 0, 0, 0, 0};
            case R16G16B16_SNORM            : return {"R16G16B16_SNORM",             0,                                              GL_RGB            , GL_SHORT                         ,  6,   1,  1,   1, 0, 0, 0, 0, 1};
            case R16G16B16_UINT             : return {"R16G16B16_UINT",              0,                                              GL_RGB_INTEGER    , GL_UNSIGNED_SHORT                ,  6,   1,  1,   0, 1, 0, 0, 0, 0};
            case R16G16B16_SINT             : return {"R16G16B16_SINT",              0,                                              GL_RGB_INTEGER    , GL_SHORT                         ,  6,   1,  1,   0, 1, 0, 0, 0, 1};
            case R32G32B32_UNORM            : return {"R32G32B32_UNORM",             0,                                              GL_RGB            , GL_UNSIGNED_INT                  , 12,   1,  1,   1, 0, 0, 0, 0, 0};
            case R32G32B32_SNORM            : return {"R32G32B32_SNORM",             0,                                              GL_RGB            , GL_INT                           , 12,   1,  1,   1, 0, 0, 0, 0, 1};
            case R32G32B32_UINT             : return {"R32G32B32_UINT",              0,                                              GL_RGB_INTEGER    , GL_UNSIGNED_INT                  , 12,   1,  1,   0, 1, 0, 0, 0, 0};
            case R32G32B32_SINT             : return {"R32G32B32_SINT",              0,                                              GL_RGB_INTEGER    , GL_INT                           , 12,   1,  1,   0, 1, 0, 0, 0, 1};
            case R16G16B16_SFLOAT           : return {"R16G16B16_SFLOAT",            0,                                              GL_RGB            , GL_HALF_FLOAT                    ,  6,   1,  1,   1, 0, 0, 0, 0, 1};
            case R32G32B32_SFLOAT           : return {"R32G32B32_SFLOAT",            0,                                              GL_RGB            , GL_FLOAT                         , 12,   1,  1,   1, 0, 0, 0, 0, 1};

            case B8G8R8_UNORM               : return {"B8G8R8_UNORM",                0,                                              GL_BGR            , GL_UNSIGNED_BYTE                 ,  3,   1,  1,   1, 0, 0, 0, 0, 0};
            case B8G8R8_SNORM               : return {"B8G8R8_SNORM",                0,                                              GL_BGR            , GL_BYTE                          ,  3,   1,  1,   1, 0, 0, 0, 0, 1};
            case B8G8R8_UINT                : return {"B8G8R8_UINT",                 0,                                              GL_BGR_INTEGER    , GL_UNSIGNED_BYTE                 ,  3,   1,  1,   0, 1, 0, 0, 0, 0};
            case B8G8R8_SINT                : return {"B8G8R8_SINT",                 0,                                              GL_BGR_INTEGER    , GL_BYTE                          ,  3,   1,  1,   0, 1, 0, 0, 0, 1};
            case B16G16R16_UNORM            : return {"B16G16R16_UNORM",             0,                                              GL_BGR            , GL_UNSIGNED_SHORT                ,  6,   1,  1,   1, 0, 0, 0, 0, 0};
            case B16G16R16_SNORM            : return {"B16G16R16_SNORM",             0,                                              GL_BGR            , GL_SHORT                         ,  6,   1,  1,   1, 0, 0, 0, 0, 1};
            case B16G16R16_UINT             : return {"B16G16R16_UINT",              0,                                              GL_BGR_INTEGER    , GL_UNSIGNED_SHORT                ,  6,   1,  1,   0, 1, 0, 0, 0, 0};
            case B16G16R16_SINT             : return {"B16G16R16_SINT",              0,                                              GL_BGR_INTEGER    , GL_SHORT                         ,  6,   1,  1,   0, 1, 0, 0, 0, 1};
            case B32G32R32_UNORM            : return {"B32G32R32_UNORM",             0,                                              GL_BGR            , GL_UNSIGNED_INT                  , 12,   1,  1,   1, 0, 0, 0, 0, 0};
            case B32G32R32_SNORM            : return {"B32G32R32_SNORM",             0,                                              GL_BGR            , GL_INT                           , 12,   1,  1,   1, 0, 0, 0, 0, 1};
            case B32G32R32_UINT             : return {"B32G32R32_UINT",              0,                                              GL_BGR_INTEGER    , GL_UNSIGNED_INT                  , 12,   1,  1,   0, 1, 0, 0, 0, 0};
            case B32G32R32_SINT             : return {"B32G32R32_SINT",              0,                                              GL_BGR_INTEGER    , GL_INT                           , 12,   1,  1,   0, 1, 0, 0, 0, 1};
            case B16G16R16_SFLOAT           : return {"B16G16R16_SFLOAT",            0,                                              GL_BGR            , GL_HALF_FLOAT                    ,  6,   1,  1,   1, 0, 0, 0, 0, 1};
            case B32G32R32_SFLOAT           : return {"B32G32R32_SFLOAT",            0,                                              GL_BGR            , GL_FLOAT                         , 12,   1,  1,   1, 0, 0, 0, 0, 1};

            case R8G8B8A8_UNORM             : return {"R8G8B8A8_UNORM",              0,                                              GL_RGBA           , GL_UNSIGNED_BYTE                 ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
            case R8G8B8A8_SNORM             : return {"R8G8B8A8_SNORM",              0,                                              GL_RGBA           , GL_BYTE                          ,  4,   1,  1,   1, 0, 0, 0, 0, 1};
            case R8G8B8A8_UINT              : return {"R8G8B8A8_UINT",               0,                                              GL_RGBA_INTEGER   , GL_UNSIGNED_BYTE                 ,  4,   1,  1,   0, 1, 0, 0, 0, 0};
            case R8G8B8A8_SINT              : return {"R8G8B8A8_SINT",               0,                                              GL_RGBA_INTEGER   , GL_BYTE                          ,  4,   1,  1,   0, 1, 0, 0, 0, 1};
            case R16G16B16A16_UNORM         : return {"R16G16B16A16_UNORM",          0,                                              GL_RGBA           , GL_UNSIGNED_SHORT                ,  8,   1,  1,   1, 0, 0, 0, 0, 0};
            case R16G16B16A16_SNORM         : return {"R16G16B16A16_SNORM",          0,                                              GL_RGBA           , GL_SHORT                         ,  8,   1,  1,   1, 0, 0, 0, 0, 1};
            case R16G16B16A16_UINT          : return {"R16G16B16A16_UINT",           0,                                              GL_RGBA_INTEGER   , GL_UNSIGNED_SHORT                ,  8,   1,  1,   0, 1, 0, 0, 0, 0};
            case R16G16B16A16_SINT          : return {"R16G16B16A16_SINT",           0,                                              GL_RGBA_INTEGER   , GL_SHORT                         ,  8,   1,  1,   0, 1, 0, 0, 0, 1};
            case R32G32B32A32_UNORM         : return {"R32G32B32A32_UNORM",          0,                                              GL_RGBA           , GL_UNSIGNED_INT                  , 16,   1,  1,   1, 0, 0, 0, 0, 0};
            case R32G32B32A32_SNORM         : return {"R32G32B32A32_SNORM",          0,                                              GL_RGBA           , GL_INT                           , 16,   1,  1,   1, 0, 0, 0, 0, 1};
            case R32G32B32A32_UINT          : return {"R32G32B32A32_UINT",           0,                                              GL_RGBA_INTEGER   , GL_UNSIGNED_INT                  , 16,   1,  1,   0, 1, 0, 0, 0, 0};
            case R32G32B32A32_SINT          : return {"R32G32B32A32_SINT",           0,                                              GL_RGBA_INTEGER   , GL_INT                           , 16,   1,  1,   0, 1, 0, 0, 0, 1};
            case R16G16B16A16_SFLOAT        : return {"R16G16B16A16_SFLOAT",         0,                                              GL_RGBA           , GL_HALF_FLOAT                    ,  8,   1,  1,   1, 0, 0, 0, 0, 1};
            case R32G32B32A32_SFLOAT        : return {"R32G32B32A32_SFLOAT",         0,                                              GL_RGBA           , GL_FLOAT                         , 16,   1,  1,   1, 0, 0, 0, 0, 1};

            case B8G8R8A8_UNORM             : return {"B8G8R8A8_UNORM",              0,                                              GL_BGRA           , GL_UNSIGNED_BYTE                 ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
            case B8G8R8A8_SNORM             : return {"B8G8R8A8_SNORM",              0,                                              GL_BGRA           , GL_BYTE                          ,  4,   1,  1,   1, 0, 0, 0, 0, 1};
            case B8G8R8A8_UINT              : return {"B8G8R8A8_UINT",               0,                                              GL_BGRA_INTEGER   , GL_UNSIGNED_BYTE                 ,  4,   1,  1,   0, 1, 0, 0, 0, 0};
            case B8G8R8A8_SINT              : return {"B8G8R8A8_SINT",               0,                                              GL_BGRA_INTEGER   , GL_BYTE                          ,  4,   1,  1,   0, 1, 0, 0, 0, 1};
            case B16G16R16A16_UNORM         : return {"B16G16R16A16_UNORM",          0,                                              GL_BGRA           , GL_UNSIGNED_SHORT                ,  8,   1,  1,   1, 0, 0, 0, 0, 0};
            case B16G16R16A16_SNORM         : return {"B16G16R16A16_SNORM",          0,                                              GL_BGRA           , GL_SHORT                         ,  8,   1,  1,   1, 0, 0, 0, 0, 1};
            case B16G16R16A16_UINT          : return {"B16G16R16A16_UINT",           0,                                              GL_BGRA_INTEGER   , GL_UNSIGNED_SHORT                ,  8,   1,  1,   0, 1, 0, 0, 0, 0};
            case B16G16R16A16_SINT          : return {"B16G16R16A16_SINT",           0,                                              GL_BGRA_INTEGER   , GL_SHORT                         ,  8,   1,  1,   0, 1, 0, 0, 0, 1};
            case B32G32R32A32_UNORM         : return {"B32G32R32A32_UNORM",          0,                                              GL_BGRA           , GL_UNSIGNED_INT                  , 16,   1,  1,   1, 0, 0, 0, 0, 0};
            case B32G32R32A32_SNORM         : return {"B32G32R32A32_SNORM",          0,                                              GL_BGRA           , GL_INT                           , 16,   1,  1,   1, 0, 0, 0, 0, 1};
            case B32G32R32A32_UINT          : return {"B32G32R32A32_UINT",           0,                                              GL_BGRA_INTEGER   , GL_UNSIGNED_INT                  , 16,   1,  1,   0, 1, 0, 0, 0, 0};
            case B32G32R32A32_SINT          : return {"B32G32R32A32_SINT",           0,                                              GL_BGRA_INTEGER   , GL_INT                           , 16,   1,  1,   0, 1, 0, 0, 0, 1};
            case B16G16R16A16_SFLOAT        : return {"B16G16R16A16_SFLOAT",         0,                                              GL_BGRA           , GL_HALF_FLOAT                    ,  8,   1,  1,   1, 0, 0, 0, 0, 1};
            case B32G32R32A32_SFLOAT        : return {"B32G32R32A32_SFLOAT",         0,                                              GL_BGRA           , GL_FLOAT                         , 16,   1,  1,   1, 0, 0, 0, 0, 1};

            case R3G3B2_UNORM_PACK8         : return {"R3G3B2_UNORM_PACK8",          0,                                              GL_RGB            , GL_UNSIGNED_BYTE_3_3_2           ,  1,   1,  1,   1, 0, 0, 0, 0, 0};
            case R3G3B2_UINT_PACK8          : return {"R3G3B2_UINT_PACK8",           0,                                              GL_RGB_INTEGER    , GL_UNSIGNED_BYTE_3_3_2           ,  1,   1,  1,   0, 1, 0, 0, 0, 0};
            case B2G3R3_UNORM_PACK8         : return {"B2G3R3_UNORM_PACK8",          0,                                              GL_RGB            , GL_UNSIGNED_BYTE_2_3_3_REV       ,  1,   1,  1,   1, 0, 0, 0, 0, 0};
            case B2G3R3_UINT_PACK8          : return {"B2G3R3_UINT_PACK8",           0,                                              GL_RGB_INTEGER    , GL_UNSIGNED_BYTE_2_3_3_REV       ,  1,   1,  1,   0, 1, 0, 0, 0, 0};

            case R5G6B5_UNORM_PACK16        : return {"R5G6B5_UNORM_PACK16",         0,                                              GL_RGB            , GL_UNSIGNED_SHORT_5_6_5          ,  2,   1,  1,   1, 0, 0, 0, 0, 0};
            case R5G6B5_UINT_PACK16         : return {"R5G6B5_UINT_PACK16",          0,                                              GL_RGB_INTEGER    , GL_UNSIGNED_SHORT_5_6_5          ,  2,   1,  1,   0, 1, 0, 0, 0, 0};
            case B5G6R5_UNORM_PACK16        : return {"B5G6R5_UNORM_PACK16",         0,                                              GL_RGB            , GL_UNSIGNED_SHORT_5_6_5_REV      ,  2,   1,  1,   1, 0, 0, 0, 0, 0};
            case B5G6R5_UINT_PACK16         : return {"B5G6R5_UINT_PACK16",          0,                                              GL_RGB_INTEGER    , GL_UNSIGNED_SHORT_5_6_5_REV      ,  2,   1,  1,   0, 1, 0, 0, 0, 0};

            case R4G4B4A4_UNORM_PACK16      : return {"R4G4B4A4_UNORM_PACK16",       0,                                              GL_RGBA           , GL_UNSIGNED_SHORT_4_4_4_4        ,  2,   1,  1,   1, 0, 0, 0, 0, 0};
            case R4G4B4A4_UINT_PACK16       : return {"R4G4B4A4_UINT_PACK16",        0,                                              GL_RGBA_INTEGER   , GL_UNSIGNED_SHORT_4_4_4_4        ,  2,   1,  1,   0, 1, 0, 0, 0, 0};
            case B4G4R4A4_UNORM_PACK16      : return {"B4G4R4A4_UNORM_PACK16",       0,                                              GL_BGRA           , GL_UNSIGNED_SHORT_4_4_4_4        ,  2,   1,  1,   1, 0, 0, 0, 0, 0};
            case B4G4R4A4_UINT_PACK16       : return {"B4G4R4A4_UINT_PACK16",        0,                                              GL_BGRA_INTEGER   , GL_UNSIGNED_SHORT_4_4_4_4        ,  2,   1,  1,   0, 1, 0, 0, 0, 0};
            case A4B4G4R4_UNORM_PACK16      : return {"A4B4G4R4_UNORM_PACK16",       0,                                              GL_RGBA           , GL_UNSIGNED_SHORT_4_4_4_4_REV    ,  2,   1,  1,   1, 0, 0, 0, 0, 0};
            case A4B4G4R4_UINT_PACK16       : return {"A4B4G4R4_UINT_PACK16",        0,                                              GL_RGBA_INTEGER   , GL_UNSIGNED_SHORT_4_4_4_4_REV    ,  2,   1,  1,   0, 1, 0, 0, 0, 0};
            case A4R4G4B4_UNORM_PACK16      : return {"A4R4G4B4_UNORM_PACK16",       0,                                              GL_BGRA           , GL_UNSIGNED_SHORT_4_4_4_4_REV    ,  2,   1,  1,   1, 0, 0, 0, 0, 0};
            case A4R4G4B4_UINT_PACK16       : return {"A4R4G4B4_UINT_PACK16",        0,                                              GL_BGRA_INTEGER   , GL_UNSIGNED_SHORT_4_4_4_4_REV    ,  2,   1,  1,   0, 1, 0, 0, 0, 0};

            case R5G5B5A1_UNORM_PACK16      : return {"R5G5B5A1_UNORM_PACK16",       0,                                              GL_RGBA           , GL_UNSIGNED_SHORT_5_5_5_1        ,  2,   1,  1,   1, 0, 0, 0, 0, 0};
            case R5G5B5A1_UINT_PACK16       : return {"R5G5B5A1_UINT_PACK16",        0,                                              GL_RGBA_INTEGER   , GL_UNSIGNED_SHORT_5_5_5_1        ,  2,   1,  1,   0, 1, 0, 0, 0, 0};
            case B5G5R5A1_UNORM_PACK16      : return {"B5G5R5A1_UNORM_PACK16",       0,                                              GL_BGRA           , GL_UNSIGNED_SHORT_5_5_5_1        ,  2,   1,  1,   1, 0, 0, 0, 0, 0};
            case B5G5R5A1_UINT_PACK16       : return {"B5G5R5A1_UINT_PACK16",        0,                                              GL_BGRA_INTEGER   , GL_UNSIGNED_SHORT_5_5_5_1        ,  2,   1,  1,   0, 1, 0, 0, 0, 0};
            case A1B5G5R5_UNORM_PACK16      : return {"A1B5G5R5_UNORM_PACK16",       0,                                              GL_RGBA           , GL_UNSIGNED_SHORT_1_5_5_5_REV    ,  2,   1,  1,   1, 0, 0, 0, 0, 0};
            case A1B5G5R5_UINT_PACK16       : return {"A1B5G5R5_UINT_PACK16",        0,                                              GL_RGBA_INTEGER   , GL_UNSIGNED_SHORT_1_5_5_5_REV    ,  2,   1,  1,   0, 1, 0, 0, 0, 0};
            case A1R5G5B5_UNORM_PACK16      : return {"A1R5G5B5_UNORM_PACK16",       0,                                              GL_BGRA           , GL_UNSIGNED_SHORT_1_5_5_5_REV    ,  2,   1,  1,   1, 0, 0, 0, 0, 0};
            case A1R5G5B5_UINT_PACK16       : return {"A1R5G5B5_UINT_PACK16",        0,                                              GL_BGRA_INTEGER   , GL_UNSIGNED_SHORT_1_5_5_5_REV    ,  2,   1,  1,   0, 1, 0, 0, 0, 0};

          //GL_UNSIGNED_INT_8_8_8_8 is already covered by {GL_RGBA, GL_UNSIGNED_BYTE}
          //case R8G8B8A8_UNORM             : return {"R8G8B8A8_UNORM",              0,                                              GL_RGBA           , GL_UNSIGNED_INT_8_8_8_8          ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
          //case B8G8R8A8_UNORM             : return {"B8G8R8A8_UNORM",              0,                                              GL_BGRA           , GL_UNSIGNED_INT_8_8_8_8          ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
            case A8B8G8R8_UNORM_PACK32      : return {"A8B8G8R8_UNORM_PACK32",       0,                                              GL_RGBA           , GL_UNSIGNED_INT_8_8_8_8_REV      ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
            case A8B8G8R8_UINT_PACK32       : return {"A8B8G8R8_UINT_PACK32",        0,                                              GL_RGBA_INTEGER   , GL_UNSIGNED_INT_8_8_8_8_REV      ,  4,   1,  1,   0, 1, 0, 0, 0, 0};
            case A8R8G8B8_UNORM_PACK32      : return {"A8R8G8B8_UNORM_PACK32",       0,                                              GL_BGRA           , GL_UNSIGNED_INT_8_8_8_8_REV      ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
            case A8R8G8B8_UINT_PACK32       : return {"A8R8G8B8_UINT_PACK32",        0,                                              GL_BGRA_INTEGER   , GL_UNSIGNED_INT_8_8_8_8_REV      ,  4,   1,  1,   0, 1, 0, 0, 0, 0};

            case R10G10B10A2_UNORM_PACK32   : return {"R10G10B10A2_UNORM_PACK32",    0,                                              GL_RGBA           , GL_UNSIGNED_INT_10_10_10_2       ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
            case R10G10B10A2_UINT_PACK32    : return {"R10G10B10A2_UINT_PACK32",     0,                                              GL_RGBA_INTEGER   , GL_UNSIGNED_INT_10_10_10_2       ,  4,   1,  1,   0, 1, 0, 0, 0, 0};
            case B10G10R10A2_UNORM_PACK32   : return {"B10G10R10A2_UNORM_PACK32",    0,                                              GL_BGRA           , GL_UNSIGNED_INT_10_10_10_2       ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
            case B10G10R10A2_UINT_PACK32    : return {"B10G10R10A2_UINT_PACK32",     0,                                              GL_BGRA_INTEGER   , GL_UNSIGNED_INT_10_10_10_2       ,  4,   1,  1,   0, 1, 0, 0, 0, 0};
            case A2B10G10R10_UNORM_PACK32   : return {"A2B10G10R10_UNORM_PACK32",    0,                                              GL_RGBA           , GL_UNSIGNED_INT_2_10_10_10_REV   ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
            case A2B10G10R10_UINT_PACK32    : return {"A2B10G10R10_UINT_PACK32",     0,                                              GL_RGBA_INTEGER   , GL_UNSIGNED_INT_2_10_10_10_REV   ,  4,   1,  1,   0, 1, 0, 0, 0, 0};
            case A2R10G10B10_UNORM_PACK32   : return {"A2R10G10B10_UNORM_PACK32",    0,                                              GL_BGRA           , GL_UNSIGNED_INT_2_10_10_10_REV   ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
            case A2R10G10B10_UINT_PACK32    : return {"A2R10G10B10_UINT_PACK32",     0,                                              GL_BGRA_INTEGER   , GL_UNSIGNED_INT_2_10_10_10_REV   ,  4,   1,  1,   0, 1, 0, 0, 0, 0};

            //Win7 AMD blob driver accepts this for integer images, but that is not standard conform and e.g. mesa rejects it
            case B10G11R11_UFLOAT_PACK32    : return {"B10G11R11_UFLOAT_PACK32",     0,                                              GL_RGB            , GL_UNSIGNED_INT_10F_11F_11F_REV  ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
          //case R11G11B10_UFLOAT_PACK32    : return {"R11G11B10_UFLOAT_PACK32",     0,                                              GL_BGR            , GL_UNSIGNED_INT_10F_11F_11F_REV  ,  4,   1,  1,   1, 0, 0, 0, 0, 0}; ?
            case E5B9G9R9_UFLOAT_PACK32     : return {"E5B9G9R9_UFLOAT_PACK32",      0,                                              GL_RGB            , GL_UNSIGNED_INT_5_9_9_9_REV      ,  4,   1,  1,   1, 0, 0, 0, 0, 0};
          //case E5R9G9B9_UFLOAT_PACK32     : return {"E5R9G9B9_UFLOAT_PACK32",      0,                                              GL_BGR            , GL_UNSIGNED_INT_5_9_9_9_REV      ,  4,   1,  1,   1, 0, 0, 0, 0, 0}; ?

            //depth
            case D8_UNORM                   : return {"D8_UNORM",                    0,                                              GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE                 ,  1,   1,  1,   0, 0, 1, 0, 0, 0};
            case D16_UNORM                  : return {"D16_UNORM",                   0,                                              GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT                ,  2,   1,  1,   0, 0, 1, 0, 0, 0};
            case D32_UNORM                  : return {"D32_UNORM",                   0,                                              GL_DEPTH_COMPONENT, GL_UNSIGNED_INT                  ,  4,   1,  1,   0, 0, 1, 0, 0, 0};
            case D32_SFLOAT                 : return {"D32_SFLOAT",                  0,                                              GL_DEPTH_COMPONENT, GL_FLOAT                         ,  4,   1,  1,   0, 0, 1, 0, 0, 1};

            //depth stencil
            case D24_UNORM_S8_UINT          : return {"D24_UNORM_S8_UINT",           0,                                              GL_DEPTH_STENCIL  , GL_UNSIGNED_INT_24_8             ,  4,   1,  1,   0, 0, 1, 1, 0, 0};
            case D32_SFLOAT_X24_S8_UINT     : return {"D32_SFLOAT_X24_S8_UINT",      0,                                              GL_DEPTH_STENCIL  , GL_FLOAT_32_UNSIGNED_INT_24_8_REV,  8,   1,  1,   0, 0, 1, 1, 0, 1}; //Only the depth value can have a sign, so the isSigned bit references it

            //stencil
            //Vulkan only supports 8 bit stencil formats. For simplicity and bug hit prevention we also only support that one!
            case S8_UINT                    : return {"S8_UINT",                     0,                                              GL_STENCIL_INDEX,   GL_UNSIGNED_BYTE,                   1,   1,  1,   0, 0, 0, 1, 0, 0};
          //case S16_UINT                   : return {"S16_UINT",                    0,                                              GL_STENCIL_INDEX,   GL_UNSIGNED_SHORT,                  2,   1,  1,   0, 0, 0, 1, 0, 0};




          //                                           name                          sizedFormat (To check compressed format compatibility with SurfaceFormat)                               bytePerPixelOrBlock
          //                                           |                             |                                                                                                       |    blockSizeX
          //                                           |                             |                                                                                                       |    |   blockSizeX
          //                                           |                             |                                                                                                       |    |   |    isRgbaNormalizedIntegerOrFloat
          //                                           |                             |                                                                                                       |    |   |    |  isRgbaInteger
          //                                           |                             |                                                                                                       |    |   |    |  |  isDepth
          //                                           |                             |                                                                                                       |    |   |    |  |  |  isStencil
          //                                           |                             |                                                                                                       |    |   |    |  |  |  |  isCompressed
          //                                           |                             |                                                                                                       |    |   |    |  |  |  |  |  isSigned
          //                                           |                             |                                                                                                       |    |   |    |  |  |  |  |  |
            //S3 Texture Compression (S3TC/DXT)
            case BC1_RGB_UNORM_BLOCK        : return {"BC1_RGB_UNORM_BLOCK",         GL_COMPRESSED_RGB_S3TC_DXT1_EXT,                0,                  0,                                  8,   4,  4,   1, 0, 0, 0, 1, 0};
            case BC1_RGBA_UNORM_BLOCK       : return {"BC1_RGBA_UNORM_BLOCK",        GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,               0,                  0,                                  8,   4,  4,   1, 0, 0, 0, 1, 0};
            case BC2_UNORM_BLOCK            : return {"BC2_UNORM_BLOCK",             GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,               0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 0};
            case BC3_UNORM_BLOCK            : return {"BC3_UNORM_BLOCK",             GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,               0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 0};
            //GL_EXT_texture_compression_s3tc_srgb (Not core)
            case BC1_RGB_SRGB_BLOCK         : return {"BC1_RGB_SRGB_BLOCK",          GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,               0,                  0,                                  8,   4,  4,   1, 0, 0, 0, 1, 0};
            case BC1_RGBA_SRGB_BLOCK        : return {"BC1_RGBA_SRGB_BLOCK",         GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,         0,                  0,                                  8,   4,  4,   1, 0, 0, 0, 1, 0};
            case BC2_SRGB_BLOCK             : return {"BC2_SRGB_BLOCK",              GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,         0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 0};
            case BC3_SRGB_BLOCK             : return {"BC3_SRGB_BLOCK",              GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,         0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 0};

            //Red Green Texture Compression (RGTC) (compressed like the alpha chanel in S3TC)
            //EXT_texture_compression_rgtc
            case BC4_UNORM_BLOCK            : return {"BC4_UNORM_BLOCK",             GL_COMPRESSED_RED_RGTC1,                        0,                  0,                                  8,   4,  4,   1, 0, 0, 0, 1, 0};
            case BC4_SNORM_BLOCK            : return {"BC4_SNORM_BLOCK",             GL_COMPRESSED_SIGNED_RED_RGTC1,                 0,                  0,                                  8,   4,  4,   1, 0, 0, 0, 1, 1};
            case BC5_UNORM_BLOCK            : return {"BC5_UNORM_BLOCK",             GL_COMPRESSED_RG_RGTC2,                         0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 0};
            case BC5_SNORM_BLOCK            : return {"BC5_SNORM_BLOCK",             GL_COMPRESSED_SIGNED_RG_RGTC2,                  0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 1};

            //BPTC
            case BC6H_UFLOAT_BLOCK          : return {"BC6H_UFLOAT_BLOCK",           GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,          0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 0};
            case BC6H_SFLOAT_BLOCK          : return {"BC6H_SFLOAT_BLOCK",           GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT,            0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 1};
            case BC7_UNORM_BLOCK            : return {"BC7_UNORM_BLOCK",             GL_COMPRESSED_RGBA_BPTC_UNORM,                  0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 0};
            case BC7_SRGB_BLOCK             : return {"BC7_SRGB_BLOCK",              GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,            0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 0};

            //Ericsson Texture Compression (ETC2/EAC)
            //Also core since 4.3 (No extension name)
            //apparently core of opengl, but no real hardware support on desktop GL (remove from list?)
            case ETC2_R8G8B8_UNORM_BLOCK    : return {"ETC2_R8G8B8_UNORM_BLOCK",     GL_COMPRESSED_RGB8_ETC2,                        0,                  0,                                  8,   4,  4,   1, 0, 0, 0, 1, 0};
            case ETC2_R8G8B8_SRGB_BLOCK     : return {"ETC2_R8G8B8_SRGB_BLOCK",      GL_COMPRESSED_SRGB8_ETC2,                       0,                  0,                                  8,   4,  4,   1, 0, 0, 0, 1, 0};
            case ETC2_R8G8B8A1_UNORM_BLOCK  : return {"ETC2_R8G8B8A1_UNORM_BLOCK",   GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,    0,                  0,                                  8,   4,  4,   1, 0, 0, 0, 1, 0};
            case ETC2_R8G8B8A1_SRGB_BLOCK   : return {"ETC2_R8G8B8A1_SRGB_BLOCK",    GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,   0,                  0,                                  8,   4,  4,   1, 0, 0, 0, 1, 0};
            case ETC2_R8G8B8A8_UNORM_BLOCK  : return {"ETC2_R8G8B8A8_UNORM_BLOCK",   GL_COMPRESSED_RGBA8_ETC2_EAC,                   0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 0};
            case ETC2_R8G8B8A8_SRGB_BLOCK   : return {"ETC2_R8G8B8A8_SRGB_BLOCK",    GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,            0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 0};
            case EAC_R11_UNORM_BLOCK        : return {"EAC_R11_UNORM_BLOCK",         GL_COMPRESSED_R11_EAC,                          0,                  0,                                  8,   4,  4,   1, 0, 0, 0, 1, 0};
            case EAC_R11_SNORM_BLOCK        : return {"EAC_R11_SNORM_BLOCK",         GL_COMPRESSED_SIGNED_R11_EAC,                   0,                  0,                                  8,   4,  4,   1, 0, 0, 0, 1, 1};
            case EAC_R11G11_UNORM_BLOCK     : return {"EAC_R11G11_UNORM_BLOCK",      GL_COMPRESSED_RG11_EAC,                         0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 0};
            case EAC_R11G11_SNORM_BLOCK     : return {"EAC_R11G11_SNORM_BLOCK",      GL_COMPRESSED_SIGNED_RG11_EAC,                  0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 1};

            //Adaptable Scalable Texture Compression (ASTC)
            case ASTC_4x4_UNORM_BLOCK       : return {"ASTC_4x4_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_4x4,                    0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 0};
            case ASTC_4x4_SRGB_BLOCK        : return {"ASTC_4x4_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4,            0,                  0,                                 16,   4,  4,   1, 0, 0, 0, 1, 0};
            case ASTC_5x4_UNORM_BLOCK       : return {"ASTC_5x4_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_5x4,                    0,                  0,                                 16,   5,  4,   1, 0, 0, 0, 1, 0};
            case ASTC_5x4_SRGB_BLOCK        : return {"ASTC_5x4_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4,            0,                  0,                                 16,   5,  4,   1, 0, 0, 0, 1, 0};
            case ASTC_5x5_UNORM_BLOCK       : return {"ASTC_5x5_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_5x5,                    0,                  0,                                 16,   5,  5,   1, 0, 0, 0, 1, 0};
            case ASTC_5x5_SRGB_BLOCK        : return {"ASTC_5x5_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5,            0,                  0,                                 16,   5,  5,   1, 0, 0, 0, 1, 0};
            case ASTC_6x5_UNORM_BLOCK       : return {"ASTC_6x5_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_6x5,                    0,                  0,                                 16,   6,  5,   1, 0, 0, 0, 1, 0};
            case ASTC_6x5_SRGB_BLOCK        : return {"ASTC_6x5_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5,            0,                  0,                                 16,   6,  5,   1, 0, 0, 0, 1, 0};
            case ASTC_6x6_UNORM_BLOCK       : return {"ASTC_6x6_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_6x6,                    0,                  0,                                 16,   6,  6,   1, 0, 0, 0, 1, 0};
            case ASTC_6x6_SRGB_BLOCK        : return {"ASTC_6x6_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6,            0,                  0,                                 16,   6,  6,   1, 0, 0, 0, 1, 0};
            case ASTC_8x5_UNORM_BLOCK       : return {"ASTC_8x5_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_8x5,                    0,                  0,                                 16,   8,  5,   1, 0, 0, 0, 1, 0};
            case ASTC_8x5_SRGB_BLOCK        : return {"ASTC_8x5_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5,            0,                  0,                                 16,   8,  5,   1, 0, 0, 0, 1, 0};
            case ASTC_8x6_UNORM_BLOCK       : return {"ASTC_8x6_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_8x6,                    0,                  0,                                 16,   8,  6,   1, 0, 0, 0, 1, 0};
            case ASTC_8x6_SRGB_BLOCK        : return {"ASTC_8x6_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6,            0,                  0,                                 16,   8,  6,   1, 0, 0, 0, 1, 0};
            case ASTC_8x8_UNORM_BLOCK       : return {"ASTC_8x8_UNORM_BLOCK",        GL_COMPRESSED_RGBA_ASTC_8x8,                    0,                  0,                                 16,   8,  8,   1, 0, 0, 0, 1, 0};
            case ASTC_8x8_SRGB_BLOCK        : return {"ASTC_8x8_SRGB_BLOCK",         GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8,            0,                  0,                                 16,   8,  8,   1, 0, 0, 0, 1, 0};
            case ASTC_10x5_UNORM_BLOCK      : return {"ASTC_10x5_UNORM_BLOCK",       GL_COMPRESSED_RGBA_ASTC_10x5,                   0,                  0,                                 16,  10,  5,   1, 0, 0, 0, 1, 0};
            case ASTC_10x5_SRGB_BLOCK       : return {"ASTC_10x5_SRGB_BLOCK",        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5,           0,                  0,                                 16,  10,  5,   1, 0, 0, 0, 1, 0};
            case ASTC_10x6_UNORM_BLOCK      : return {"ASTC_10x6_UNORM_BLOCK",       GL_COMPRESSED_RGBA_ASTC_10x6,                   0,                  0,                                 16,  10,  6,   1, 0, 0, 0, 1, 0};
            case ASTC_10x6_SRGB_BLOCK       : return {"ASTC_10x6_SRGB_BLOCK",        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6,           0,                  0,                                 16,  10,  6,   1, 0, 0, 0, 1, 0};
            case ASTC_10x8_UNORM_BLOCK      : return {"ASTC_10x8_UNORM_BLOCK",       GL_COMPRESSED_RGBA_ASTC_10x8,                   0,                  0,                                 16,  10,  8,   1, 0, 0, 0, 1, 0};
            case ASTC_10x8_SRGB_BLOCK       : return {"ASTC_10x8_SRGB_BLOCK",        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8,           0,                  0,                                 16,  10,  8,   1, 0, 0, 0, 1, 0};
            case ASTC_10x10_UNORM_BLOCK     : return {"ASTC_10x10_UNORM_BLOCK",      GL_COMPRESSED_RGBA_ASTC_10x10,                  0,                  0,                                 16,  10, 10,   1, 0, 0, 0, 1, 0};
            case ASTC_10x10_SRGB_BLOCK      : return {"ASTC_10x10_SRGB_BLOCK",       GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10,          0,                  0,                                 16,  10, 10,   1, 0, 0, 0, 1, 0};
            case ASTC_12x10_UNORM_BLOCK     : return {"ASTC_12x10_UNORM_BLOCK",      GL_COMPRESSED_RGBA_ASTC_12x10,                  0,                  0,                                 16,  12, 10,   1, 0, 0, 0, 1, 0};
            case ASTC_12x10_SRGB_BLOCK      : return {"ASTC_12x10_SRGB_BLOCK",       GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10,          0,                  0,                                 16,  12, 10,   1, 0, 0, 0, 1, 0};
            case ASTC_12x12_UNORM_BLOCK     : return {"ASTC_12x12_UNORM_BLOCK",      GL_COMPRESSED_RGBA_ASTC_12x12,                  0,                  0,                                 16,  12, 12,   1, 0, 0, 0, 1, 0};
            case ASTC_12x12_SRGB_BLOCK      : return {"ASTC_12x12_SRGB_BLOCK",       GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12,          0,                  0,                                 16,  12, 12,   1, 0, 0, 0, 1, 0};
        }
                                              return {"INVALID_MemorySurfaceFormat", 0,                                              0,                  0,                                  0,   0,  0,   0, 0, 0, 0, 0, 0};
    }
}
