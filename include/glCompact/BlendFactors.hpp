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
