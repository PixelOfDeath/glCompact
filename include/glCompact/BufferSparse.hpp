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
    class BufferSparse;
    class BufferSparse : public BufferInterface {
        public:
            BufferSparse           () = default;
            BufferSparse           (bool clientMemoryCopyable, uintptr_t size);
            BufferSparse           (const BufferSparse&  buffer);
            BufferSparse           (      BufferSparse&& buffer);
            BufferSparse& operator=(const BufferSparse&  buffer);
            BufferSparse& operator=(      BufferSparse&& buffer);
            ~BufferSparse();
            void free();

            static uintptr_t getPageSize();
            void commitment(uintptr_t offset, uintptr_t size, bool commit);
        private:
            void create(bool clientMemoryCopyable, uintptr_t size);

            //uintptr_t commitedSize;
            //std::bitset commitMap //to store what pages are commited. So we always can accuratly calcualte what the current commit size is and also can copy this objects
            //uint64* commitMap;
            /*
                x86 only
                GCC?
                __builtin_popcount = int
                __builtin_popcountl = long int
                __builtin_popcountll = long long
            */
    };
}
