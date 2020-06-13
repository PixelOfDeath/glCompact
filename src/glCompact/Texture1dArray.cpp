#include "glCompact/Texture1dArray.hpp"
#include "glCompact/gl/Constants.hpp"
//#include "glCompact/ThreadContext.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/threadContextGroup_.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

//Uses different sampling and mipmaps then Texture1d

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    Texture1dArray::Texture1dArray(
        SurfaceFormat surfaceFormat,
        uint32_t      x,
        uint32_t      yLayers,
        bool          mipmaps
    ) {
        UNLIKELY_IF (x > getMaxX())
            throw runtime_error("Trying to create Texture1dArray with size(x = " + to_string(x) + "), but that is bayond getMaxX(GL_MAX_TEXTURE_SIZE = " + to_string(threadContextGroup_->values.GL_MAX_TEXTURE_SIZE) + ")");
        UNLIKELY_IF (yLayers > getMaxLayers())
            throw runtime_error("Trying to create Texture1dArray with size(yLayers = " + to_string(yLayers) + "), but that is bayond getMaxLayers(GL_MAX_ARRAY_TEXTURE_LAYERS = " + to_string(threadContextGroup_->values.GL_MAX_ARRAY_TEXTURE_LAYERS) + ")");
        create(GL_TEXTURE_1D_ARRAY, surfaceFormat, {x, yLayers, 1}, mipmaps, 0);
    }

    /*
        returns the maximum supported X size for a 1d texture. The minimum supported value is 1024.
    */
    uint32_t Texture1dArray::getMaxX() {
        return threadContextGroup_->values.GL_MAX_TEXTURE_SIZE;
    }

    /*
        Returns maximum layers texture can be created with. Minimum supported value is 256.
    */
    uint32_t Texture1dArray::getMaxLayers() {
        return threadContextGroup_->values.GL_MAX_ARRAY_TEXTURE_LAYERS;
    }
}

/*Texture1dArray::Texture1dArray(ImageFormat imageFormat, Texture1d& srcImages, int firstMipmap, bool mipmaps) {
    createView(srcImages, GL_TEXTURE_1D_ARRAY, imageFormat, firstMipmap, mipmaps, 0, 1);
}
Texture1dArray::Texture1dArray(ImageFormat imageFormat, Texture1dArray& srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_1D_ARRAY, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}*/
