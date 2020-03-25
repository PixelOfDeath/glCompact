#pragma once
#include "glCompact/Config.hpp"
#include "glCompact/ContextGroup.hpp"

namespace glCompact {
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
        extern thread_local ContextGroup*      __restrict threadContextGroup;
    #else
        extern              ContextGroup*const __restrict threadContextGroup;
    #endif
}
