#include "glCompact/Buffer.hpp"
#include "glCompact/ThreadContext.hpp"
#include "glCompact/ToolsInternal.hpp"

#include <assert.h>

using namespace std;

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
        create(clientMemoryCopyable, size, false, false);
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
        create(clientMemoryCopyable, size, false, false, data);
    }

    /**
        \brief Create new buffer by copying another buffer object

        \param buffer object to be copied
    */
    Buffer::Buffer(
        const Buffer& buffer
    ) {
        create(buffer.clientMemoryCopyable, buffer.size, false, false);
        copyFromBuffer(buffer, 0, 0, buffer.size);
    }

    Buffer::Buffer(
        Buffer&& buffer
    ) :
        BufferInterface(move(buffer))
    {}

    //TODO: don't create new one if current buffer is big enough but not to large? Could be an issue with ID still beeing pointed at by OpenGL? Maybe forced unbing/remove from context?
    Buffer& Buffer::operator=(
        const Buffer& buffer
    ) {
        UNLIKELY_IF (&buffer == this) return *this;
        free();
        return *new(this)Buffer(buffer);
    }

    Buffer& Buffer::operator=(
        Buffer&& buffer
    ) {
        free();
        return *new(this)Buffer(move(buffer));
    }

    Buffer::~Buffer() {
        free();
    }

    void Buffer::free() {
        BufferInterface::free();
    }
}
