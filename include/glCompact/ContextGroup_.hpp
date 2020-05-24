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
        private:
            template<typename T>
            T getValue(int32_t pname);
            template<typename T>
            T getValue(int32_t pname, uint32_t index);
            template<typename T>
            inline T versionValue(T gl46, T gl45, T gl44, T gl43, T gl42, T gl41, T gl40, T gl33, T gl32, T gl31, T gl30, T gl21,  T gles32, T gles31, T gles30, T gles20, uint32_t glConstName
            );
    };
}
