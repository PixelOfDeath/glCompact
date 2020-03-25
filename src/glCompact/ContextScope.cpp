#include "glCompact/ContextScope.hpp"
#include "glCompact/Context.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/ToolsInternal.hpp"
#include "glCompact/Config.hpp"
#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/ThreadContextGroup.hpp"

namespace glCompact {
    /**
        \ingroup API
        \class glCompact::ContextScope
        \brief Scope object that initalizes the glCompact internals

        \details Before using any other glCompact functionality in this thread/context, this object needs to be created.

        The current thread needs to have an active OpenGL context with a minimum version of 3.3.

        There are two constructors that take a function pointer to a function that itself returns a OpenGL function pointer from a c-string name parameter.

        Most Windows and OpenGL context creation libraries supply such a function.

        The type can be:

            void *(*)(const char*)

        for SDL2

            ContextScope myContextScope(SDL_GL_GetProcAddress)

        or

            void(*(*)(const char*))()

        for GLFW

            ContextScope myContextScope(glfwGetProcAddress)

        for SFML

            ContextScope myContextScope(sf::Context::getFunction)

        Otherwise a type cast to one of this types is neccessary.


        If there is another OpenGL thread/context that already has a ContextScope object and shares states with this thread/context,
        then there is a constructor that takes a pointer to the ContextScope object.
    */

    static void checkContext() {
        #ifdef GLCOMPACT_MULTIPLE_CONTEXT
            UNLIKELY_IF (threadContext != 0)
                crash("This thread already has a glCompact::Context object registered!");
        #else
            UNLIKELY_IF (threadContextConstructed)
                crash("A glCompact::Context object is already registered!\nNote: Config.hpp GLCOMPACT_MULTIPLE_CONTEXT is not defined! Define it if you want to use more then one context!");
        #endif
    }

    static void checkContextGroup() {
        #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
            UNLIKELY_IF (threadContextGroup_ != 0)
                crash("This thread already has a glCompact::ContextGroup_ object registered!");
        #else
            UNLIKELY_IF (threadContextGroupConstructed_)
                crash("A glCompact::ContextGroup object is already registered!\nNote: Config.hpp GLCOMPACT_MULTIPLE_CONTEXT_GROUP is not defined! Define it if you want to use more then one shared ContextGroup!");
        #endif
    }

    /**
     * \brief Constructor
     *
     * \param getGlFunctionPointer pointer to function that returns OpenGL functions pointers via c-string name
     */
    ContextScope::ContextScope(
        void*(*getGlFunctionPointer)(const char*)
    ) {
        checkContext();
        checkContextGroup();

        #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
            contextGroup_ = threadContextGroup_ = new ContextGroup_(getGlFunctionPointer);
            contextGroup  = threadContextGroup  = new ContextGroup (contextGroup_);
        #else
            contextGroup_ = new (threadContextGroup_)ContextGroup_(getGlFunctionPointer);
            contextGroup  = new (threadContextGroup )ContextGroup (contextGroup_);
            threadContextGroupConstructed_ = true;
        #endif

        #ifdef GLCOMPACT_MULTIPLE_CONTEXT
            threadContext = new Context;
        #else
            new (threadContext)Context;
            threadContextConstructed = true;
        #endif
        threadContext->isMainContext = true;
    }

    /**
     * \brief Constructor
     *
     * \param getGlFunctionPointer pointer to function that returns OpenGL functions pointers via c-string name
     */
    ContextScope::ContextScope(
        void(*(*getGlFunctionPointer)(const char*))()
    ) : ContextScope(reinterpret_cast<void*(*)(const char*)>(getGlFunctionPointer)){}

    /**
     * \brief Constructor
     *
     * \param contextScope already initalized ContextScope object that was initalized on a OpenGL context that is shared with the current context
     */
    ContextScope::ContextScope(
        const ContextScope* contextScope
    ) {
        checkContext();

        #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
            contextGroup_ = threadContextGroup_ = contextScope->contextGroup_;
            contextGroup  = threadContextGroup  = contextScope->contextGroup;
        #endif
        threadContextGroup_->contextCount++;

        #ifdef GLCOMPACT_MULTIPLE_CONTEXT
            threadContext = new Context;
        #else
            new (threadContext)Context;
            threadContextConstructed = true;
        #endif
    }

    ContextScope::~ContextScope() {
        #ifdef GLCOMPACT_MULTIPLE_CONTEXT
            delete threadContext;
            threadContext = 0;
        #else
            threadContextConstructed = false;
        #endif

        threadContextGroup_->contextCount--;
        if (threadContextGroup_->contextCount == 0) {
            #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
                delete threadContextGroup_;
                threadContextGroup_ = 0;
                delete threadContextGroup;
                threadContextGroup  = 0;
            #else
                threadContextGroupConstructed_ = false;
            #endif
        }
    }
}
