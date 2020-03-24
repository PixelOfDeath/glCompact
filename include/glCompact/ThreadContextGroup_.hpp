#pragma once
#include "glCompact/Config.hpp"
#include "glCompact/ContextGroup_.hpp"

namespace glCompact {
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
        extern thread_local ContextGroup_*      __restrict threadContextGroup;
    #else
        extern              ContextGroup_*const __restrict threadContextGroup_;
        extern              bool                           threadContextGroupConstructed_;
    #endif
}
