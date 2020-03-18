#pragma once
#include "glCompact/BlendEquation.hpp"

namespace glCompact {
    struct BlendEquations {
        BlendEquation  rgb = BlendEquation::add;
        BlendEquation  a   = BlendEquation::add;
        bool operator ==(const BlendEquations& rhs) const {
            return (rgb == rhs.rgb) && (a == rhs.a);
        }
        bool operator !=(const BlendEquations& rhs) const {
            return !(*this == rhs);
        }
        BlendEquations operator =(const BlendEquations& rhs) {
            rgb = rhs.rgb;
            a   = rhs.a;
            return *this;
        }
    };
}
