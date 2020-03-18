#include "glCompact/ThreadContextGroup.hpp"

namespace glCompact {
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
        thread_local ContextGroup*      __restrict threadContextGroup;
    #else
        static       char                          contextGroupMem[sizeof(ContextGroup)] alignas(ContextGroup);
                     ContextGroup*const __restrict threadContextGroup = reinterpret_cast<ContextGroup*>(&contextGroupMem);
                     bool                          threadContextGroupConstructed = false;
    #endif
}
