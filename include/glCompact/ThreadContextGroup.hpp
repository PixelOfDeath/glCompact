#pragma once
#include "glCompact/Config.hpp"
#include "glCompact/ContextGroup.hpp"

namespace glCompact {
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
        extern thread_local const ContextGroup*const& threadContextGroup;
    #else
        extern              const ContextGroup*const& threadContextGroup;
    #endif
}
