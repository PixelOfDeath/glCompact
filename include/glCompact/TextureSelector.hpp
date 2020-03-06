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
    class TextureSelector {
        ///\cond HIDDEN_FROM_DOXYGEN
            friend class Frame;
            friend class PipelineInterface;
        ///\endcond
        public:
            //TODO check for valid input param
            TextureSelector();
            TextureSelector(TextureInterface& texture, uint32_t mipmapLevel = 0);
            TextureSelector(TextureInterface& texture, uint32_t mipmapLevel    , uint32_t layer);
        ///\cond HIDDEN_FROM_DOXYGEN
        private:
            TextureInterface*const texture;
            const uint32_t mipmapLevel;
            const uint32_t layer;
        ///\endcond
    };
}
