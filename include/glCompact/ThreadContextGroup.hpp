#pragma once
#include "glCompact/config.hpp"

namespace glCompact {
    class ContextGroup;
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
        extern thread_local ContextGroup*      __restrict threadContextGroup;
    #else
        extern              ContextGroup*const __restrict threadContextGroup;
    #endif
}
