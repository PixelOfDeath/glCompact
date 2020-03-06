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
        ///\cond HIDDEN_FROM_DOXYGEN
        private:
            void* mem_ = 0;

            void create(uintptr_t size);
        ///\endcond
    };
}
