#include "glCompact/threadContext.hpp"

namespace glCompact {
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT
        thread_local Context*      __restrict threadContext;
    #else
        static       char                     contextMem[sizeof(Context)] alignas(Context);
                     Context*const __restrict threadContext_ = reinterpret_cast<Context*>(&contextMem);
                     bool                     threadContextConstructed = false;
    #endif
}
