#pragma once
#include "glCompact/ContextGroup.hpp"
#include "glCompact/Version.hpp"
#include "glCompact/Feature.hpp"
#include "glCompact/gl/Functions.hpp"
#include "glCompact/gl/Values.hpp"
#include "glCompact/gl/Extensions.hpp"

#include <atomic>

namespace glCompact {
    class ContextGroup_ {
        public:
            ContextGroup_(void*(*getGlFunctionPointer)(const char* glFunctionName));

            Version        version;
            Feature        feature;
            gl::Functions  functions;
            gl::Extensions extensions;
            gl::Values     values;

            void setAllCoreExtensionTrue();
            void getAllValue();
            void checkAndSetFeatures();

            std::atomic<int> contextCount = {1};
    };
}
