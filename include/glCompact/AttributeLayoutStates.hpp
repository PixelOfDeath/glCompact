#pragma once
#include "glCompact/AttributeLayout.hpp"

namespace glCompact {
    class AttributeLayoutStates {
        public:
            enum class Usage : uint8_t {
                disabled,
                enabled,
                indifferent
            };
            enum class GpuType : uint8_t {
                unknown,
                f32,
                i32, //used for all integers and bool
                f64
            };

            int8_t uppermostActiveBufferIndex = -1;
            int8_t uppermostActiveLocation    = -1;

            uint32_t bufferIndexStride[config::MAX_ATTRIBUTES] = {}; //Must be uint32_t because this is an ABI parameter list for OpenGL
            bool     instancing       [config::MAX_ATTRIBUTES] = {}; //With GL_ARB_vertex_attrib_binding support this is per attribute location. Without it is per buffer index.
            struct Location {
                AttributeFormat attributeFormat = AttributeFormat::NONE;
                uint16_t        offset          = 0;
                uint8_t         bufferIndex     = 0;
                Usage           usage           = Usage::indifferent;
                GpuType         gpuType         = GpuType::unknown;
            } location[config::MAX_ATTRIBUTES];
    };
}
