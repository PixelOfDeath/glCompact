#include "glCompact/ContextGroup.hpp"
#include "glCompact/ContextGroup_.hpp"

namespace glCompact {
    ContextGroup::ContextGroup(
        const ContextGroup_* contextGroup_
    ) : version(contextGroup_->version), feature(contextGroup_->feature){}
}
