#include "glCompact/ThreadContext.hpp"
#include "glCompact/Context.hpp"

/*TODO
    Big issue still to be solved:

    In the case of a multi thread/multi context application,
    if a thread has thread-global glCompact objects and gets destroyed
    the destructors call OpenGL functions to delete the underlaying GL objects.
    If the context is already destroyed then the calls are silently ignored (Defined behavior by the OpenGL standard!)
    And we end up with memory leaks in form of left over gl objects!

    A: In debug mode will check if thread has active context in every function that needs one. Otherwise we can cause an error or warning!
*/

namespace glCompact {
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT
        thread_local Context*      __restrict threadContext;
    #else
        static       char                     contextMem[sizeof(Context)] alignas(Context);
                     Context*const __restrict threadContext = reinterpret_cast<Context*>(&contextMem);
                     bool                     threadContextConstructed = false;
    #endif
}
