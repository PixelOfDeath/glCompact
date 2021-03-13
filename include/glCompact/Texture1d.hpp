#pragma once
#include "glCompact/TextureInterface.hpp"

namespace glCompact {
    class Texture1d : public TextureInterface {
        public:
            Texture1d() = default;
            Texture1d(SurfaceFormat surfaceFormat, uint32_t x, bool mipmaps);
            Texture1d(           const Texture1d&  texture1d) = default;
            Texture1d(                 Texture1d&& texture1d) = default;
            Texture1d& operator=(const Texture1d&  texture1d) = default;
            Texture1d& operator=(      Texture1d&& texture1d) = default;

            static uint32_t getMaxX();
    };
}
