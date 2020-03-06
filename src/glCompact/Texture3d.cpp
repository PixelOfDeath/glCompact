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
#include "glCompact/Texture3d.hpp"
#include "glCompact/gl/Constants.hpp"
//#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    Texture3d::Texture3d(
        SurfaceFormat surfaceFormat,
        uint32_t      x,
        uint32_t      y,
        uint32_t      z,
        bool          mipmaps
    ) {
        UNLIKELY_IF (x > getMaxXYZ() || y > getMaxXYZ() || z > getMaxXYZ())
            throw runtime_error("Trying to create Texture3d with size(x = " + to_string(x) + ", y = " + to_string(y) + ", z = " + to_string(z) + "), but that is bayond getMaxXY(GL_MAX_3D_TEXTURE_SIZE = " + to_string(threadContextGroup->values.GL_MAX_3D_TEXTURE_SIZE) + ")");
        create(GL_TEXTURE_3D, surfaceFormat, x, y, z, mipmaps, 0);
    }

    /*
        returns the maximum supported X, Y and Z size for a 3d texture. The minimum supported value is 64!
    */
    uint32_t Texture3d::getMaxXYZ() {
        return threadContextGroup->values.GL_MAX_3D_TEXTURE_SIZE;
    }
}

/*Texture3d::Texture3d(ImageFormat imageFormat, Texture3d& srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_3D, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}*/
