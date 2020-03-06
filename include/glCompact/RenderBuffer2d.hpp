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
#include "glCompact/RenderBufferInterface.hpp"
#include "glCompact/SurfaceFormat.hpp"

namespace glCompact {
    class RenderBuffer2d : public RenderBufferInterface {
        public:
            RenderBuffer2d(SurfaceFormat surfaceFormat, uint32_t x, uint32_t y);
            RenderBuffer2d& operator=(RenderBuffer2d&& renderBuffer2d) = default;

            static uint32_t getMaxXY();
    };
}
