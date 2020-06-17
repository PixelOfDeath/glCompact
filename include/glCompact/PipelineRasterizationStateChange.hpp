#pragma once
#include <cstdint> //C++11

namespace glCompact {
    struct PipelineRasterizationStateChange {
        union {
            uint8_t all = 0;
            struct {
                bool
                    viewportScissor:1,
                    triangleFace:1,
                    rgbaMask:1,
                    stencil:1,
                    depth:1,
                    blend:1,
                    attributeLayout:1;
            }
        };
    };
}
