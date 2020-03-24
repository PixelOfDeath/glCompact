#include "glCompact/ThreadContextGroupInfo.hpp"
#include "glCompact/ThreadContextGroup_.hpp"

namespace glCompact {
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
        thread_local const ContextGroupInfo*const& threadContextGroupInfo = threadContextGroup_;
    #else
                     const ContextGroupInfo*const& threadContextGroupInfo = threadContextGroup_;
    #endif
}
