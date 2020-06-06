#pragma once
#include "glCompact/TextureInterface.hpp"

namespace glCompact {
    class Texture2dMultisampleArray : public TextureInterface {
        public:
            Texture2dMultisampleArray(SurfaceFormat surfaceFormat, uint32_t x, uint32_t y, uint32_t layers, uint32_t samples);
            Texture2dMultisampleArray(           const Texture2dMultisampleArray&  texture2dMultisampleArray) = default;
            Texture2dMultisampleArray(                 Texture2dMultisampleArray&& texture2dMultisampleArray) = default;
            Texture2dMultisampleArray& operator=(const Texture2dMultisampleArray&  texture2dMultisampleArray) = default;
            Texture2dMultisampleArray& operator=(      Texture2dMultisampleArray&& texture2dMultisampleArray) = default;
            //Texture2dMultisampleArray(ImageFormat imageFormat, Texture2dMultisample&      srcImages);
            //Texture2dMultisampleArray(ImageFormat imageFormat, Texture2dMultisampleArray& srcImages, int layer, int layerCount);

            static uint32_t getMaxXY();
            static uint32_t getMaxLayers();
            static uint32_t getMaxSamplesRgbaNormalizedOrFloat();
            static uint32_t getMaxSamplesDepthAndOrStencil();
            static uint32_t getMaxSamplesRgbaInteger();
    };
}


