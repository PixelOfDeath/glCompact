#pragma once
#include "glCompact/CompareOperator.hpp"
#include <cstdint> //C++11

namespace glCompact {
    enum class MagnificationFilter : int32_t {
        nearest = 0x2600, //GL_NEAREST
        linear  = 0x2601  //GL_LINEAR
    };

    enum class MinificationFilter : int32_t {
        nearestFromMipmap0        = 0x2600, //GL_NEAREST
        nearestFromMipmapNearest  = 0x2700, //GL_NEAREST_MIPMAP_NEAREST
        nearestFromMipmapsLinear  = 0x2702, //GL_NEAREST_MIPMAP_LINEAR
        linearFromMipmap0         = 0x2601, //GL_LINEAR
        linearFromMipmapNearest   = 0x2701, //GL_LINEAR_MIPMAP_NEAREST
        linearFromMipmapsLinear   = 0x2703  //GL_LINEAR_MIPMAP_LINEAR
    };

    enum class WrapMode : int32_t {
        repeat            = 0x2901, //GL_REPEAT
        clampToEdge       = 0x812F, //GL_CLAMP_TO_EDGE
        mirrorRepeat      = 0x8370, //GL_MIRRORED_REPEAT
        mirrorClampToEdge = 0x8743  //GL_MIRROR_CLAMP_TO_EDGE //ARB_texture_mirror_clamp_to_edge (Core since 4.4)
    };

    class Sampler {
            friend class PipelineInterface;
        public:
            Sampler();
            ~Sampler();

            void setMagnificationFilter(MagnificationFilter magnificationFilter);
            void setMinificationFilter(MinificationFilter minificationFilter);
            void setMaxAnisotropy(float maxAnisotropy);
            void setMinLod(float minLod);
            void setMaxLod(float maxLod);
            void setLodBias(float lodBias);
            void setWrapModeX(WrapMode wrapModeX);
            void setWrapModeY(WrapMode wrapModeY);
            void setWrapModeZ(WrapMode wrapModeZ);
            void setWrapModeXY(WrapMode wrapModeXY);
            void setWrapModeXY(WrapMode wrapModeX, WrapMode wrapModeY);
            void setWrapModeXYZ(WrapMode wrapModeXYZ);
            void setWrapModeXYZ(WrapMode wrapModeX, WrapMode wrapModeY, WrapMode wrapModeZ);

            void setDepthCompareMode(CompareOperator compareOperator);
            void setDepthCompareModeOff();

        private:
            uint32_t id;

            void setParameter(int32_t pname, float   value);
            void setParameter(int32_t pname, int32_t value);

            void detachFromThreadContext();
    };
}
