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
    class TextureSelector {
            friend class Frame;
            friend class PipelineInterface;
        public:
            //TODO check for valid input param
            TextureSelector();
            TextureSelector(TextureInterface& texture, uint32_t mipmapLevel = 0);
            TextureSelector(TextureInterface& texture, uint32_t mipmapLevel    , uint32_t layer);
        private:
            TextureInterface*const texture;
            const uint32_t mipmapLevel;
            const uint32_t layer;
    };
}
