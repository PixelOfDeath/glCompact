#pragma once
#include "glCompact/TextureInterface.hpp"

namespace glCompact {
    class Texture3d : public TextureInterface {
        public:
            Texture3d(SurfaceFormat surfaceFormat, uint32_t x, uint32_t y, uint32_t z, bool mipmaps);
            Texture3d(const Texture3d& sourceTexture) = default;
            Texture3d& operator=(Texture3d&& texture3d) = default;
            //Texture3d(ImageFormat imageFormat, Texture3d& srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount);

            static uint32_t getMaxXYZ();
    };
}


