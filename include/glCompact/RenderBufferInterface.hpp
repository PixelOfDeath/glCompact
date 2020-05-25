#pragma once
#include "glCompact/SurfaceInterface.hpp"

namespace glCompact {
    class RenderBufferInterface : public SurfaceInterface {
        protected:
            RenderBufferInterface() = default;
            RenderBufferInterface(const RenderBufferInterface& sourceRenderBuffer);
            RenderBufferInterface& operator=(RenderBufferInterface&& renderBufferInterface) = default;

            void create(SurfaceFormat surfaceFormat, uint32_t x, uint32_t y, uint32_t samples);

            static uint32_t getMaxXY();
    };
}
