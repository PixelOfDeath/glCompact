#pragma once
#include "glCompact/Config.hpp"
#include "glCompact/gl/Types.hpp"

namespace glCompact {
    namespace gl {
        struct Version {
            public:
                int32_t major             = 0;
                int32_t minor             = 0;
                bool    core              = false;
                bool    forwardCompatible = false;
                bool    debug             = false;
                bool    robustAccess      = false;
              //bool    resetIsolation    = false; //GL_ARB_robustness_isolation (Core since 4.3)
                bool    noErrorReporting  = false;

                constexpr bool equalOrGreater(int ma, int mi) const {
                    return Config::glEqualOrGreater(ma, mi) || (major > ma) || ((major == ma) && (minor >= mi));
                };
        };
    }
}
