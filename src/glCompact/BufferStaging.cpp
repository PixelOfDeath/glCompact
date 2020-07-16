#include "glCompact/BufferStaging.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/threadContextGroup_.hpp"
#include "glCompact/threadContext_.hpp"
#include "glCompact/Tools_.hpp"

#include <utility>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    /**
        \ingroup API
        \class glCompact::BufferStaging

        \brief OpenGL managed buffer object that can be directly accessed by the CPU and the GPU.

        \details This buffer is used for efficient data streaming between CPU and GPU.

        Synchronisation of reads, writes and cache flushing is done manually.

        CPU to GPU
        \code{.cpp}
            BufferStaging bufferStaging(1024);
            uint32_t* data = bufferStaging.getPtr();
            data[0] = 123;
            bufferStaging.bufferStagingFlushWrites();
            //All modifications are now visible to the GPU
        \endcode
        GPU to CPU
        \code{.cpp}
            BufferStaging bufferStaging(1024);
            uint32_t* data = bufferStaging.getPtr();
            //GPU command that writes to the buffer
            MemoryBarrier::bufferStagingFlushWrites();
            Fence fence;
            fence.insert();
            fence.isSignaledOrWait();
            //All modifications are now visible to the CPU
            cout << data[0] < endl;
        \endcode

        This class depends on GL_ARB_buffer_storage (Core since 4.4), to use it:

        set config.hpp:bufferStaging to FeatureSetting::mustBeSupported<br>
        or<br>
        set config.hpp:bufferStaging to FeatureSetting::runtimeDetection and at runtime check threadContextGroup->feature.bufferStaging<br>
        or<br>
        set config.hpp:glMin to GlVersion::v44 or higher
    */

    /**
        \brief Creates a buffer

        \param size Buffer size in byte
    */
    BufferStaging::BufferStaging(
        uintptr_t size
    ) {
        UNLIKELY_IF (!threadContextGroup_->feature.bufferStaging)
            crash("Trying to use BufferStaging, but missing feature bufferStaging!");
        mem = create(true, size, true, false);
    }

    BufferStaging::BufferStaging(
        const BufferStaging& buffer
    ) {
        mem = create(true, buffer.size, true, false);
        copyFromBuffer(buffer, 0, 0, size);
    }

    BufferStaging::BufferStaging(
        BufferStaging&& buffer
    ) :
        BufferInterface(move(buffer))
    {
        mem = buffer.mem;
        buffer.mem = 0;
    }

    BufferStaging& BufferStaging::operator=(
        const BufferStaging& buffer
    ) {
        UNLIKELY_IF (&buffer == this) return *this;
        free();
        return *new(this)BufferStaging(buffer);
    }

    BufferStaging& BufferStaging::operator=(
        BufferStaging&& buffer
    ) {
        free();
        return *new(this)BufferStaging(move(buffer));
    }

    BufferStaging::~BufferStaging() {
        free();
    }

    void BufferStaging::free() {
        BufferInterface::free();
        mem = 0;
    }

    void BufferStaging::bufferStagingFlushWrites() {
        bufferStagingFlushWrites(0, size);
    }

    void BufferStaging::bufferStagingFlushWrites(
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
