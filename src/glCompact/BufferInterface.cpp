#include "glCompact/BufferInterface.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/Buffer.hpp"
#include "glCompact/Context_.hpp"
#include "glCompact/ThreadContext_.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <algorithm> //min/max

#include <stdexcept>

    //#include <string>

//This can just change the content of any buffer, but not change the buffer itself!
/*
    The buffer classes automaticly use GL_ARB_buffer_storage if it is available. (Except for BufferStaging that depends on GL_ARB_buffer_storage to be present to work at all!)
*/

using namespace glCompact::gl;

namespace glCompact {
    /**
       \brief Copy data from another buffer object.

        GL_ARB_copy_buffer (Core since 3.1)
    */
    void BufferInterface::copyFromBuffer(
        const BufferInterface& srcBuffer,
        uintptr_t              srcOffset,
        uintptr_t              dstOffset,
        uintptr_t              copySize
    ) {
        Context_::assertThreadHasActiveGlContext();
        UNLIKELY_IF (copySize == 0) return;
        UNLIKELY_IF (!srcBuffer.id)
            throw std::runtime_error("Source buffer has no memory allocated");
        UNLIKELY_IF (!this->id)
            throw std::runtime_error("Target buffer has no memory allocated");
        UNLIKELY_IF (srcOffset > srcBuffer.size)
            throw std::runtime_error("Source offset is bayond source buffer size");
        UNLIKELY_IF (dstOffset > this->size)
            throw std::runtime_error("Target offset is bayond target buffer size");
        UNLIKELY_IF (srcOffset + copySize > srcBuffer.size)
            throw std::runtime_error("Offset and size is bayond source buffer size");
        UNLIKELY_IF (dstOffset + copySize > this->size)
            throw std::runtime_error("Offset and size is bayond target buffer size");

        if (threadContextGroup_->extensions.GL_ARB_direct_state_access)
            threadContextGroup_->functions.glCopyNamedBufferSubData(srcBuffer.id, id, srcOffset, dstOffset, copySize);
        else {
            threadContext_->cachedBindCopyReadBuffer(srcBuffer.id);
            threadContext_->cachedBindCopyWriteBuffer(id);
            threadContextGroup_->functions.glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, copySize);
        }
    }

    /**
        Most drivers will copy the memory block to OpenGL managed system memory and then return from this functions.
        Then the transfer between this OpenGL managed system memory and the final memory (vram in most cases) will happen.
        Unlike copyFromBuffer this function might limit the copying to the GPU VRam, so it runs in paralel/in chunks and does not completely block Rasterization/Compute shaders.
    */
    void BufferInterface::copyFromMemory(
        const void* srcMem,
        uintptr_t   thisOffset,
        uintptr_t   copySize
    ) {
        Context_::assertThreadHasActiveGlContext();
        UNLIKELY_IF (copySize == 0)
            return;
        UNLIKELY_IF (!this->id)
            throw std::runtime_error("Target buffer has no memory allocated");
        UNLIKELY_IF (thisOffset >= this->size)
            throw std::runtime_error("Offset is bayond buffer size");
        UNLIKELY_IF (thisOffset + copySize > this->size)
            throw std::runtime_error("Memory copy is bayond buffer size");
        UNLIKELY_IF (!clientMemoryCopyable)
            throw std::runtime_error("Buffer is not clientMemoryCopyable!");

        if (threadContextGroup_->extensions.GL_ARB_direct_state_access)
            threadContextGroup_->functions.glNamedBufferSubData(id, thisOffset, copySize, srcMem);
        else {
            threadContext_->cachedBindCopyWriteBuffer(id);
            threadContextGroup_->functions.glBufferSubData(GL_COPY_WRITE_BUFFER, thisOffset, copySize, srcMem);
        }
    }

    void BufferInterface::copyToMemory(
        void*     destMem,
        uintptr_t thisOffset,
        uintptr_t copySize
    ) const {
        Context_::assertThreadHasActiveGlContext();
        UNLIKELY_IF (copySize == 0)
            return;
        UNLIKELY_IF (!this->id)
            throw std::runtime_error("Source buffer has no memory allocated");
        UNLIKELY_IF (thisOffset >= this->size)
            throw std::runtime_error("Offset is bayond buffer size");
        UNLIKELY_IF (thisOffset + copySize > this->size)
            throw std::runtime_error("Memory copy is bayond buffer size");
        UNLIKELY_IF (!clientMemoryCopyable)
            throw std::runtime_error("Buffer is not clientMemoryCopyable!");

        if (threadContextGroup_->extensions.GL_ARB_direct_state_access)
            threadContextGroup_->functions.glGetNamedBufferSubData(id, thisOffset, copySize, destMem);
        else {
            threadContext_->cachedBindCopyReadBuffer(this->id);
            threadContextGroup_->functions.glGetBufferSubData(GL_COPY_READ_BUFFER, thisOffset, copySize, destMem);
        }
    }

    /**
        \brief Set the whole buffer content to the value 0
    */
    void BufferInterface::clear() {
        Context_::assertThreadHasActiveGlContext();
        //glClearBufferData -> If data is NULL , then the pointer is ignored and the sub-range of the buffer is filled with zeros.
        //Not sure if standard needs parameters when pointer is 0, but some drivers may fuck around otherwise!
        //GL_R8UI is core since 3.0.
        if (threadContextGroup_->extensions.GL_ARB_clear_buffer_object) {
            if (threadContextGroup_->extensions.GL_ARB_direct_state_access)
                threadContextGroup_->functions.glClearNamedBufferData       (this->id, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 0);
            else {
                threadContext_->cachedBindCopyWriteBuffer(this->id);
                threadContextGroup_->functions.glClearBufferData(GL_COPY_WRITE_BUFFER, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 0);
            }
        } else {
            clear(0, this->size);
        }
    }

    /**
        \brief Set the buffer content in the range offset and size to the value 0
    */
    void BufferInterface::clear(
        uintptr_t offset,
        uintptr_t size
    ) {
        Context_::assertThreadHasActiveGlContext();
        clear(offset, size, 1, 0);
    }

    /*
        Interface fits GL_ARB_clear_buffer_object (Core since 4.3), so it can automaticaly be used if present.

        This will also work without the extension via copy commands, but might be significantly slower. Therefor it should not be used in a hot loop without the extension being present.

        TODO: Will there be issues with big-endian/little-endian here? (AKA will this lib ever be used on non x86?)

        Only core since 3.0
            RG8
            RGBA32UI
        Only core since 3.1
            ..._INTEGER

        Probably not an issue except if we extent this library to support rust blocks.

        TODO: Can GL_BUFFER_UPDATE_BARRIER_BIT be used for both paths?
    */
    void BufferInterface::clear(
        uintptr_t   offset,
        uintptr_t   clearSize,
        uintptr_t   fillValueSize,
        const void* fillValue
    ) {
        Context_::assertThreadHasActiveGlContext();
        UNLIKELY_IF (clearSize == 0)
            return;
        UNLIKELY_IF (offset % fillValueSize != 0)
            throw std::runtime_error("offset must be aligned to size of fillValue!");
        UNLIKELY_IF (clearSize   % fillValueSize != 0)
            throw std::runtime_error("clearSize must be aligned to size of fillValue!");
        UNLIKELY_IF (!this->id)
            throw std::runtime_error("Buffer can't be cleared because there is no memory allocated");
        UNLIKELY_IF (offset >= this->size)
            throw std::runtime_error("Offset is bayond size of buffer");
        UNLIKELY_IF (offset + clearSize > this->size)
            throw std::runtime_error("Trying to clear bayond buffer size");

        if (threadContextGroup_->extensions.GL_ARB_clear_buffer_object) {
            GLenum internalFormat;
            GLenum componentArrangement;
            GLenum componentTypes;
            switch (fillValueSize) {
                case 1:
                    internalFormat       = GL_R8;
                    componentArrangement = GL_RED;
                    componentTypes       = GL_UNSIGNED_BYTE;
                    break;
                case 2:
                    internalFormat       = GL_RG8;
                    componentArrangement = GL_RG;
                    componentTypes       = GL_UNSIGNED_BYTE;
                    break;
                case 4:
                    internalFormat       = GL_RGBA8;
                    componentArrangement = GL_RGBA;
                    componentTypes       = GL_UNSIGNED_BYTE;
                    break;
                case 8:
                    internalFormat       = GL_RGBA16;
                    componentArrangement = GL_RGBA;
                    componentTypes       = GL_UNSIGNED_SHORT;
                    break;
                case 12:
                    internalFormat       = GL_RGB32UI;
                    componentArrangement = GL_RGB_INTEGER; //does not exist in 3.0, is there a non integer format with 12 bytes?
                    componentTypes       = GL_UNSIGNED_INT;
                    break;
                case 16:
                    internalFormat       = GL_RGBA32UI;
                    componentArrangement = GL_RGBA_INTEGER;
                    componentTypes       = GL_UNSIGNED_INT;
                    break;
                default:
                    throw std::runtime_error("fillValueSize must be 1, 2, 4, 8, 12 or 16!");
            }
            if (threadContextGroup_->extensions.GL_ARB_clear_buffer_object) {
                if (threadContextGroup_->extensions.GL_ARB_direct_state_access)
                    threadContextGroup_->functions.glClearNamedBufferSubData       (this->id, internalFormat, offset, clearSize, componentArrangement, componentTypes, fillValue);
                else {
                    threadContext_->cachedBindCopyWriteBuffer(this->id);
                    threadContextGroup_->functions.glClearBufferSubData(GL_COPY_WRITE_BUFFER, internalFormat, offset, clearSize, componentArrangement, componentTypes, fillValue);
                }
            }
        } else {
            //GL_ARB_sparse_buffer (Not core) depends on OpenGL 4.4. Therefor GL_ARB_clear_buffer_object (Core since 4.3) will be garantied to be present,
            //and we can ignore sparse buffers in this path!

            switch (fillValueSize) {
                case 1:
                case 2:
                case 4:
                case 8:
                case 12:
                case 16:
                    break;
                default:
                    throw std::runtime_error("fillValueSize must be 1, 2, 4, 8, 12 or 16!");
            }

            //upload data to buffer and then repeat copy with doubling size until the whole range is filled
            char nullValue = 0;
            if (fillValue == 0) {
                fillValue     = &nullValue;
                fillValueSize = 1;
            }
            if (clientMemoryCopyable) {
                copyFromMemory(fillValue, offset, fillValueSize);
            } else {
                //TODO make one buffer per context? Lazy init?
                Buffer buffer(false, fillValueSize, fillValue);
                copyFromBuffer(buffer, 0, offset, fillValueSize);
            }
            uintptr_t writeOffset  = offset    + fillValueSize;
            uintptr_t bytesToWrite = clearSize - fillValueSize;
            while (bytesToWrite) {
                uintptr_t copySize = std::min(writeOffset - offset, bytesToWrite);
                copyFromBuffer(*this, offset, writeOffset, copySize);
                writeOffset  += copySize;
                bytesToWrite -= copySize;
            }
        }
    }

    //If GL_ARB_invalidate_subdata (core in GL4.3) is not support this function will just do nothing
    //invalidating could be emulated by recreating the buffer object if using non-immutable?
    //this is basically just a hint, so we always can "support" this function without doing anything
    //Invalidating for example allows OpenGL to deallocate, to lower the memory pressure
    //and/or allocate new memory to start new rendering commands when the old target still is in use (indirect sync)
    void BufferInterface::invalidate() {
        Context_::assertThreadHasActiveGlContext();
        if (threadContextGroup_->extensions.GL_ARB_invalidate_subdata) {
            threadContextGroup_->functions.glInvalidateBufferData(id);
        } else if (!threadContextGroup_->extensions.GL_ARB_buffer_storage) {
            //For buffer objects with non-immutable storage, a buffer can be invalidated by calling glBufferData with the exact same size and usage hint as before,
            //and with a NULL data​ parameter. This is an older method (hack) of invalidation, and it should only be used when the others are not available.
            GLenum usageHint = GL_DYNAMIC_DRAW;
            if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
                threadContextGroup_->functions.glNamedBufferData(id, size, 0, usageHint);
            } else {
                threadContext_->cachedBindCopyWriteBuffer(id);
                threadContextGroup_->functions.glBufferData(GL_COPY_WRITE_BUFFER, size, 0, usageHint);
            }
        } else {
            //just ignore call, it is just a hind anyway...
        }
    }

    void BufferInterface::invalidate(
        uintptr_t offset,
        uintptr_t invalidateSize
    ) {
        Context_::assertThreadHasActiveGlContext();
        if (threadContextGroup_->extensions.GL_ARB_invalidate_subdata) {
            threadContextGroup_->functions.glInvalidateBufferSubData(id, offset, invalidateSize);
        } else {
            //just ignore call, it is just a hind anyway...
        }
    }

    BufferInterface::BufferInterface(BufferInterface&& buffer) {
        id                   = buffer.id;
        size                 = buffer.size;
        clientMemoryCopyable = buffer.clientMemoryCopyable;
        buffer.id                   = 0;
        buffer.size                 = 0;
        buffer.clientMemoryCopyable = false;
    }

    /*void BufferInterface::setDebugLabel(const std::string& label) {
        Context_::assertThreadHasActiveGlContext();
        if (threadContextGroup_->extensions.GL_KHR_debug) {
            GLsizei lenght = std::min(GLsizei(label.size()), GLsizei(threadContextGroup_->values.GL_MAX_LABEL_LENGTH));
            threadContextGroup_->functions.glObjectLabel(GL_BUFFER, id, lenght, label.c_str());
            //int maxSize = threadContextGroup_->values.GL_MAX_LABEL_LENGTH;
            //if (label.size() > maxSize) label.resize(maxSize);
            //threadContext->glObjectLabel(GL_BUFFER, id, -1, label.c_str());
        } else {
            //error?
        }
    }*/

    void* BufferInterface::create(
        bool        clientMemoryCopyable,
        uintptr_t   size,
        bool        stagingBuffer,
        bool        sparseBuffer,
        const void* data
    ) {
        Context_::assertThreadHasActiveGlContext();
        UNLIKELY_IF (stagingBuffer && !threadContextGroup_->extensions.GL_ARB_buffer_storage)
            throw std::runtime_error("Staging buffer not supported (missing GL_ARB_buffer_storage)");
        UNLIKELY_IF (sparseBuffer  && !threadContextGroup_->extensions.GL_ARB_sparse_buffer )
            throw std::runtime_error("Sparse buffer is not supported (missing GL_ARB_sparse_buffer)");
        //UNLIKELY_IF (stagingBuffer && sparseBuffer)
        //    throw std::runtime_error("Sparse buffer and staging buffer functionality can not be mixed");

        //Without GL_DYNAMIC_STORAGE_BIT the buffer can only be changed by server side commands. E.g. glBufferSubData would not work.
        //TODO: what happens with sparse buffers that get data to initalize? Is the data ignored or does the full buffer size gets commited automaticaly?
        GLbitfield flags =
            clientMemoryCopyable ? GL_DYNAMIC_STORAGE_BIT    : 0
        ||  sparseBuffer         ? GL_SPARSE_STORAGE_BIT_ARB : 0;

        if (stagingBuffer) flags |=
            GL_MAP_WRITE_BIT
        |   GL_MAP_READ_BIT
        |   GL_MAP_PERSISTENT_BIT
        |   GL_CLIENT_STORAGE_BIT;
        //| GL_MAP_COHERENT_BIT;

        if (sparseBuffer) flags |= GL_SPARSE_STORAGE_BIT_ARB;

        /*GLbitfield flags =
                (writeAccess            ? GL_DYNAMIC_STORAGE_BIT    : 0)
            |   (rawWriteAccess         ? GL_MAP_WRITE_BIT          : 0)
            |   (rawReadAccess          ? GL_MAP_READ_BIT           : 0)
            |   (rawAccess              ? GL_MAP_PERSISTENT_BIT    : 0)
            |   (clientStorageHint      ? GL_CLIENT_STORAGE_BIT     : 0)
            |   (sparseBuffer                 ? GL_SPARSE_STORAGE_BIT_ARB : 0);*/

        const GLenum stagingBufferAccessFlags =
            GL_MAP_WRITE_BIT
        |   GL_MAP_READ_BIT
        |   GL_MAP_PERSISTENT_BIT
        |   GL_MAP_FLUSH_EXPLICIT_BIT //TODO !?
            //| GL_MAP_COHERENT_BIT
        ;

        /*
            This are just hints... and the most significant hiden property is where the memory resists (system memory or vram)
            Drivers mostly ignore the hints and do what they want anyway.
            The usage of the buffer often makes the driver later decide/change what kind of memory is used.
            That is the reason why most of the possible hint values (e.g. GL_STATIC_COPY) are not even listed her
        */
        GLenum usageHint = GL_DYNAMIC_DRAW;
        /*if ( writeAccess ||  clientStorageHint) usageHint = GL_STREAM_DRAW;
        if (!writeAccess ||  clientStorageHint) usageHint = GL_STREAM_READ;
        if ( writeAccess || !clientStorageHint) usageHint = GL_DYNAMIC_DRAW;
        if (!writeAccess || !clientStorageHint) usageHint = GL_STATIC_DRAW;*/

        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            threadContextGroup_->functions.glCreateBuffers(1, &id);
            if (threadContextGroup_->extensions.GL_ARB_buffer_storage) {
                threadContextGroup_->functions.glNamedBufferStorage(id, size, data, flags);
                if (stagingBuffer) return threadContextGroup_->functions.glMapNamedBufferRange(id, 0, size, stagingBufferAccessFlags);
            } else {
                threadContextGroup_->functions.glNamedBufferData(id, size, data, usageHint);
            }
        } else {
            threadContextGroup_->functions.glGenBuffers(1, &id);
            threadContext_->cachedBindCopyWriteBuffer(id);
            if (threadContextGroup_->extensions.GL_ARB_buffer_storage) {
                threadContextGroup_->functions.glBufferStorage(GL_COPY_WRITE_BUFFER, size, data, flags);
                if (stagingBuffer) return threadContextGroup_->functions.glMapBufferRange(GL_COPY_WRITE_BUFFER, 0, size, stagingBufferAccessFlags);
            } else {
                threadContextGroup_->functions.glBufferData(GL_COPY_WRITE_BUFFER, size, data, usageHint);
            }
        }
        this->size                = size;
        this->clientMemoryCopyable = clientMemoryCopyable;
        return 0;
    }

    void BufferInterface::free() {
        //need something like this   if (std::is_static<*this>::value) return;
        //so I can ignore destructors from static objects, because the application is termination anyway and driver will clean up!
        //TODO: When do static objects from a closing thread get destroyed when the application keeps running?
        if (id) {
            UNLIKELY_IF (!threadContextGroup_)
                crash("glCompact::Buffer destructor called but thread has no reference to threadContextGroup_! Leaking OpenGL object!");
            #ifdef GLCOMPACT_DEBUG_ASSERT_THREAD_HAS_ACTIVE_CONTEXT
                UNLIKELY_IF (!threadContextGroup_->functions.glGetString(GL_VERSION))
                    crash("glCompact::Buffer destructor called but thread has no active OpenGL context! glDeleteBuffers without effect! Leaking OpenGL object!");
            #endif

            if (threadContext_) threadContext_->forgetBufferId(id);
            threadContextGroup_->functions.glDeleteBuffers(1, &id);

            id    = 0;
            size = 0;
            clientMemoryCopyable = false;
        }
    }
}
