#include "glCompact/BufferMapped.hpp"
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
        \class glCompact::BufferMapped

        \brief OpenGL managed buffer object that can be directly accessed by the CPU and the GPU.

        \details Synchronisation of reads, writes and cache flushing is done manually.
        This buffer can be used as staging buffer to stream data to and from other Buffer objects. Or for data that is only written once by the producer and and read once by the consumer.

        CPU to GPU
        \code{.cpp}
            BufferMapped bufferMapped(1024);
            uint32_t* data = bufferMapped.getPtr();
            data[0] = 123;
            bufferMapped.flushMappedMemoryWrites();
            //All modifications are now visible to the GPU<br>
        \endcode
        GPU to CPU
        \code{.cpp}
            BufferMapped bufferMapped(1024);
            uint32_t* data = bufferMapped.getPtr();
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
    BufferMapped::BufferMapped(
        uintptr_t size
    ) {
        mem = create(true, size, true, false);
    }

    BufferMapped::BufferMapped(
        const BufferMapped& buffer
    ) {
        mem = create(true, buffer.size, true, false);
        copyFromBuffer(buffer, 0, 0, size);
    }

    BufferMapped::BufferMapped(
        BufferMapped&& buffer
    ) :
        BufferInterface(move(buffer))
    {
        mem = buffer.mem;
        buffer.mem = 0;
    }

    BufferMapped& BufferMapped::operator=(
        const BufferMapped& buffer
    ) {
        UNLIKELY_IF (&buffer == this) return *this;
        free();
        return *new(this)BufferMapped(buffer);
    }

    BufferMapped& BufferMapped::operator=(
        BufferMapped&& buffer
    ) {
        free();
        return *new(this)BufferMapped(move(buffer));
    }

    BufferMapped::~BufferMapped() {
        free();
    }

    void BufferMapped::free() {
        BufferInterface::free();
        mem = 0;
    }

    void BufferMapped::flushMappedMemoryWrites() {
        flushMappedMemoryWrites(0, size);
    }

    void BufferMapped::flushMappedMemoryWrites(
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
