#include "glCompact/RenderBuffer2d.hpp"

namespace glCompact {
    RenderBuffer2d::RenderBuffer2d(
        SurfaceFormat surfaceFormat,
        uint32_t      x,
        uint32_t      y
    ) {
        create(surfaceFormat, x, y, 0);
    }
}
