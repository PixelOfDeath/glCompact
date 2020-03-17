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
#include "glCompact/Config.hpp"
#include <cstdint> //C++11

namespace glCompact {
    class BufferInterface;
    class BufferInterface {
            friend class TextureInterface;
            friend class PipelineInterface;
            friend class PipelineRasterization;
            friend class PipelineCompute;
        public:
            void copyFromBuffer(const BufferInterface& srcBuffer, uintptr_t   srcOffset, uintptr_t thisOffset, uintptr_t size);
            void copyFromMemory(                                  const void* srcMem   , uintptr_t thisOffset, uintptr_t size);
            void copyToMemory  (                                        void* destMem  , uintptr_t thisOffset, uintptr_t size) const;

            void clear();
            void clear(uintptr_t offset, uintptr_t size);
            /**
                \brief Set the buffer content in the range offset and size to the value of fillValue.
                fillValue must have a size of 1, 2, 4, 8, 12 or 16 bytes!
            */
            template <typename T>
            void clear(uintptr_t offset, uintptr_t size, T fillValue) {
                static_assert(
                        sizeof(T) == 1
                    ||  sizeof(T) == 2
                    ||  sizeof(T) == 4
                    ||  sizeof(T) == 8
                    ||  sizeof(T) == 12
                    ||  sizeof(T) == 16,
                    "fillValue type must have a size of 1, 2, 4, 8, 12 or 16 bytes!"
                );
                //static_assert(offset % sizeof(T) == 0, "offset must be aligned to the size of fillValue");
                //GCC should support opportunistic static asserts

                clear_(offset, size, sizeof(T), &fillValue);
            }

            void invalidate();
            void invalidate(uintptr_t offset, uintptr_t size);

            void detachFromThreadContext();
            //void setDebugLabel(const std::string& label);

            bool isClientMemoryCopyable()const{return clientMemoryCopyable;}

            uintptr_t getSize()const{return size_;}
        protected:
            BufferInterface() = default;
            ~BufferInterface() = default;

            uint32_t  id    = 0;
            uintptr_t size_ = 0;
            bool clientMemoryCopyable = false;

            void* create_(bool clientMemoryCopyable, uintptr_t size, bool stagingBuffer, bool sparseBuffer, const void* data = 0);
            void clear_(uintptr_t offset, uintptr_t size, uintptr_t fillValueSize, const void* fillValue);
            void free();
        private:
    };
}
