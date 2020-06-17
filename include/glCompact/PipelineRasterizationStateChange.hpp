#pragma once
#include <cstdint> //C++11

//Note: BitFiels (and union) from the view of the C++ standard bring a lot of undefined behavior
//It just so happens that all compilers that I target do behave like I want to use them.

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
            };
        };
    };
}
