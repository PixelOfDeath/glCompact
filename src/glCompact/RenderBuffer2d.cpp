/*
    glCompact
    Copyright (C) 2019-2020 Frederik Uje vom Hofe

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "glCompact/RenderBuffer2d.hpp"
//#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"
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
            throw runtime_error("Trying to create RenderBuffer2d with size(x = " + to_string(x) + ", y = " + to_string(y) + "), but that is bayond getMaxXY(GL_MAX_RENDERBUFFER_SIZE = " + to_string(threadContextGroup->values.GL_MAX_RENDERBUFFER_SIZE) + ")");

        if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
            threadContextGroup->functions.glCreateRenderbuffers(1, &id);
            threadContextGroup->functions.glNamedRenderbufferStorage(id, surfaceFormat->sizedFormat, x, y);
        } else {
            threadContextGroup->functions.glGenRenderbuffers(1, &id);
            threadContextGroup->functions.glBindRenderbuffer(GL_RENDERBUFFER, id); //TODO use caching?
            threadContextGroup->functions.glRenderbufferStorage(GL_RENDERBUFFER, surfaceFormat->sizedFormat, x, y);
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
        return threadContextGroup->values.GL_MAX_RENDERBUFFER_SIZE;
    }
}
