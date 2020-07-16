#include "glCompact/ContextScope.hpp"
#include "glCompact/Tools_.hpp"
#include "glCompact/config.hpp"
#include "glCompact/Context_.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/ContextGroup.hpp"
#include "glCompact/threadContext_.hpp"
#include "glCompact/threadContextGroup_.hpp"
#include "glCompact/threadContextGroup.hpp"

namespace glCompact {
    /**
        \ingroup API
        \class glCompact::ContextScope
        \brief Scope object that initalizes the glCompact internals

        \details Each thread/context needs this object to be created before using any glCompact functionality.
        The current thread needs to have an active OpenGL 3.3 context or higher. Or whatever is set as minimum in config.hpp.

        There are three constructors. Two that take a function pointer for getting OpenGL functions by c-string.
        And one that take a pointer to an already created ContextScope from another thread/context to share a ContextGroup.

        Most Windows/OpenGL context creation libraries supply such a function.

        \code{.cpp}
        //void *(*)(const char*)
        ContextScope myContextScope(SDL_GL_GetProcAddress)    //SDL2

        //void(*(*)(const char*))()
        ContextScope myContextScope(glfwGetProcAddress)       //GLFW
        ContextScope myContextScope(sf::Context::getFunction) //SFML
        \endcode
    */

    static void checkContext() {
        #ifdef GLCOMPACT_MULTIPLE_CONTEXT
            UNLIKELY_IF (threadContext_ != 0)
                crash("This thread already has a glCompact::Context object registered!");
        #else
            UNLIKELY_IF (threadContextConstructed_)
                crash("A glCompact::Context object is already registered!\nNote: config.hpp GLCOMPACT_MULTIPLE_CONTEXT is not defined! Define it if you want to use more then one context!");
        #endif
    }

    static void checkContextGroup() {
        #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
            UNLIKELY_IF (threadContextGroup_ != 0)
                crash("This thread already has a glCompact::ContextGroup_ object registered!");
        #else
            UNLIKELY_IF (threadContextGroupConstructed_)
                crash("A glCompact::ContextGroup object is already registered!\nNote: config.hpp GLCOMPACT_MULTIPLE_CONTEXT_GROUP is not defined! Define it if you want to use more then one shared ContextGroup!");
        #endif
    }

    /**
     * \brief Constructor
     *
     * \param getGlFunctionPointer pointer to function that returns OpenGL function pointers via c-string name
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
            threadContext_ = new Context_;
        #else
            new (threadContext_)Context_;
            new (threadContext )Context;
            threadContextConstructed_ = true;
            threadContextConstructed  = true;
        #endif
        threadContext_->isMainContext = true;
    }

    /**
     * \brief Constructor
     *
     * \param getGlFunctionPointer pointer to function that returns OpenGL function pointers via c-string name
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
            threadContext_ = new Context_;
        #else
            new (threadContext_)Context_;
            new (threadContext )Context;
            threadContextConstructed_ = true;
            threadContextConstructed  = true;
        #endif
    }

    ContextScope::~ContextScope() {
        #ifdef GLCOMPACT_MULTIPLE_CONTEXT
            delete threadContext_;
            threadContext_ = 0;
        #else
            threadContextConstructed_ = false;
        #endif

        threadContextGroup_->contextCount--;
        if (threadContextGroup_->contextCount == 0) {
            #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
                delete threadContextGroup_;
                delete threadContextGroup;
                threadContextGroup_ = 0;
                threadContextGroup  = 0;
            #else
                threadContextGroupConstructed_ = false;
            #endif
        }
    }
}
