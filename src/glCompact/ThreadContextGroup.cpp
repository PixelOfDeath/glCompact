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
