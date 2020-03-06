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
#include "glCompact/ThreadContextGroup.hpp"

namespace glCompact {
    #ifdef GLCOMPACT_MULTIPLE_CONTEXT_GROUP
        thread_local ContextGroup*      __restrict threadContextGroup;
    #else
        static       char                          contextGroupMem[sizeof(ContextGroup)] alignas(ContextGroup);
                     ContextGroup*const __restrict threadContextGroup = reinterpret_cast<ContextGroup*>(&contextGroupMem);
                     bool                          threadContextGroupConstructed = false;
    #endif
}
