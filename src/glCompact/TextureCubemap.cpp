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
#include "glCompact/TextureCubemap.hpp"
#include "glCompact/gl/Constants.hpp"
//#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    TextureCubemap::TextureCubemap(
        SurfaceFormat surfaceFormat,
        uint32_t      xy,
        bool          mipmaps
    ) {
        UNLIKELY_IF (xy > getMaxXY())
            throw runtime_error("Trying to create TextureCubemap with size(xy = " + to_string(xy) + "), but that is bayond getMaxXY(GL_MAX_CUBE_MAP_TEXTURE_SIZE = " + to_string(threadContextGroup->values.GL_MAX_CUBE_MAP_TEXTURE_SIZE) + ")");
        create(GL_TEXTURE_CUBE_MAP, surfaceFormat, xy, xy, 1, mipmaps, 0);
    }

    /*
        Returns maximum supported xy size for a cubemap texture. Minimum supported value is 1024.
    */
    uint32_t TextureCubemap::getMaxXY() {
        return threadContextGroup->values.GL_MAX_CUBE_MAP_TEXTURE_SIZE;
    }
}

/*TextureCubemap::TextureCubemap(ImageFormat imageFormat, Texture2dArray&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_3D, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}
TextureCubemap::TextureCubemap(ImageFormat imageFormat, TextureCubemap&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_3D, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}
TextureCubemap::TextureCubemap(ImageFormat imageFormat, TextureCubemapArray& srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_3D, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}*/
