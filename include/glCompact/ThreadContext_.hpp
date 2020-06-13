#pragma once
#include "glCompact/config.hpp"

namespace glCompact {
    class Context_;
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT
        extern thread_local Context_*      __restrict threadContext_;
    #else
        extern              Context_*const __restrict threadContext_;
        extern              bool                      threadContextConstructed;
    #endif
};
