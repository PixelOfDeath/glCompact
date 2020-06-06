#include "glCompact/RenderBuffer2dMultisample.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include <stdexcept>

using namespace std;

namespace glCompact {
    RenderBuffer2dMultisample::RenderBuffer2dMultisample(
        SurfaceFormat surfaceFormat,
        uint32_t      x,
        uint32_t      y,
        uint32_t      samples
    ) {
        //TODO: throw out sampler = 1 as valid value? Does it has any use case?
        //TODO: check sampler limit of implementation
        UNLIKELY_IF (!(samples == 1 || samples == 2 || samples == 4 || samples == 8))
            throw runtime_error("Samples must be 1, 2, 4 or 8!");

        create(surfaceFormat, {x, y}, samples);
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
