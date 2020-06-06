#pragma once
#include "glCompact/RenderBufferInterface.hpp"
#include "glCompact/SurfaceFormat.hpp"

#include <glm/vec2.hpp>

namespace glCompact {
    class RenderBuffer2d : public RenderBufferInterface {
        public:
            RenderBuffer2d(SurfaceFormat surfaceFormat, glm::uvec2 newSize);
            RenderBuffer2d(           const RenderBuffer2d&  renderBuffer2d) = default;
            RenderBuffer2d(                 RenderBuffer2d&& renderBuffer2d) = default;
            RenderBuffer2d& operator=(const RenderBuffer2d&  renderBuffer2d) = default;
            RenderBuffer2d& operator=(      RenderBuffer2d&& renderBuffer2d) = default;
    };
}
