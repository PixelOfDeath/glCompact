#include "glCompact/Texture2dMultisample.hpp"
#include "glCompact/gl/Constants.hpp"
//#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    Texture2dMultisample::Texture2dMultisample(
        SurfaceFormat surfaceFormat,
        uint32_t      x,
        uint32_t      y,
        uint32_t      samples
    ) {
        UNLIKELY_IF (x > getMaxXY() || y > getMaxXY())
            throw runtime_error("Trying to create Texture2dMultisample with size(x = " + to_string(x) + ", y = " + to_string(y) + "), but that is bayond getMaxXY(GL_MAX_TEXTURE_SIZE = " + to_string(threadContextGroup_->values.GL_MAX_TEXTURE_SIZE) + ")");
        //TODO: check sample limits
        create(GL_TEXTURE_2D_MULTISAMPLE, surfaceFormat, x, y, 1, false, samples);
    }

    /*
        Returns maximum supported x and y size. Minimum supported value is 1024.
    */
    uint32_t Texture2dMultisample::getMaxXY() {
        return threadContextGroup_->values.GL_MAX_TEXTURE_SIZE;
    }

    /*
        Returns the maximum supported samples for all texture rgba formats (except non-normalized integer formats)
    */
    uint32_t Texture2dMultisample::getMaxSamplesRgbaNormalizedOrFloat() {
        return threadContextGroup_->values.GL_MAX_COLOR_TEXTURE_SAMPLES;
    }

    /*
        Returns the maximum supported samples for all texture depth and/or stencil formats
    */
    uint32_t Texture2dMultisample::getMaxSamplesDepthAndOrStencil() {
        return threadContextGroup_->values.GL_MAX_DEPTH_TEXTURE_SAMPLES;
    }

    /*
        Returns the maximum supported samples for non-normalized rgba integer formats
    */
    uint32_t Texture2dMultisample::getMaxSamplesRgbaInteger() {
        return threadContextGroup_->values.GL_MAX_INTEGER_SAMPLES;
    }
}

/*Texture2dMultisample::Texture2dMultisample(ImageFormat imageFormat, Texture2dMultisample&      srcImages) {
    createView(srcImages, GL_TEXTURE_2D_MULTISAMPLE, imageFormat, 0, 0, 0, 1);
}
Texture2dMultisample::Texture2dMultisample(ImageFormat imageFormat, Texture2dMultisampleArray& srcImages, int layer) {
    createView(srcImages, GL_TEXTURE_2D_MULTISAMPLE, imageFormat, 0, 0, layer, 1);
}*/
