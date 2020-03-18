#pragma once
#include "glCompact/SurfaceInterface.hpp"

namespace glCompact {
    class RenderBufferInterface : public SurfaceInterface {
        protected:
            RenderBufferInterface() = default;
            RenderBufferInterface& operator=(RenderBufferInterface&& renderBufferInterface) = default;
    };
}
