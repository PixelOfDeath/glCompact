#include "glCompact/Texture2dArray.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    Texture2dArray::Texture2dArray(
        SurfaceFormat surfaceFormat,
        uint32_t      x,
        uint32_t      y,
        uint32_t      layers,
        bool          mipmaps
    ) {
        UNLIKELY_IF (x > getMaxXY() || y > getMaxXY())
            throw runtime_error("Trying to create Texture2dArray with size(x = " + to_string(x) + ", y = " + to_string(y) + "), but that is bayond getMaxXY(GL_MAX_TEXTURE_SIZE = " + to_string(threadContextGroup_->values.GL_MAX_TEXTURE_SIZE) + ")");
        UNLIKELY_IF (layers > getMaxLayers())
            throw runtime_error("Trying to create Texture2dArray with size(layers = " + to_string(layers) + "), but that is bayond getMaxLayers(GL_MAX_ARRAY_TEXTURE_LAYERS = " + to_string(threadContextGroup_->values.GL_MAX_ARRAY_TEXTURE_LAYERS) + ")");
        create(GL_TEXTURE_2D_ARRAY, surfaceFormat, x, y, layers, mipmaps, 0);
    }

    /*
        Returns maximum supported x and y size. Minimum supported value is 1024.
    */
    uint32_t Texture2dArray::getMaxXY() {
        return threadContextGroup_->values.GL_MAX_TEXTURE_SIZE;
    }

    /*
        Returns maximum supported layer count. Minimum supported value is 256.
    */
    uint32_t Texture2dArray::getMaxLayers() {
        return threadContextGroup_->values.GL_MAX_ARRAY_TEXTURE_LAYERS;
    }
}

/*Texture2dArray::Texture2dArray(ImageFormat imageFormat, Texture2d&           srcImages, int firstMipmap, bool mipmaps) {
    createView(srcImages, GL_TEXTURE_2D_ARRAY, imageFormat, firstMipmap, mipmaps, 0, 1);
}
Texture2dArray::Texture2dArray(ImageFormat imageFormat, Texture2dArray&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_2D_ARRAY, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}
Texture2dArray::Texture2dArray(ImageFormat imageFormat, TextureCubemap&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_2D_ARRAY, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}
Texture2dArray::Texture2dArray(ImageFormat imageFormat, TextureCubemapArray& srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_2D_ARRAY, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}*/
