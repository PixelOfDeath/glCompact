#pragma once
#include "glCompact/config.hpp"

namespace glCompact {
    class Context;
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT
        extern thread_local Context*      __restrict threadContext;
    #else
        extern              Context*const __restrict threadContext;
        extern              bool                     threadContextConstructed;
    #endif
};
