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
#pragma once
#include "glCompact/BufferInterface.hpp"

namespace glCompact {
    class Buffer;
    class Buffer : public BufferInterface {
        public:
            Buffer           () = default;
            Buffer           (bool clientMemoryCopyable, uintptr_t size);
            Buffer           (bool clientMemoryCopyable, uintptr_t size, const void* data);
            Buffer           (const Buffer&  buffer);
            Buffer           (      Buffer&& buffer);
            Buffer& operator=(const Buffer&  buffer);
            Buffer& operator=(      Buffer&& buffer);
            ~Buffer          ();
            void free();
        private:
            void create(bool clientMemoryCopyable, uintptr_t size, const void* data = 0);
    };
}
