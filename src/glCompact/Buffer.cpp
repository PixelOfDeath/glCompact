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
#include "glCompact/Buffer.hpp"
#include "glCompact/ThreadContext.hpp"

namespace glCompact {
    /**
        \ingroup API
        \class glCompact::Buffer

        \brief OpenGL managed memory object

        \details If the GPU has didicated memory, this buffer can be considered to be located on it.
        Aside from OpenGL driver lazy initalisation (create and copy on first use) and memory pressure that may cause
        the driver to move the buffers content back to system memory.
    */

    /**
        \brief Creates a new buffer

        \param clientMemoryCopyable if set to true, the CPU can directly copy memory from and to this buffer via copyFromMemory/copyToMemory.
        For optimal performance, this should bet set to false. (Even on GPUs that share system memory with the CPU!)
        \param size size in byte
    */
    Buffer::Buffer(
        bool      clientMemoryCopyable,
        uintptr_t size
    ) {
        create(clientMemoryCopyable, size);
    }

    /**
        \brief Creates a new buffer with initial memory content

        \param clientMemoryCopyable if set to true, the CPU can directly copy memory from and to this buffer via copyFromMemory/copyToMemory.
        For optimal performance, this should bet set to false. (Even on GPUs that share system memory with the CPU!)
        \param size size in byte
        \param data data that gets copied into this buffer object
    */
    Buffer::Buffer(
        bool        clientMemoryCopyable,
        uintptr_t   size,
        const void* data
    ) {
        create(clientMemoryCopyable, size, data);
    }

    /**
        \brief Create new buffer by copying another buffer object

        \param buffer object to be copied
    */
    Buffer::Buffer(
        const Buffer& buffer
    ) {
        create(buffer.clientMemoryCopyable, buffer.size_);
        copyFromBuffer(buffer, 0, 0, buffer.size_);
    }

    Buffer::Buffer(
        Buffer&& buffer
    ) {
        id                   = buffer.id;
        size_                = buffer.size_;
        clientMemoryCopyable = buffer.clientMemoryCopyable;
        buffer.id            = 0;
        buffer.size_         = 0;
    }

    //TODO: don't create new one if current buffer is big enough but not to large? Could be an issue with ID still beeing pointed at by OpenGL? Maybe forced unbing/remove from context?
    Buffer& Buffer::operator=(
        const Buffer& buffer
    ) {
        if (this != &buffer) {
            create(buffer.clientMemoryCopyable, buffer.size_);
            copyFromBuffer(buffer, 0, 0, buffer.size_);
        }
        return *this;
    }

    Buffer& Buffer::operator=(
        Buffer&& buffer
    ) {
        assert(this != &buffer);
        free();
        id                   = buffer.id ;
        size_                = buffer.size_;
        clientMemoryCopyable = buffer.clientMemoryCopyable;
        buffer.id            = 0;
        buffer.size_         = 0;
        return *this;
    }

    Buffer::~Buffer() {
        free();
    }

    void Buffer::free() {
        BufferInterface::free();
    }

    void Buffer::create(
        bool        clientMemoryCopyable,
        uintptr_t   size,
        const void* data
    ) {
        free();
        create_(clientMemoryCopyable, size, false, false, data);
    }
}
