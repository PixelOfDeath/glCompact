#pragma once
#include "glCompact/TextureInterface.hpp"

namespace glCompact {
    class Texture2dMultisample : public TextureInterface {
        public:
            Texture2dMultisample() = default;
            Texture2dMultisample(SurfaceFormat surfaceFormat, uint32_t x, uint32_t y, uint32_t samples);
            Texture2dMultisample(           const Texture2dMultisample&  texture2dMultisample) = default;
            Texture2dMultisample(                 Texture2dMultisample&& texture2dMultisample) = default;
            Texture2dMultisample& operator=(const Texture2dMultisample&  texture2dMultisample) = default;
            Texture2dMultisample& operator=(      Texture2dMultisample&& texture2dMultisample) = default;
            //Texture2dMultisample(ImageFormat imageFormat, Texture2dMultisample&      srcImages);
            //Texture2dMultisample(ImageFormat imageFormat, Texture2dMultisampleArray& srcImages, int layer);

            static uint32_t getMaxXY();
            static uint32_t getMaxSamplesRgbaNormalizedOrFloat();
            static uint32_t getMaxSamplesDepthAndOrStencil();
            static uint32_t getMaxSamplesRgbaInteger();
    };
}

