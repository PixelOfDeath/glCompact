#include "glCompact/PipelineRasterizationStateChange.hpp"
#include "glCompact/MacroDefineBitOperatorsForType.hpp"

namespace glCompact {
    enum class PipelineRasterizationStateChange : uint32_t {
        none            = 0,
        viewportScissor = 1 << 0,
        triangleFace    = 1 << 1,
        rgbaMask        = 1 << 2,
        stencil         = 1 << 3,
        depth           = 1 << 4,
        blend           = 1 << 5,
        attributeLayout = 1 << 6,
        all             = ~uint32_t(0)
    };
    DEFINE_BIT_OPERATORS_FOR_TYPE_OUTSIDE_CLASS(PipelineRasterizationStateChange)
}
