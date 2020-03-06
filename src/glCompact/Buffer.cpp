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
        Creates a new buffer.

        \brief Creates a new OpenGL buffer.
        \param size Buffer size in byte
    */
    Buffer::Buffer(
        bool      clientMemoryCopyable,
        uintptr_t size
    ) {
        create(clientMemoryCopyable, size);
    }

    Buffer::Buffer(
        bool        clientMemoryCopyable,
        uintptr_t   size,
        const void* data
    ) {
        create(clientMemoryCopyable, size, data);
    }

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
