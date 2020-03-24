#include "glCompact/RenderBuffer2dMultisample.hpp"
//#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/SurfaceFormatDetail.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    RenderBuffer2dMultisample::RenderBuffer2dMultisample(
        SurfaceFormat surfaceFormat,
        uint32_t      x,
        uint32_t      y,
        uint32_t      samples
    ) {
        UNLIKELY_IF (surfaceFormat->isCompressed)
            throw runtime_error("SurfaceFormat for RenderBuffer must be uncompressed format!");
        UNLIKELY_IF (!surfaceFormat->isRenderable)
            throw runtime_error("SurfaceFormat for RenderBuffer must be renderable!");
        //TODO: throw out sampler = 1 as valid value? Does it has any use case?
        UNLIKELY_IF (!(samples == 1 || samples == 2 || samples == 4 || samples == 8))
            throw runtime_error("Samples must be 1, 2, 4 or 8!");
        UNLIKELY_IF (x > getMaxXY() || y > getMaxXY())
            throw runtime_error("Trying to create RenderBuffer2dMultisample with size(x = " + to_string(x) + ", y = " + to_string(y) + "), but that is bayond getMaxXY(GL_MAX_RENDERBUFFER_SIZE = " + to_string(threadContextGroup_->values.GL_MAX_RENDERBUFFER_SIZE) + ")");
        //TODO: check sampler limit

        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            threadContextGroup_->functions.glCreateRenderbuffers(1, &id);
            threadContextGroup_->functions.glNamedRenderbufferStorageMultisample(id, samples, surfaceFormat->sizedFormat, x, y);
        } else {
            threadContextGroup_->functions.glGenRenderbuffers(1, &id);
            threadContextGroup_->functions.glBindRenderbuffer(GL_RENDERBUFFER, id); //TODO use caching?
            threadContextGroup_->functions.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, surfaceFormat->sizedFormat, x, y);
        }

        this->target        = GL_RENDERBUFFER;
        this->mipmapCount   = 1;
        this->x             = x;
        this->y             = y;
        this->z             = 1;
        this->samples       = samples;
        this->surfaceFormat = surfaceFormat;
    }

    /*
        Returns maximum supported x and y size. Minimum supported value is 64.
    */
    uint32_t RenderBuffer2dMultisample::getMaxXY() {
        return threadContextGroup_->values.GL_MAX_RENDERBUFFER_SIZE;
    }

    /*
        Returns the maximum supported samples for all renderBuffer formats (rgba normalized, rgba float, depth and/or stencil), except rgba non-normalized integer formats
    */
    uint32_t RenderBuffer2dMultisample::getMaxSamples() {
        return threadContextGroup_->values.GL_MAX_SAMPLES;
    }

    /*
        Returns the maximum supported samples for rgba non-normalized integer formats
    */
    uint32_t RenderBuffer2dMultisample::getMaxSamplesRgbaInteger() {
        return threadContextGroup_->values.GL_MAX_INTEGER_SAMPLES;
    }
}
