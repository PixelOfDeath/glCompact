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
#include "glCompact/SurfaceSelector.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;

namespace glCompact {
    SurfaceSelector::SurfaceSelector() :
        surface    (0),
        mipmapLevel(0),
        layer      (-1)
    {
    }

    SurfaceSelector::SurfaceSelector(
        SurfaceInterface& surface,
        uint32_t          mipmapLevel
    ) :
        surface    (&surface),
        mipmapLevel(mipmapLevel),
        layer      (-1)
    {
        UNLIKELY_IF (!surface.id)
            throw runtime_error("Trying to use empty surface object for Frame!");
        UNLIKELY_IF (mipmapLevel > surface.mipmapCount - 1)
            throw out_of_range ("Trying to select surface mipmapLevel that does not exist!");
    }

    SurfaceSelector::SurfaceSelector(
        SurfaceInterface& surface,
        uint32_t          mipmapLevel,
        uint32_t          layer
    ) :
        surface    (&surface),
        mipmapLevel(mipmapLevel),
        layer      (layer)
    {
        UNLIKELY_IF (!surface.id)
            throw runtime_error   ("Trying to use empty surface object for Frame!");
        UNLIKELY_IF (mipmapLevel > surface.mipmapCount - 1)
            throw out_of_range    ("Trying to select surface mipmapLevel that does not exist!");
        UNLIKELY_IF (!surface.isLayered())
            throw invalid_argument("Trying to select layer of unlayered surface!");
        UNLIKELY_IF (layer       > surface.z)
            throw out_of_range    ("Trying to select surface layer that does not exist!");
    }
}
