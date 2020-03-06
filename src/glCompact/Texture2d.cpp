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
#include "glCompact/Texture2d.hpp"
#include "glCompact/gl/Constants.hpp"
//#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    Texture2d::Texture2d(
        SurfaceFormat surfaceFormat,
        uint32_t      x,
        uint32_t      y,
        bool          mipmaps
    ) {
        UNLIKELY_IF (x > getMaxXY() || y > getMaxXY())
            throw runtime_error("Trying to create Texture2d with size(x = " + to_string(x) + ", y = " + to_string(y) + "), but that is bayond getMaxXY(GL_MAX_TEXTURE_SIZE = " + to_string(threadContextGroup->values.GL_MAX_TEXTURE_SIZE) + ")");
        create(GL_TEXTURE_2D, surfaceFormat, x, y, 1, mipmaps, 0);
    }

    /*
        Returns maximum supported x and y size. Minimum supported value is 1024.
    */
    uint32_t Texture2d::getMaxXY() {
        return threadContextGroup->values.GL_MAX_TEXTURE_SIZE;
    }
}

/*Texture2d::Texture2d(ImageFormat imageFormat, Texture2d&           srcImages, int firstMipmap, bool mipmaps) {
    createView(srcImages, GL_TEXTURE_2D, imageFormat, firstMipmap, mipmaps, 0, 1);
}
Texture2d::Texture2d(ImageFormat imageFormat, Texture2dArray&      srcImages, int firstMipmap, bool mipmaps, int layer) {
    createView(srcImages, GL_TEXTURE_2D, imageFormat, firstMipmap, mipmaps, layer, 1);
}
Texture2d::Texture2d(ImageFormat imageFormat, TextureCubemap&      srcImages, int firstMipmap, bool mipmaps, int layer) {
    createView(srcImages, GL_TEXTURE_2D, imageFormat, firstMipmap, mipmaps, layer, 1);
}
Texture2d::Texture2d(ImageFormat imageFormat, TextureCubemapArray& srcImages, int firstMipmap, bool mipmaps, int layer) {
    createView(srcImages, GL_TEXTURE_2D, imageFormat, firstMipmap, mipmaps, layer, 1);
}*/
