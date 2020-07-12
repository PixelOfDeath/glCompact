#include "glCompact/BufferGpu.hpp"
#include "glCompact/threadContext_.hpp"
#include "glCompact/Tools_.hpp"

using namespace std;

namespace glCompact {
    /**
        \ingroup API
        \class glCompact::BufferGpu

        \brief OpenGL managed memory object

        \details If the GPU has didicated memory, this buffer can be considered to be located on it.
        Aside from OpenGL driver lazy initalisation (create and/or copy on first use) and memory pressure that may cause
        the driver to move the buffer content back to system memory.
    */

    /**
        \brief Creates a new buffer

        \param clientMemoryCopyable If set to true, the CPU can directly copy memory from and to this buffer via copyFromMemory/copyToMemory.
        For optimal performance, this should bet set to false if no direct CPU access is needed. Including on APUs that share system memory with the CPU!
        \param size size in byte
    */
    BufferGpu::BufferGpu(
        bool      clientMemoryCopyable,
        uintptr_t size
    ) {
        create(clientMemoryCopyable, size, false, false);
    }

    /**
        \brief Creates a new buffer with initial memory content

        \param clientMemoryCopyable If set to true, the CPU can directly copy memory from and to this buffer via copyFromMemory/copyToMemory.
        For optimal performance, this should bet set to false if no direct CPU access is needed. Including on APUs that share system memory with the CPU!
        \param size size in byte
        \param data data that gets copied into this buffer object
    */
    BufferGpu::BufferGpu(
        bool        clientMemoryCopyable,
        uintptr_t   size,
        const void* data
    ) {
        create(clientMemoryCopyable, size, false, false, data);
    }

    /**
        \brief Create new buffer by copying another buffer size and content

        \param buffer object to be copied
    */
    BufferGpu::BufferGpu(
        const BufferGpu& buffer
    ) {
        create(buffer.clientMemoryCopyable, buffer.size, false, false);
        copyFromBuffer(buffer, 0, 0, buffer.size);
    }

    BufferGpu::BufferGpu(
        BufferGpu&& buffer
    ) :
        BufferInterface(move(buffer))
    {}

    //TODO: don't create new one if current buffer is big enough but not to large? Could be an issue with ID still beeing pointed at by OpenGL? Maybe forced unbing/remove from context?
    BufferGpu& BufferGpu::operator=(
        const BufferGpu& buffer
    ) {
        UNLIKELY_IF (&buffer == this) return *this;
        free();
        return *new(this)BufferGpu(buffer);
    }

    BufferGpu& BufferGpu::operator=(
        BufferGpu&& buffer
    ) {
        free();
        return *new(this)BufferGpu(move(buffer));
    }

    BufferGpu::~BufferGpu() {
        free();
    }

    void BufferGpu::free() {
        BufferInterface::free();
    }
}
