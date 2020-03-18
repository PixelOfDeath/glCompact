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
