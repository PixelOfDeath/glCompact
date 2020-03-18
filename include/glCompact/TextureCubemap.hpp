#pragma once
#include "glCompact/TextureInterface.hpp"

namespace glCompact {
    class TextureCubemap : public TextureInterface {
        public:
            TextureCubemap(SurfaceFormat surfaceFormat, uint32_t xy, bool mipmaps);
            //TextureCubemap(ImageFormat imageFormat, Texture2dArray&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount);
            //TextureCubemap(ImageFormat imageFormat, TextureCubemap&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount);
            //TextureCubemap(ImageFormat imageFormat, TextureCubemapArray& srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount);

            static uint32_t getMaxXY();
    };
}


