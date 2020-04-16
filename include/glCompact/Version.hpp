#pragma once
#include "glCompact/config.hpp"

#include <cstdint> //C++11
#include <string>

namespace glCompact {
    struct Version {
        std::string versionString;
        std::string shadingLanguageVersionString;

        GlVersion   gl;
        GlesVersion gles;

        bool    core                = false;
        bool    forwardCompatible   = false;
        bool    debug               = false;
        bool    robustAccess        = false;
      //bool    resetIsolation      = false; //GL_ARB_robustness_isolation (Core since 4.3)
        bool    noErrorReporting    = false;
    };
}
