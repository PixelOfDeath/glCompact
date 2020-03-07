/*
    glCompact
    Copyright (C) 2019-2020 Frederik Uje vom Hofe

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "glCompact/ContextScope.hpp"
#include "glCompact/Context.hpp"
#include "glCompact/ContextGroup.hpp"
#include "glCompact/ToolsInternal.hpp"
#include "glCompact/Config.hpp"
#include "glCompact/ThreadContext.hpp"
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
            UNLIKELY_IF (threadContextGroup != 0)
                crash("This thread already has a glCompact::ContextGroup object registered!");
        #else
            UNLIKELY_IF (threadContextGroupConstructed)
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
            threadContextGroup = new ContextGroup(getGlFunctionPointer);
        #else
            new (threadContextGroup)ContextGroup(getGlFunctionPointer);
            threadContextGroupConstructed = true;
        #endif
        contextGroup = threadContextGroup;

        #ifdef GLCOMPACT_MULTIPLE_CONTEXT
            threadContext = new Context;
        #else
            new (threadContext)Context;
            threadContextConstructed = true;
        #endif
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
            contextGroup = contextScope->contextGroup;
            threadContextGroup = contextGroup;
        #endif
        threadContextGroup->contextCount++;

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

        if (--threadContextGroup->contextCount == 0) {
            #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
                delete threadContextGroup;
                threadContextGroup = 0;
            #else
                threadContextGroupConstructed = false;
            #endif
        }
    }
}
