#pragma once
#include <cstdint> //C++11
#include "glCompact/Config.hpp"

namespace glCompact {
    class ContextGroupInfo {
        public:
            struct {
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
            } version;

            struct {
                bool drawIndirect               = Config::glEqualOrGreater(4, 0) || Config::FeatureSetting::mustBeSupported == Config::drawIndirect;
                bool blendModePerDrawbuffer     = Config::glEqualOrGreater(4, 0) || Config::FeatureSetting::mustBeSupported == Config::blendModePerDrawbuffer;
                bool drawBaseInstance           = Config::glEqualOrGreater(4, 2) || Config::FeatureSetting::mustBeSupported == Config::drawBaseInstance;
                bool bptc                       = Config::glEqualOrGreater(4, 2) || Config::FeatureSetting::mustBeSupported == Config::bptc;
                bool atomicCounter              = Config::glEqualOrGreater(4, 2) || Config::FeatureSetting::mustBeSupported == Config::atomicCounter;
                bool shaderStorageBufferObject  = Config::glEqualOrGreater(4, 3) || Config::FeatureSetting::mustBeSupported == Config::shaderStorageBufferObject;
                bool astc                       = Config::glEqualOrGreater(4, 3) || Config::FeatureSetting::mustBeSupported == Config::astc;
                bool drawIndirectCount          = Config::glEqualOrGreater(4, 6) || Config::FeatureSetting::mustBeSupported == Config::drawIndirectCount;
                bool polygonOffsetClamp         = Config::glEqualOrGreater(4, 6) || Config::FeatureSetting::mustBeSupported == Config::polygonOffsetClamp;
                bool anisotropicFilter          = Config::glEqualOrGreater(4, 6) || Config::FeatureSetting::mustBeSupported == Config::anisotropicFilter;
                bool spirv                      =                                   Config::FeatureSetting::mustBeSupported == Config::spirv;
            } feature;
    };
}
