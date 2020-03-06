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
#include "glCompact/TextureCubemapArray.hpp"
#include "glCompact/gl/Constants.hpp"
//#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    //This kind of texture depends on GL_ARB_texture_cube_map_array (Core since 4.0)
    TextureCubemapArray::TextureCubemapArray(
        SurfaceFormat surfaceFormat,
        uint32_t      xy,
        uint32_t      cubeMapCount,
        bool          mipmaps
    ) {
        UNLIKELY_IF (!threadContextGroup->extensions.GL_ARB_texture_cube_map_array)
            throw std::runtime_error("Missing GL_ARB_Texture_cube_map_array to create Texture!");
        UNLIKELY_IF (xy > getMaxXY())
            throw runtime_error("Trying to create TextureCubemapArray with size(xy = " + to_string(xy) + "), but that is bayond getMaxXY(GL_MAX_CUBE_MAP_TEXTURE_SIZE = " + to_string(threadContextGroup->values.GL_MAX_CUBE_MAP_TEXTURE_SIZE) + ")");
        UNLIKELY_IF (6 * cubeMapCount > getMaxLayers())
            throw runtime_error("Trying to create TextureCubemapArray with size(cubeMapCount * 6 = layers = " + to_string(6 * cubeMapCount) + "), but that is bayond getMaxLayers(GL_MAX_ARRAY_TEXTURE_LAYERS = " + to_string(threadContextGroup->values.GL_MAX_ARRAY_TEXTURE_LAYERS) + ")");
        TextureInterface::create(GL_TEXTURE_CUBE_MAP_ARRAY, surfaceFormat, xy, xy, 6 * cubeMapCount, mipmaps, 0);
    }

    /*
        Returns maximum supported xy size for a cubemap texture. Minimum supported value is 1024.
    */
    uint32_t TextureCubemapArray::getMaxXY() {
        return threadContextGroup->values.GL_MAX_CUBE_MAP_TEXTURE_SIZE;
    }

    /*
        Returns maximum supported layer count. Minimum supported value is 256.
    */
    uint32_t TextureCubemapArray::getMaxLayers() {
        return threadContextGroup->values.GL_MAX_ARRAY_TEXTURE_LAYERS;
    }
}
/*TextureCubemapArray::TextureCubemapArray(ImageFormat imageFormat, Texture2dArray&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_3D, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}
TextureCubemapArray::TextureCubemapArray(ImageFormat imageFormat, TextureCubemap&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_3D, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}
TextureCubemapArray::TextureCubemapArray(ImageFormat imageFormat, TextureCubemapArray& srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_3D, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}*/
