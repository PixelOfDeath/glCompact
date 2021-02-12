#pragma once
#include <glCompact/AttributeLayout.hpp>

namespace glCompact {
    class AttributeLayout_ : public AttributeLayout {
        friend class PipelineRasterization;

        enum class GpuType : uint8_t {
            unused,
            f32,
            i32, //used for all integers and bool
            f64
        };
        GpuType gpuType[config::MAX_ATTRIBUTES] = {};

        bool operator==(const AttributeLayout_& rhs) const;
        bool operator!=(const AttributeLayout_& rhs) const;
    };
}
