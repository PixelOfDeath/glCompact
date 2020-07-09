#include "glCompact/BufferCpu.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/threadContextGroup_.hpp"
#include "glCompact/threadContext_.hpp"
#include "glCompact/ToolsInternal.hpp"

#include <utility>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    /**
        \ingroup API
        \class glCompact::BufferCpu

        \brief OpenGL managed buffer object that is located in CPU memory. It can be directly accessed by the CPU and the GPU.

        \details Synchronisation of reads, writes and cache flushing is done manually.
        This buffer can be used as staging buffer to stream data to and from BufferGpu. Or for data that is only used once after modification.

        CPU to GPU
        \code{.cpp}
            BufferCpu bufferCpu(1024);
            uint32_t* data = bufferCpu.getPtr();
            data[0] = 123;
            bufferCpu.flushMappedMemoryWrites();
            //All modifications are now visible to the GPU<br>
        \endcode
        GPU to CPU
        \code{.cpp}
            BufferCpu bufferCpu(1024);
            uint32_t* data = bufferCpu.getPtr();
            //GPU command that writes to the buffer
            MemoryBarrier::flushMappedMemoryWrites();
            Fence fence;
            fence.insert();
            fence.isSignaledOrWait();
            //All modifications are now visible to the CPU
            cout << data[0] < endl;
        \endcode
        This type of buffer depends on the extension GL_ARB_buffer_storage (Core since 4.4)
    */

    /**
        \brief Creates the staging buffer

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

    void BufferCpu::flushMappedMemoryWrites() {
        flushMappedMemoryWrites(0, size);
    }

    void BufferCpu::flushMappedMemoryWrites(
        uintptr_t offset,
        uintptr_t size
    ) {
        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            threadContextGroup_->functions.glFlushMappedNamedBufferRange(id, offset, size);
        } else {
            threadContext_->cachedBindCopyWriteBuffer(id);
            threadContextGroup_->functions.glFlushMappedBufferRange(GL_COPY_WRITE_BUFFER, offset, size);
        }
    }
}
