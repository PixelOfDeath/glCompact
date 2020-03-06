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
#include "glCompact/RenderBufferInterface.hpp"
#include "glCompact/SurfaceFormat.hpp"

namespace glCompact {
    class RenderBuffer2dMultisample : public RenderBufferInterface {
        public:
            RenderBuffer2dMultisample(SurfaceFormat surfaceFormat, uint32_t x, uint32_t y, uint32_t samples);
            RenderBuffer2dMultisample& operator=(RenderBuffer2dMultisample&& renderBufferMultisample) = default;

            static uint32_t getMaxXY();
            static uint32_t getMaxSamples();
            static uint32_t getMaxSamplesRgbaInteger();
    };
}
