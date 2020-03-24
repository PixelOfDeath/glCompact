#include "glCompact/ThreadContextGroup.hpp"
#include "glCompact/ThreadContextGroup_.hpp"

namespace glCompact {
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
        thread_local const ContextGroup*const& threadContextGroup = threadContextGroup_;
    #else
                     const ContextGroup*const& threadContextGroup = threadContextGroup_;
    #endif
}
