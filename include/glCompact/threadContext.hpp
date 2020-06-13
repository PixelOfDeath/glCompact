#pragma once
#include "glCompact/config.hpp"
#include "glCompact/Context.hpp"

namespace glCompact {
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT
        extern thread_local Context*      __restrict threadContext;
    #else
        extern              Context*const __restrict threadContext;
        extern              bool                     threadContextConstructed;
    #endif
};
