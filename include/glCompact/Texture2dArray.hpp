#pragma once
#include "glCompact/TextureInterface.hpp"

namespace glCompact {
    class Texture2dArray : public TextureInterface {
        public:
            Texture2dArray() = default;
            Texture2dArray(SurfaceFormat surfaceFormat, uint32_t x, uint32_t y, uint32_t layers, bool mipmaps);
            Texture2dArray(           const Texture2dArray&  texture2dArray) = default;
            Texture2dArray(                 Texture2dArray&& texture2dArray) = default;
            Texture2dArray& operator=(const Texture2dArray&  texture2dArray) = default;
            Texture2dArray& operator=(      Texture2dArray&& texture2dArray) = default;
            //Texture2dArray(ImageFormat imageFormat, Texture2d&           srcImages, int firstMipmap, bool mipmaps);
            //Texture2dArray(ImageFormat imageFormat, Texture2dArray&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount);
            //Texture2dArray(ImageFormat imageFormat, TextureCubemap&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount);
            //Texture2dArray(ImageFormat imageFormat, TextureCubemapArray& srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount);

            static uint32_t getMaxXY();
            static uint32_t getMaxLayers();
    };
}

