#pragma once
#include "glCompact/config.hpp"

namespace glCompact {
    class Context_;
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT
        extern thread_local Context_*      __restrict threadContext;
    #else
        extern              Context_*const __restrict threadContext;
        extern              bool                      threadContextConstructed;
    #endif
};
