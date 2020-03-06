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
#include "glCompact/TextureSelector.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;

namespace glCompact {
    TextureSelector::TextureSelector() :
        texture    (0),
        mipmapLevel(0),
        layer      (-1)
    {
    }

    TextureSelector::TextureSelector(
        TextureInterface& texture,
        uint32_t          mipmapLevel
    ) :
        texture    (&texture),
        mipmapLevel(mipmapLevel),
        layer      (-1)
    {
        UNLIKELY_IF (!texture.id)
            throw runtime_error("Trying to use empty texture object for Frame!");
        UNLIKELY_IF (mipmapLevel > texture.mipmapCount - 1)
            throw out_of_range ("Trying to select texture mipmapLevel that does not exist!");
    }

    TextureSelector::TextureSelector(
        TextureInterface& texture,
        uint32_t          mipmapLevel,
        uint32_t          layer
    ) :
        texture    (&texture),
        mipmapLevel(mipmapLevel),
        layer      (layer)
    {
        UNLIKELY_IF (!texture.id)
            throw runtime_error   ("Trying to use empty texture object for Frame!");
        UNLIKELY_IF (mipmapLevel > texture.mipmapCount - 1)
            throw out_of_range    ("Trying to select texture mipmapLevel that does not exist!");
        UNLIKELY_IF (!texture.isLayered())
            throw invalid_argument("Trying to select layer of unlayered texture!");
        UNLIKELY_IF (layer       > texture.z)
            throw out_of_range    ("Trying to select texture layer that does not exist!");
    }
}
