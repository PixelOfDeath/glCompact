/*
    This file is part of glCompact.
    Copyright (C) 2019-2020 Frederik Uje vom Hofe

    glCompact is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    glCompact is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "glCompact/ThreadContext.hpp"

/*TODO
  Big issue still to be solveed:

  In the case of a multi thread/multi context application,
  if a thread has thread-global glCompact objects and gets destroeyed
  the destructors call OpenGL functions to delete the underlaying GL objects.
  If the context is already destroyed then the calls are silently ignored (OpenGL standard)
  And we end up with memory leaks in form of left over gl objects!

  A: In debug mode will check if thread has active context in every function that needs one. Otherwise throw error!
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
