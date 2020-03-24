#pragma once
#include "glCompact/ContextGroupInfo.hpp"
#include "glCompact/gl/Functions.hpp"
#include "glCompact/gl/Values.hpp"
#include "glCompact/gl/Extensions.hpp"

#include <atomic>

namespace glCompact {
    class ContextGroup_ : public ContextGroupInfo {
        public:
            ContextGroup_(void*(*getGlFunctionPointer)(const char* glFunctionName));

            gl::Functions  functions;
            gl::Values     values;
            gl::Extensions extensions;

            void setAllCoreExtensionTrue();
            void getAllValue();
            void checkAndOrSetFeatures();

            std::atomic<int> contextCount = {1};
    };
}
