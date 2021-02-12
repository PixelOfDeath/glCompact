#include <glCompact/AttributeLayout_.hpp>
#include <glCompact/minimumMaximum.hpp>

namespace glCompact {
    bool AttributeLayout_::operator==(const AttributeLayout_& rhs) const {
        if (AttributeLayout::operator!=(rhs)) return false;
        int8_t uppermostActiveLocation = maximum(uppermostActiveLocation, rhs.uppermostActiveLocation);
        for (int i = 0; i <= uppermostActiveLocation; ++i) if (gpuType[i] != rhs.gpuType[i]) return false;
        return true;
    }

    bool AttributeLayout_::operator!=(const AttributeLayout_& rhs) const {
        return !operator==(rhs);
    }
}
