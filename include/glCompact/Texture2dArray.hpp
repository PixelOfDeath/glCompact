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
#pragma once
#include "glCompact/TextureInterface.hpp"

namespace glCompact {
    class Texture2dArray : public TextureInterface {
        public:
            Texture2dArray(SurfaceFormat surfaceFormat, uint32_t x, uint32_t y, uint32_t layers, bool mipmaps);
            Texture2dArray& operator=(Texture2dArray&& texture2dArray) = default;
            //Texture2dArray(ImageFormat imageFormat, Texture2d&           srcImages, int firstMipmap, bool mipmaps);
            //Texture2dArray(ImageFormat imageFormat, Texture2dArray&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount);
            //Texture2dArray(ImageFormat imageFormat, TextureCubemap&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount);
            //Texture2dArray(ImageFormat imageFormat, TextureCubemapArray& srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount);

            static uint32_t getMaxXY();
            static uint32_t getMaxLayers();
    };
}

