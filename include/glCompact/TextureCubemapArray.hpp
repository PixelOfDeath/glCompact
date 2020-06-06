#pragma once
#include "glCompact/TextureInterface.hpp"

namespace glCompact {
    //This kind of texture depends on GL_ARB_texture_cube_map_array (Core since 4.0)
    class TextureCubemapArray : public TextureInterface {
        public:
            TextureCubemapArray(SurfaceFormat surfaceFormat, uint32_t xy, uint32_t cubeMapCount, bool mipmaps);
            TextureCubemapArray(           const TextureCubemapArray&  textureCubemapArray) = default;
            TextureCubemapArray(                 TextureCubemapArray&& textureCubemapArray) = default;
            TextureCubemapArray& operator=(const TextureCubemapArray&  textureCubemapArray) = default;
            TextureCubemapArray& operator=(      TextureCubemapArray&& textureCubemapArray) = default;
            //TextureCubemapArray(ImageFormat imageFormat, Texture2dArray&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount);
            //TextureCubemapArray(ImageFormat imageFormat, TextureCubemap&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount);
            //TextureCubemapArray(ImageFormat imageFormat, TextureCubemapArray& srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount);

            static uint32_t getMaxXY();
            static uint32_t getMaxLayers();
    };
}

