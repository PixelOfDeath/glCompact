#pragma once
#include "glCompact/RenderBufferInterface.hpp"
#include "glCompact/SurfaceFormat.hpp"

namespace glCompact {
    class RenderBuffer2d : public RenderBufferInterface {
        public:
            RenderBuffer2d(SurfaceFormat surfaceFormat, uint32_t x, uint32_t y);
            RenderBuffer2d(const RenderBuffer2d& sourceRenderBuffer) = default;
            RenderBuffer2d& operator=(RenderBuffer2d&& renderBuffer2d) = default;
    };
}
