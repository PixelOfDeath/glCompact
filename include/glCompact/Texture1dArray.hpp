#pragma once
#include "glCompact/TextureInterface.hpp"

namespace glCompact {
    class Texture1dArray : public TextureInterface {
        public:
            Texture1dArray(SurfaceFormat surfaceFormat, uint32_t x, uint32_t yLayers, bool mipmaps);
            Texture1dArray(           const Texture1dArray&  texture1dArray) = default;
            Texture1dArray(                 Texture1dArray&& texture1dArray) = default;
            Texture1dArray& operator=(const Texture1dArray&  texture1dArray) = default;
            Texture1dArray& operator=(      Texture1dArray&& texture1dArray) = default;
            //Texture1dArray(ImageFormat imageFormat, Texture1d&      srcImages, int firstMipmap, bool mipmaps);
            //Texture1dArray(ImageFormat imageFormat, Texture1dArray& srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount);

            static uint32_t getMaxX();
            static uint32_t getMaxLayers();
    };
}
