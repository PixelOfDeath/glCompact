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
#include "glCompact/AttributeLayout.hpp"

namespace glCompact {
    class AttributeLayoutStates {
        public:
            enum class Usage : uint8_t {
                disabled,
                enabled,
                indifferent
            };
            enum class GpuType : uint8_t {
                unknown,
                f32,
                i32, //used for all integers and bool
                f64
            };

            int8_t uppermostActiveBufferIndex = -1;
            int8_t uppermostActiveLocation    = -1;

            uint32_t bufferIndexStride[Config::MAX_ATTRIBUTES] = {}; //Must be uint32_t because this is an ABI parameter list for OpenGL
            bool     instancing       [Config::MAX_ATTRIBUTES] = {}; //With GL_ARB_vertex_attrib_binding support this is per attribute location. Without it is per buffer index.
            struct Location {
                AttributeFormat attributeFormat = AttributeFormat::NONE;
                uint16_t        offset          = 0;
                uint8_t         bufferIndex     = 0;
                Usage           usage           = Usage::indifferent;
                GpuType         gpuType         = GpuType::unknown;
            } location[Config::MAX_ATTRIBUTES];
    };
}
