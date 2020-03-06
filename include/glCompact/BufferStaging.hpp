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
    class BufferStaging;
    class BufferStaging : public BufferInterface {
        public:
            BufferStaging           () = default;
            BufferStaging           (uintptr_t size);
            BufferStaging           (const BufferStaging&  buffer);
            BufferStaging           (      BufferStaging&& buffer);
            BufferStaging& operator=(const BufferStaging&  buffer);
            BufferStaging& operator=(      BufferStaging&& buffer);
            ~BufferStaging();
            void free();

            void* getMem()const{return mem_;}
        private:
            void* mem_ = 0;

            void create(uintptr_t size);
    };
}
