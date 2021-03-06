#pragma once
#include "glCompact/SurfaceInterface.hpp"

#include <glm/vec2.hpp>

namespace glCompact {
    class RenderBufferInterface : public SurfaceInterface {
        protected:
            RenderBufferInterface() = default;
            RenderBufferInterface(           const RenderBufferInterface&  renderBufferInterface);
            RenderBufferInterface(                 RenderBufferInterface&& renderBufferInterface) = default;
            RenderBufferInterface& operator=(const RenderBufferInterface&  renderBufferInterface);
            RenderBufferInterface& operator=(      RenderBufferInterface&& renderBufferInterface) = default;

            void create(SurfaceFormat surfaceFormat, glm::uvec2 newSize, uint32_t samples);

            static uint32_t getMaxXY();
    };
}
