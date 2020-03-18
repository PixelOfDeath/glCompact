#include "glCompact/ThreadContextGroupInfo.hpp"
#include "glCompact/ThreadContextGroup.hpp"

namespace glCompact {
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
        thread_local const ContextGroupInfo*const& threadContextGroupInfo = threadContextGroup;
    #else
                     const ContextGroupInfo*const& threadContextGroupInfo = threadContextGroup;
    #endif
}
