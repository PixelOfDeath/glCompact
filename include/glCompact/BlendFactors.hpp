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
#include "glCompact/BlendFactorRgb.hpp"
#include "glCompact/BlendFactorA.hpp"

namespace glCompact {
    struct BlendFactors {
        BlendFactorRgb srcRgb = BlendFactorRgb::one;
        BlendFactorA   srcA   = BlendFactorA  ::one;
        BlendFactorRgb dstRgb = BlendFactorRgb::zero;
        BlendFactorA   dstA   = BlendFactorA  ::zero;
        bool operator ==(const BlendFactors& rhs) const {
            return (srcRgb == rhs.srcRgb) && (srcA == rhs.srcA) && (dstRgb == rhs.dstRgb) && (dstA == rhs.dstA);
        }
        bool operator !=(const BlendFactors& rhs) const {
            return !(*this == rhs);
        }
        BlendFactors operator =(const BlendFactors& rhs) {
            srcRgb = rhs.srcRgb;
            srcA   = rhs.srcA;
            dstRgb = rhs.dstRgb;
            dstA   = rhs.dstA;
            return *this;
        }
    };
}
