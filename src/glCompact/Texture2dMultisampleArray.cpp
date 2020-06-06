#include "glCompact/Texture2dMultisampleArray.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    Texture2dMultisampleArray::Texture2dMultisampleArray(
        SurfaceFormat surfaceFormat,
        uint32_t      x,
        uint32_t      y,
        uint32_t      layers,
        uint32_t      samples
    ) {
        UNLIKELY_IF (x > getMaxXY() || y > getMaxXY())
            throw runtime_error("Trying to create Texture2dMultisampleArray with size(x = " + to_string(x) + ", y = " + to_string(y) + "), but that is bayond getMaxXY(GL_MAX_TEXTURE_SIZE = " + to_string(threadContextGroup_->values.GL_MAX_TEXTURE_SIZE) + ")");
        UNLIKELY_IF (layers > getMaxLayers())
            throw runtime_error("Trying to create Texture2dMultisampleArray with size(layers = " + to_string(layers) + "), but that is bayond getMaxLayers(GL_MAX_ARRAY_TEXTURE_LAYERS = " + to_string(threadContextGroup_->values.GL_MAX_ARRAY_TEXTURE_LAYERS) + ")");
        //TODO: check sample limits
        create(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, surfaceFormat, {x, y, layers}, false, samples);
    }

    /*
        Returns maximum supported x and y size. Minimum supported value is 1024.
    */
    uint32_t Texture2dMultisampleArray::getMaxXY() {
        return threadContextGroup_->values.GL_MAX_TEXTURE_SIZE;
    }

    /*
        Returns maximum supported layer count. Minimum supported value is 256.
    */
    uint32_t Texture2dMultisampleArray::getMaxLayers() {
        return threadContextGroup_->values.GL_MAX_ARRAY_TEXTURE_LAYERS;
    }

    /*
        Returns the maximum supported samples for all texture rgba formats (except non-normalized integer formats)
    */
    uint32_t Texture2dMultisampleArray::getMaxSamplesRgbaNormalizedOrFloat() {
        return threadContextGroup_->values.GL_MAX_COLOR_TEXTURE_SAMPLES;
    }

    /*
        Returns the maximum supported samples for all texture depth and/or stencil formats
    */
    uint32_t Texture2dMultisampleArray::getMaxSamplesDepthAndOrStencil() {
        return threadContextGroup_->values.GL_MAX_DEPTH_TEXTURE_SAMPLES;
    }

    /*
        Returns the maximum supported samples for non-normalized rgba integer formats
    */
    uint32_t Texture2dMultisampleArray::getMaxSamplesRgbaInteger() {
        return threadContextGroup_->values.GL_MAX_INTEGER_SAMPLES;
    }
}

/*Texture2dMultisampleArray::Texture2dMultisampleArray(ImageFormat imageFormat, Texture2dMultisample&      srcImages) {
    createView(srcImages, GL_TEXTURE_2D_MULTISAMPLE_ARRAY, imageFormat, 0, 0, 0, 1);
}
Texture2dMultisampleArray::Texture2dMultisampleArray(ImageFormat imageFormat, Texture2dMultisampleArray& srcImages, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_2D_MULTISAMPLE_ARRAY, imageFormat, 0, 0, layer, layerCount);
}*/
