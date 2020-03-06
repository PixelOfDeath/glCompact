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
#include "glCompact/BufferStaging.hpp"
#include "glCompact/gl/Constants.hpp"
//#include "glCompact/ThreadContext.hpp"

/*
    needs GL_ARB_buffer_storage (Core since 4.4)

    Classical mapping/unmapping is an anti patern on modern drivers. It hurts performance because it forces syncing betwin the application side API and driver thread.
    So we only support a persisten mapped client buffer as staging buffer.

    Memory barrier usage patern:<br>
        MAP_COHERENT_BIT is not used and syncing is always needed after writing/reading raw to the buffer:
        after  writing: glMemoryBarrier(CLIENT_MAPPED_BUFFER_BARRIER_BIT);
        before reading: glMemoryBarrier(CLIENT_MAPPED_BUFFER_BARRIER_BIT) and (glFenceSync(SYNC_GPU_COMMANDS_COMPLETE, 0) and wait for it OR glFinish())

        <br>
        After writing there needs to be a memory barrier before using OpenGL commands on the new buffer data. Otherwise the behaviour is undefined.<br>
        <b><i>write to memory...</i><br>
        glMemoryBarrier(CLIENT_MAPPED_BUFFER_BARRIER_BIT)<br>
        <i>OpenGL commands that use new buffer data</i></b>

        After OpenGL commands chang the data there needs to be a memory barrier and sync with a fance or glFinish befor reading from it. Otherwise the behavior is undefined.<br>
        <b><i>OpenGL command changing the buffer data</i><br>
        glMemoryBarrier(CLIENT_MAPPED_BUFFER_BARRIER_BIT)<br>
        GLsync fence = glFenceSync(SYNC_GPU_COMMANDS_COMPLETE, 0)<br>
        ...<br>
        glClientWaitSync(fence)<br>
        <i>read from memory</i></b>
        \return pointer to buffer memory


        //GL_MAP_FLUSH_EXPLICIT_BIT and glFlushMappedBufferRange() ?
        //void glFlushMappedBufferRange( enum target, intptr offset, sizeiptr length );
        //void glFlushMappedNamedBufferRange( uint buffer, intptr offset, sizeiptr length );


    Note:
        GL_ARB_buffer_storage will NOT give you a unambiguous way to set the buffer memory location (system ram or vram).
        There is just a probability by usage and hints that MAYBE influences it enough to do what we want it to do...

    TODO:
        For debuging GetWriteWatch (or others) could be used to catch if trying to use buffer after write without memory barrier!

        Maybe also use AMD_pinned_memory, problem: to free memory save a sync point is needed to make sure no more gpu access happens

        Older Intel drivers may not support GL_ARB_buffer_storage, but in non core mode they allow (non standard conform) to map memory directly as buffer.
        So one could use it like pinned memory.
*/

namespace glCompact {

    /**
        Creates a new client side mapped buffer. The memory can be directly accessed via the mem pointer.

        Needs GL_ARB_buffer_storage (Core since OpenGL 4.4)

        This buffers are basicly normal system memory. But the driver manages the memory so you can call copy commands on it to other buffers/textures.

        \brief Creates a new OpenGL buffer in client memory that can be directly accessed.
        \param size Buffer size in byte
    */
    BufferStaging::BufferStaging(
        uintptr_t size
    ) {
        create(size);
    }

    BufferStaging::BufferStaging(
        const BufferStaging& buffer
    ) {
        create(buffer.size_);
        copyFromBuffer(buffer, 0, 0, size_);
    }

    BufferStaging::BufferStaging(
        BufferStaging&& buffer
    ) {
        id           = buffer.id;
        size_        = buffer.size_;
        mem_         = buffer.mem_;
        buffer.id    = 0;
        buffer.size_ = 0;
        buffer.mem_  = 0;
    }

    BufferStaging& BufferStaging::operator=(
        const BufferStaging& buffer
    ) {
        if (this != &buffer) {
            create(buffer.size_); //TODO: don't create new one if current buffer is big enough but not to large? Could be an issue with ID still beeing pointed at by OpenGL?
            copyFromBuffer(buffer, 0, 0, buffer.size_);
        }
        return *this;
    }

    BufferStaging& BufferStaging::operator=(
        BufferStaging&& buffer
    ) {
        if (this != &buffer) {
            free();
            id           = buffer.id;
            size_        = buffer.size_;
            mem_         = buffer.mem_;
            buffer.id    = 0;
            buffer.size_ = 0;
            buffer.mem_  = 0;
        }
        return *this;
    }

    BufferStaging::~BufferStaging() {
        mem_ = 0;
    }

    void BufferStaging::free() {
        BufferInterface::free();
        mem_ = 0;
    }

    void BufferStaging::create(
        uintptr_t size
    ) {
        free();
        mem_ = create_(true, size, true, false);
    }
}
