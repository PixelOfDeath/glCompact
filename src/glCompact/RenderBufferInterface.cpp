#include "glCompact/RenderBufferInterface.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/SurfaceFormatDetail.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;
using namespace glm;

namespace glCompact {
    RenderBufferInterface::RenderBufferInterface(const RenderBufferInterface& renderBufferInterface) {
        create(renderBufferInterface.surfaceFormat, renderBufferInterface.size, renderBufferInterface.samples);
        if (threadContextGroup_->extensions.GL_ARB_copy_image) {
            copyFromSurfaceMemory    (renderBufferInterface, 0, {0, 0, 0}, 0, {0, 0, 0}, renderBufferInterface.size);
        } else {
            copyFromSurfaceComponents(renderBufferInterface, 0, {0, 0, 0}, 0, {0, 0, 0}, renderBufferInterface.size);
        }
    }

    RenderBufferInterface& RenderBufferInterface::operator=(const RenderBufferInterface& renderBufferInterface) {
        free();
        create(renderBufferInterface.surfaceFormat, renderBufferInterface.size, renderBufferInterface.samples);
        if (threadContextGroup_->extensions.GL_ARB_copy_image) {
            copyFromSurfaceMemory    (renderBufferInterface, 0, {0, 0, 0}, 0, {0, 0, 0}, renderBufferInterface.size);
        } else {
            copyFromSurfaceComponents(renderBufferInterface, 0, {0, 0, 0}, 0, {0, 0, 0}, renderBufferInterface.size);
        }
        return *this;
    }

    void RenderBufferInterface::create(SurfaceFormat surfaceFormat, uvec2 newSize, uint32_t samples) {
        UNLIKELY_IF (surfaceFormat->isCompressed)
            throw runtime_error("SurfaceFormat for RenderBuffer must be uncompressed format!");
        UNLIKELY_IF (!surfaceFormat->isRenderable)
            throw runtime_error("SurfaceFormat for RenderBuffer must be renderable!");
        //TODO: throw out sampler = 1 as valid value? Does it has any use case?
        UNLIKELY_IF (newSize.x > getMaxXY() || newSize.y > getMaxXY())
            throw runtime_error("Trying to create RenderBuffer with size(x = " + to_string(newSize.x) + ", y = " + to_string(newSize.y) + "), but that is bayond getMaxXY(GL_MAX_RENDERBUFFER_SIZE = " + to_string(threadContextGroup_->values.GL_MAX_RENDERBUFFER_SIZE) + ")");

        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            threadContextGroup_->functions.glCreateRenderbuffers(1, &id);
            if (samples)
                threadContextGroup_->functions.glNamedRenderbufferStorageMultisample(id, samples, surfaceFormat->sizedFormat, newSize.x, newSize.y);
            else
                threadContextGroup_->functions.glNamedRenderbufferStorage(id, surfaceFormat->sizedFormat, newSize.x, newSize.y);
        } else {
            threadContextGroup_->functions.glGenRenderbuffers(1, &id);
            threadContextGroup_->functions.glBindRenderbuffer(GL_RENDERBUFFER, id); //TODO use caching?
            if (samples)
                threadContextGroup_->functions.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, surfaceFormat->sizedFormat, newSize.x, newSize.y);
            else
                threadContextGroup_->functions.glRenderbufferStorage(GL_RENDERBUFFER, surfaceFormat->sizedFormat, newSize.x, newSize.y);
        }

        this->target        = GL_RENDERBUFFER;
        this->mipmapCount   = 1;
        this->size          = uvec3(newSize, 1);
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
