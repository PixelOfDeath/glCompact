#pragma once
#include "glCompact/Version.hpp"
#include "glCompact/Feature.hpp"

namespace glCompact {
    class ContextGroup_;
    class ContextGroup {
        public:
            ContextGroup(const ContextGroup_* contextGroup_);
            const Version version;
            const Feature feature;
    };
}
