#include "glCompact/TextureCubemapArray.hpp"
#include "glCompact/gl/Constants.hpp"
//#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    //This kind of texture depends on GL_ARB_texture_cube_map_array (Core since 4.0)
    TextureCubemapArray::TextureCubemapArray(
        SurfaceFormat surfaceFormat,
        uint32_t      xy,
        uint32_t      cubeMapCount,
        bool          mipmaps
    ) {
        UNLIKELY_IF (!threadContextGroup_->extensions.GL_ARB_texture_cube_map_array)
            throw std::runtime_error("Missing GL_ARB_Texture_cube_map_array to create Texture!");
        UNLIKELY_IF (xy > getMaxXY())
            throw runtime_error("Trying to create TextureCubemapArray with size(xy = " + to_string(xy) + "), but that is bayond getMaxXY(GL_MAX_CUBE_MAP_TEXTURE_SIZE = " + to_string(threadContextGroup_->values.GL_MAX_CUBE_MAP_TEXTURE_SIZE) + ")");
        UNLIKELY_IF (6 * cubeMapCount > getMaxLayers())
            throw runtime_error("Trying to create TextureCubemapArray with size(cubeMapCount * 6 = layers = " + to_string(6 * cubeMapCount) + "), but that is bayond getMaxLayers(GL_MAX_ARRAY_TEXTURE_LAYERS = " + to_string(threadContextGroup_->values.GL_MAX_ARRAY_TEXTURE_LAYERS) + ")");
        TextureInterface::create(GL_TEXTURE_CUBE_MAP_ARRAY, surfaceFormat, xy, xy, 6 * cubeMapCount, mipmaps, 0);
    }

    /*
        Returns maximum supported xy size for a cubemap texture. Minimum supported value is 1024.
    */
    uint32_t TextureCubemapArray::getMaxXY() {
        return threadContextGroup_->values.GL_MAX_CUBE_MAP_TEXTURE_SIZE;
    }

    /*
        Returns maximum supported layer count. Minimum supported value is 256.
    */
    uint32_t TextureCubemapArray::getMaxLayers() {
        return threadContextGroup_->values.GL_MAX_ARRAY_TEXTURE_LAYERS;
    }
}
/*TextureCubemapArray::TextureCubemapArray(ImageFormat imageFormat, Texture2dArray&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_3D, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}
TextureCubemapArray::TextureCubemapArray(ImageFormat imageFormat, TextureCubemap&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_3D, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}
TextureCubemapArray::TextureCubemapArray(ImageFormat imageFormat, TextureCubemapArray& srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_3D, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}*/
