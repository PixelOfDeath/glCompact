#include "glCompact/RenderBuffer2d.hpp"
//#include "glCompact/ThreadContext.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/SurfaceFormatDetail.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    RenderBuffer2d::RenderBuffer2d(
        SurfaceFormat surfaceFormat,
        uint32_t      x,
        uint32_t      y
    ) {
        UNLIKELY_IF (surfaceFormat->isCompressed)
            throw runtime_error("SurfaceFormat for RenderBuffer must be uncompressed format!");
        UNLIKELY_IF (!surfaceFormat->isRenderable)
            throw runtime_error("SurfaceFormat for RenderBuffer must be renderable!");
        UNLIKELY_IF (x > getMaxXY() || y > getMaxXY())
            throw runtime_error("Trying to create RenderBuffer2d with size(x = " + to_string(x) + ", y = " + to_string(y) + "), but that is bayond getMaxXY(GL_MAX_RENDERBUFFER_SIZE = " + to_string(threadContextGroup_->values.GL_MAX_RENDERBUFFER_SIZE) + ")");

        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            threadContextGroup_->functions.glCreateRenderbuffers(1, &id);
            threadContextGroup_->functions.glNamedRenderbufferStorage(id, surfaceFormat->sizedFormat, x, y);
        } else {
            threadContextGroup_->functions.glGenRenderbuffers(1, &id);
            threadContextGroup_->functions.glBindRenderbuffer(GL_RENDERBUFFER, id); //TODO use caching?
            threadContextGroup_->functions.glRenderbufferStorage(GL_RENDERBUFFER, surfaceFormat->sizedFormat, x, y);
        }

        this->target        = GL_RENDERBUFFER;
        this->mipmapCount   = 1;
        this->x             = x;
        this->y             = y;
        this->z             = 1;
        this->samples       = 0;
        this->surfaceFormat = surfaceFormat;
    }

    /*
        Returns maximum supported x and y size. Minimum supported value is 64.
    */
    uint32_t RenderBuffer2d::getMaxXY() {
        return threadContextGroup_->values.GL_MAX_RENDERBUFFER_SIZE;
    }
}
