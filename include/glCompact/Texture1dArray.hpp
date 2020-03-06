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
#pragma once
#include "glCompact/TextureInterface.hpp"

namespace glCompact {
    class Texture1dArray : public TextureInterface {
        public:
            Texture1dArray(SurfaceFormat surfaceFormat, uint32_t x, uint32_t yLayers, bool mipmaps);
            Texture1dArray& operator=(Texture1dArray&& texture1dArray) = default;
            //Texture1dArray(ImageFormat imageFormat, Texture1d&      srcImages, int firstMipmap, bool mipmaps);
            //Texture1dArray(ImageFormat imageFormat, Texture1dArray& srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount);

            static uint32_t getMaxX();
            static uint32_t getMaxLayers();
    };
}
