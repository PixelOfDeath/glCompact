#pragma once
#include "glCompact/config.hpp"

#include <cstdint> //C++11
#include <string>

namespace glCompact {
    struct Version {
        std::string string;
        int32_t major;
        int32_t minor;

        bool    es;
        bool    core;
        bool    forwardCompatible;
        bool    debug;
        bool    robustAccess;
        //bool    resetIsolation; //GL_ARB_robustness_isolation (Core since 4.3)
        bool    noErrorReporting;

        constexpr bool equalOrGreater(int ma, int mi) const {
            return config::glEqualOrGreater(ma, mi) || (major > ma) || ((major == ma) && (minor >= mi));
        };
    };
}
