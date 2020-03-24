#include "glCompact/ThreadContextGroup_.hpp"

namespace glCompact {
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
        thread_local ContextGroup_*      __restrict threadContextGroup_;
    #else
        static       char                           contextGroupMem[sizeof(ContextGroup_)] alignas(ContextGroup_);
                     ContextGroup_*const __restrict threadContextGroup_ = reinterpret_cast<ContextGroup_*>(&contextGroupMem);
                     bool                           threadContextGroupConstructed_ = false;
    #endif
}
