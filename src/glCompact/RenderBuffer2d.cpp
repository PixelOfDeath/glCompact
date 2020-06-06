#include "glCompact/RenderBuffer2d.hpp"

#include <glm/glm.hpp>

using namespace glm;

namespace glCompact {
    RenderBuffer2d::RenderBuffer2d(
        SurfaceFormat surfaceFormat,
        uvec2         newSize
    ) {
        create(surfaceFormat, newSize, 0);
    }
}
