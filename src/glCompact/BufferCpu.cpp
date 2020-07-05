#include "glCompact/BufferCpu.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/threadContextGroup_.hpp"
#include "glCompact/threadContext_.hpp"
#include "glCompact/ToolsInternal.hpp"

#include <utility>

using namespace std;
using namespace glCompact::gl;

/*
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
    BufferCpu::BufferCpu(
        uintptr_t size
    ) {
        mem = create(true, size, true, false);
    }

    BufferCpu::BufferCpu(
        const BufferCpu& buffer
    ) {
        mem = create(true, buffer.size, true, false);
        copyFromBuffer(buffer, 0, 0, size);
    }

    BufferCpu::BufferCpu(
        BufferCpu&& buffer
    ) :
        BufferInterface(move(buffer))
    {
        mem = buffer.mem;
        buffer.mem = 0;
    }

    BufferCpu& BufferCpu::operator=(
        const BufferCpu& buffer
    ) {
        UNLIKELY_IF (&buffer == this) return *this;
        free();
        return *new(this)BufferCpu(buffer);
    }

    BufferCpu& BufferCpu::operator=(
        BufferCpu&& buffer
    ) {
        free();
        return *new(this)BufferCpu(move(buffer));
    }

    BufferCpu::~BufferCpu() {
        free();
    }

    void BufferCpu::free() {
        BufferInterface::free();
        mem = 0;
    }

    void BufferCpu::flushCpuCache() {
        flushCpuCache(0, size);
    }

    void BufferCpu::flushCpuCache(uintptr_t offset, uintptr_t size) {
        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            threadContextGroup_->functions.glFlushMappedNamedBufferRange(id, offset, size);
        } else {
            threadContext_->cachedBindCopyWriteBuffer(id);
            threadContextGroup_->functions.glFlushMappedBufferRange(GL_COPY_WRITE_BUFFER, offset, size);
        }
    }
}
