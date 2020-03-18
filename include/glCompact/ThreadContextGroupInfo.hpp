#pragma once
#include "glCompact/Config.hpp"
#include "glCompact/ContextGroupInfo.hpp"

namespace glCompact {
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
        extern thread_local const ContextGroupInfo*const& threadContextGroupInfo;
    #else
        extern              const ContextGroupInfo*const& threadContextGroupInfo;
    #endif
}
