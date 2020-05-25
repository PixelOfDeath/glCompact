#pragma once
#include "glCompact/RenderBufferInterface.hpp"
#include "glCompact/SurfaceFormat.hpp"

namespace glCompact {
    class RenderBuffer2dMultisample : public RenderBufferInterface {
        public:
            RenderBuffer2dMultisample(SurfaceFormat surfaceFormat, uint32_t x, uint32_t y, uint32_t samples);
            RenderBuffer2dMultisample& operator=(RenderBuffer2dMultisample&& renderBufferMultisample) = default;

            static uint32_t getMaxSamples();
            static uint32_t getMaxSamplesRgbaInteger();
    };
}
