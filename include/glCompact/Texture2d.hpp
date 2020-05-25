#pragma once
#include "glCompact/TextureInterface.hpp"

namespace glCompact {
    class Texture2d : public TextureInterface {
        public:
            Texture2d(SurfaceFormat surfaceFormat, uint32_t x, uint32_t y, bool mipmaps);
            Texture2d(const Texture2d& sourceTexture) = default;
            Texture2d& operator=(Texture2d&& texture2d) = default;
            //Texture2d(ImageFormat imageFormat, Texture2d&           srcImages, int firstMipmap, bool mipmaps);
            //Texture2d(ImageFormat imageFormat, Texture2dArray&      srcImages, int firstMipmap, bool mipmaps, int layer);
            //Texture2d(ImageFormat imageFormat, TextureCubemap&      srcImages, int firstMipmap, bool mipmaps, int layer);
            //Texture2d(ImageFormat imageFormat, TextureCubemapArray& srcImages, int firstMipmap, bool mipmaps, int layer);

            static uint32_t getMaxXY();
    };
}
