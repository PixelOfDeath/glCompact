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
#include "glCompact/SurfaceInterface.hpp"

namespace glCompact {
    class SurfaceSelector {
        ///\cond HIDDEN_FROM_DOXYGEN
            friend class Frame;
        ///\endcond
        public:
            SurfaceSelector();
            SurfaceSelector(SurfaceInterface& surface, uint32_t mipmapLevel = 0);
            SurfaceSelector(SurfaceInterface& surface, uint32_t mipmapLevel    , uint32_t layer);
            //TODO if we stay with non virtual interface class this can be specialized to only take layer parameter for texture classes with layers
        ///\cond HIDDEN_FROM_DOXYGEN
        private:
            SurfaceInterface*const surface;
            const uint32_t mipmapLevel;
            const  int32_t layer;
        ///\endcond
    };
}
