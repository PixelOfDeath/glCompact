/*
    This file is part of glCompact.
    Copyright (C) 2019-2020 Frederik Uje vom Hofe

    glCompact is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    glCompact is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "glCompact/RenderBuffer2dMultisample.hpp"
//#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"
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
            throw runtime_error("Trying to create RenderBuffer2dMultisample with size(x = " + to_string(x) + ", y = " + to_string(y) + "), but that is bayond getMaxXY(GL_MAX_RENDERBUFFER_SIZE = " + to_string(threadContextGroup->values.GL_MAX_RENDERBUFFER_SIZE) + ")");
        //TODO: check sampler limit

        if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
            threadContextGroup->functions.glCreateRenderbuffers(1, &id);
            threadContextGroup->functions.glNamedRenderbufferStorageMultisample(id, samples, surfaceFormat->sizedFormat, x, y);
        } else {
            threadContextGroup->functions.glGenRenderbuffers(1, &id);
            threadContextGroup->functions.glBindRenderbuffer(GL_RENDERBUFFER, id); //TODO use caching?
            threadContextGroup->functions.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, surfaceFormat->sizedFormat, x, y);
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
        return threadContextGroup->values.GL_MAX_RENDERBUFFER_SIZE;
    }

    /*
        Returns the maximum supported samples for all renderBuffer formats (rgba normalized, rgba float, depth and/or stencil), except rgba non-normalized integer formats
    */
    uint32_t RenderBuffer2dMultisample::getMaxSamples() {
        return threadContextGroup->values.GL_MAX_SAMPLES;
    }

    /*
        Returns the maximum supported samples for rgba non-normalized integer formats
    */
    uint32_t RenderBuffer2dMultisample::getMaxSamplesRgbaInteger() {
        return threadContextGroup->values.GL_MAX_INTEGER_SAMPLES;
    }
}
