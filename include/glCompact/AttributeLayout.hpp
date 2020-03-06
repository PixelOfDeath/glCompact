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
#include "glCompact/Config.hpp"
#include "glCompact/AttributeFormat.hpp"

namespace glCompact {
    class AttributeLayout {
            friend class PipelineRasterization;
        public:
            AttributeLayout();
            ~AttributeLayout();

            void addBufferIndex();
            void addBufferIndexWithInstancing();
            void addLocation(uint8_t location, AttributeFormat attributeFormat);
            void addSpacing(uint16_t byteSize);
            void addSpacing(AttributeFormat attributeFormat);

            void reset();
        private:
            int8_t uppermostActiveBufferIndex = -1;
            int8_t uppermostActiveLocation    = -1;

            struct BufferIndex {
                uint16_t stride     = 0;
                bool     instancing = 0;
            } bufferIndex[Config::MAX_ATTRIBUTES];
            struct Location {
                AttributeFormat attributeFormat = AttributeFormat::NONE; //AttributeFormat::none = location disabled
                uint16_t        offset          = 0;
                uint8_t         bufferIndex     = 0;
            } location[Config::MAX_ATTRIBUTES];

            void addBufferIndex_(bool instancing);
    };
}
