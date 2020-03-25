#pragma once
#include "glCompact/Config.hpp"
#include <cstdint> //C++11

namespace glCompact {
    struct Version {
        int32_t major;
        int32_t minor;

        bool    core;
        bool    forwardCompatible;
        bool    debug;
        bool    robustAccess;
        //bool    resetIsolation; //GL_ARB_robustness_isolation (Core since 4.3)
        bool    noErrorReporting;

        constexpr bool equalOrGreater(int ma, int mi) const {
            return Config::glEqualOrGreater(ma, mi) || (major > ma) || ((major == ma) && (minor >= mi));
        };
    };
}
