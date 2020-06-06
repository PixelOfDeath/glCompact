#pragma once
#include "glCompact/RenderBufferInterface.hpp"
#include "glCompact/SurfaceFormat.hpp"

#include <glm/vec2.hpp>

namespace glCompact {
    class RenderBuffer2dMultisample : public RenderBufferInterface {
        public:
            RenderBuffer2dMultisample(SurfaceFormat surfaceFormat, glm::uvec2 newSize, uint32_t samples);
            RenderBuffer2dMultisample(const RenderBuffer2dMultisample& sourceRenderBuffer) = default;
            RenderBuffer2dMultisample& operator=(RenderBuffer2dMultisample&& renderBufferMultisample) = default;

            static uint32_t getMaxSamples();
            static uint32_t getMaxSamplesRgbaInteger();
    };
}
