#pragma once
#include "glCompact/ContextGroupInfo.hpp"
#include "glCompact/gl/Functions.hpp"
#include "glCompact/gl/Values.hpp"
#include "glCompact/gl/Extensions.hpp"

namespace glCompact {
    class ContextGroup : public ContextGroupInfo {
        public:
            ContextGroup(void*(*getGlFunctionPointer)(const char* glFunctionName));

            gl::Functions  functions;
            gl::Values     values;
            gl::Extensions extensions;

            void setAllCoreExtensionTrue();
            void getAllValue();
            void checkAndOrSetFeatures();

            int contextCount = 1;
    };
}
