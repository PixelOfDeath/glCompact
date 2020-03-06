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
    class Texture2d : public TextureInterface {
        public:
            Texture2d(SurfaceFormat surfaceFormat, uint32_t x, uint32_t y, bool mipmaps);
            Texture2d& operator=(Texture2d&& texture2d) = default;
            //Texture2d(ImageFormat imageFormat, Texture2d&           srcImages, int firstMipmap, bool mipmaps);
            //Texture2d(ImageFormat imageFormat, Texture2dArray&      srcImages, int firstMipmap, bool mipmaps, int layer);
            //Texture2d(ImageFormat imageFormat, TextureCubemap&      srcImages, int firstMipmap, bool mipmaps, int layer);
            //Texture2d(ImageFormat imageFormat, TextureCubemapArray& srcImages, int firstMipmap, bool mipmaps, int layer);

            static uint32_t getMaxXY();
    };
}
