#include "glCompact/RenderBufferInterface.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/SurfaceFormatDetail.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    RenderBufferInterface::RenderBufferInterface(const RenderBufferInterface& sourceRenderBuffer) {
        create(sourceRenderBuffer.surfaceFormat, sourceRenderBuffer.x, sourceRenderBuffer.y, sourceRenderBuffer.samples);
        if (threadContextGroup_->extensions.GL_ARB_copy_image) {
            copyFromSurfaceMemory    (sourceRenderBuffer, 0, {0, 0, 0}, 0, {0, 0, 0}, {sourceRenderBuffer.x, sourceRenderBuffer.y, 1});
        } else {
            copyFromSurfaceComponents(sourceRenderBuffer, 0, {0, 0, 0}, 0, {0, 0, 0}, {sourceRenderBuffer.x, sourceRenderBuffer.y, 1});
        }
    }

    void RenderBufferInterface::create(SurfaceFormat surfaceFormat, uint32_t x, uint32_t y, uint32_t samples) {
        UNLIKELY_IF (surfaceFormat->isCompressed)
            throw runtime_error("SurfaceFormat for RenderBuffer must be uncompressed format!");
        UNLIKELY_IF (!surfaceFormat->isRenderable)
            throw runtime_error("SurfaceFormat for RenderBuffer must be renderable!");
        //TODO: throw out sampler = 1 as valid value? Does it has any use case?
        UNLIKELY_IF (x > getMaxXY() || y > getMaxXY())
            throw runtime_error("Trying to create RenderBuffer with size(x = " + to_string(x) + ", y = " + to_string(y) + "), but that is bayond getMaxXY(GL_MAX_RENDERBUFFER_SIZE = " + to_string(threadContextGroup_->values.GL_MAX_RENDERBUFFER_SIZE) + ")");

        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            threadContextGroup_->functions.glCreateRenderbuffers(1, &id);
            if (samples)
                threadContextGroup_->functions.glNamedRenderbufferStorageMultisample(id, samples, surfaceFormat->sizedFormat, x, y);
            else
                threadContextGroup_->functions.glNamedRenderbufferStorage(id, surfaceFormat->sizedFormat, x, y);
        } else {
            threadContextGroup_->functions.glGenRenderbuffers(1, &id);
            threadContextGroup_->functions.glBindRenderbuffer(GL_RENDERBUFFER, id); //TODO use caching?
            if (samples)
                threadContextGroup_->functions.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, surfaceFormat->sizedFormat, x, y);
            else
                threadContextGroup_->functions.glRenderbufferStorage(GL_RENDERBUFFER, surfaceFormat->sizedFormat, x, y);
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
    uint32_t RenderBufferInterface::getMaxXY() {
        return threadContextGroup_->values.GL_MAX_RENDERBUFFER_SIZE;
    }
}
