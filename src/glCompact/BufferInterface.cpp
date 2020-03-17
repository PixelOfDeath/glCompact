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
#include "glCompact/BufferInterface.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/Buffer.hpp"
#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"
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
     * \brief Copy data from another buffer object.

        GL_ARB_copy_buffer (Core since 3.1)
    */
    void BufferInterface::copyFromBuffer(
        const BufferInterface& srcBuffer,
        uintptr_t              srcOffset,
        uintptr_t              dstOffset,
        uintptr_t              copySize
    ) {
        Context::assertThreadHasActiveGlContext();
        UNLIKELY_IF (copySize == 0) return;
        UNLIKELY_IF (!srcBuffer.id)
            throw std::runtime_error("Source buffer has no memory allocated");
        UNLIKELY_IF (!this->id)
            throw std::runtime_error("Target buffer has no memory allocated");
        UNLIKELY_IF (srcOffset > srcBuffer.size_)
            throw std::runtime_error("Source offset is bayond source buffer size");
        UNLIKELY_IF (dstOffset > this->size_)
            throw std::runtime_error("Target offset is bayond target buffer size");
        UNLIKELY_IF (srcOffset + copySize > srcBuffer.size_)
            throw std::runtime_error("Offset and size is bayond source buffer size");
        UNLIKELY_IF (dstOffset + copySize > this->size_)
            throw std::runtime_error("Offset and size is bayond target buffer size");

        if (threadContextGroup->extensions.GL_ARB_direct_state_access)
            threadContextGroup->functions.glCopyNamedBufferSubData(srcBuffer.id, id, srcOffset, dstOffset, copySize);
        else if (threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glNamedCopyBufferSubDataEXT(srcBuffer.id, id, srcOffset, dstOffset, copySize);
        else {
            threadContext->cachedBindCopyReadBuffer(srcBuffer.id);
            threadContext->cachedBindCopyWriteBuffer(id);
            threadContextGroup->functions.glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, copySize);
        }
    }

    /**
        Most drivers will copy the memory block to OpenGL managed system memory and then return from this functions.
        Then the transfer between this OpenGL managed system memory and the final memory (vram in most cases) will happen.
        Unlike copyFromBuffer this function will limit the copying so it runs in paralel/in chunks and does not completely block the system.
    */
    void BufferInterface::copyFromMemory(
        const void* srcMem,
        uintptr_t   thisOffset,
        uintptr_t   copySize
    ) {
        Context::assertThreadHasActiveGlContext();
        UNLIKELY_IF (copySize == 0)
            return;
        UNLIKELY_IF (!this->id)
            throw std::runtime_error("Target buffer has no memory allocated");
        UNLIKELY_IF (thisOffset >= this->size_)
            throw std::runtime_error("Offset is bayond buffer size");
        UNLIKELY_IF (thisOffset + copySize > this->size_)
            throw std::runtime_error("Memory copy is bayond buffer size");
        UNLIKELY_IF (!clientMemoryCopyable)
            throw std::runtime_error("Buffer is not clientMemoryCopyable!");

        if (threadContextGroup->extensions.GL_ARB_direct_state_access)
            threadContextGroup->functions.glNamedBufferSubData(id, thisOffset, copySize, srcMem);
        else if (threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glNamedBufferSubDataEXT(id, thisOffset, copySize, srcMem);
        else {
            threadContext->cachedBindCopyWriteBuffer(id);
            threadContextGroup->functions.glBufferSubData(GL_COPY_WRITE_BUFFER, thisOffset, copySize, srcMem);
        }
    }

    void BufferInterface::copyToMemory(
        void*     destMem,
        uintptr_t thisOffset,
        uintptr_t copySize
    ) const {
        Context::assertThreadHasActiveGlContext();
        UNLIKELY_IF (copySize == 0)
            return;
        UNLIKELY_IF (!this->id)
            throw std::runtime_error("Source buffer has no memory allocated");
        UNLIKELY_IF (thisOffset >= this->size_)
            throw std::runtime_error("Offset is bayond buffer size");
        UNLIKELY_IF (thisOffset + copySize > this->size_)
            throw std::runtime_error("Memory copy is bayond buffer size");
        UNLIKELY_IF (!clientMemoryCopyable)
            throw std::runtime_error("Buffer is not clientMemoryCopyable!");

        if (threadContextGroup->extensions.GL_ARB_direct_state_access)
            threadContextGroup->functions.glGetNamedBufferSubData(id, thisOffset, copySize, destMem);
        else if (threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glGetNamedBufferSubDataEXT(id, thisOffset, copySize, destMem);
        else {
            threadContext->cachedBindCopyReadBuffer(this->id);
            threadContextGroup->functions.glGetBufferSubData(GL_COPY_READ_BUFFER, thisOffset, copySize, destMem);
        }
    }

    //NOTE: glClearBufferData -> If data is NULL , then the pointer is ignored and the sub-range of the buffer is filled with zeros.
    void BufferInterface::clear() {
        Context::assertThreadHasActiveGlContext();
        //not sure if standard needs parameters when pointer is 0, but some drivers may fuck around otherwise!
        //GL_R8UI is core since 3.0.
        if (threadContextGroup->extensions.GL_ARB_clear_buffer_object) {
            if (threadContextGroup->extensions.GL_ARB_direct_state_access)
                threadContextGroup->functions.glClearNamedBufferData       (this->id, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 0);
            else if (threadContextGroup->extensions.GL_EXT_direct_state_access)
                threadContextGroup->functions.glClearNamedBufferDataEXT    (this->id, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 0);
            else {
                threadContext->cachedBindCopyWriteBuffer(this->id);
                threadContextGroup->functions.glClearBufferData(GL_COPY_WRITE_BUFFER, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 0);
            }
        } else {
            clear(0, this->size_);
        }
    }

    void BufferInterface::clear(
        uintptr_t offset,
        uintptr_t size
    ) {
        Context::assertThreadHasActiveGlContext();
        clear_(offset, size, 1, 0);
    }

    /*
        TODO: Will there be issues with big-endian/little-endian here? (AKA will this lib ever be used on non x86?)

        Only core since 3.0
            RG8
            RGBA32UI
        Only core since 3.1
            ..._INTEGER

        Probably not an issue except if we extent this library to support rust blocks.
    */
    void BufferInterface::clear_(
        uintptr_t   offset,
        uintptr_t   clearSize,
        uintptr_t   fillValueSize,
        const void* fillValue
    ) {
        Context::assertThreadHasActiveGlContext();
        UNLIKELY_IF (clearSize == 0)
            return;
        UNLIKELY_IF (offset % fillValueSize != 0)
            throw std::runtime_error("offset must be aligned to size of fillValue!");
        UNLIKELY_IF (clearSize   % fillValueSize != 0)
            throw std::runtime_error("clearSize must be aligned to size of fillValue!");
        UNLIKELY_IF (!this->id)
            throw std::runtime_error("Buffer can't be cleared because there is no memory allocated");
        UNLIKELY_IF (offset >= this->size_)
            throw std::runtime_error("Offset is bayond size of buffer");
        UNLIKELY_IF (offset + clearSize > this->size_)
            throw std::runtime_error("Trying to clear bayond buffer size");

        if (threadContextGroup->extensions.GL_ARB_clear_buffer_object) {
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
            if (threadContextGroup->extensions.GL_ARB_clear_buffer_object) {
                if (threadContextGroup->extensions.GL_ARB_direct_state_access)
                    threadContextGroup->functions.glClearNamedBufferSubData       (this->id, internalFormat, offset, clearSize, componentArrangement, componentTypes, fillValue);
                else if (threadContextGroup->extensions.GL_EXT_direct_state_access)
                    threadContextGroup->functions.glClearNamedBufferSubDataEXT    (this->id, internalFormat, offset, clearSize, componentArrangement, componentTypes, fillValue);
                else {
                    threadContext->cachedBindCopyWriteBuffer(this->id);
                    threadContextGroup->functions.glClearBufferSubData(GL_COPY_WRITE_BUFFER, internalFormat, offset, clearSize, componentArrangement, componentTypes, fillValue);
                }
            }
        } else {
            //TODO: missing limiter check for size = 1, 2, 4, 8, 12, 16

            //upload data and then repeat copy in buffer until the whole size range is filled
            //Q: This needs a special implementation for sparse buffers?
            //A: Probably no sparse support anyway without the real clear function being present!
            char nullValue = 0;
            if (fillValue == 0) {
                fillValue     = &nullValue;
                fillValueSize = 1;
            }
            if (clientMemoryCopyable) {
                copyFromMemory(fillValue, offset, fillValueSize); //ignore check for clientMemoryCopyable
            } else {
                //TODO make one buffer per context? Lazy init?
                Buffer buffer(fillValueSize, true, fillValue);
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
        Context::assertThreadHasActiveGlContext();
        if (threadContextGroup->extensions.GL_ARB_invalidate_subdata) {
            threadContextGroup->functions.glInvalidateBufferData(id);
        } else if (!threadContextGroup->extensions.GL_ARB_buffer_storage) {
            //For buffer objects with non-immutable storage, a buffer can be invalidated by calling glBufferData with the exact same size and usage hint as before,
            //and with a NULL data​ parameter. This is an older method (hack) of invalidation, and it should only be used when the others are not available.
            GLenum usageHint = GL_DYNAMIC_DRAW;
            if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
                threadContextGroup->functions.glNamedBufferData(id, size_, 0, usageHint);
            } else if (threadContextGroup->extensions.GL_EXT_direct_state_access) {
                threadContextGroup->functions.glNamedBufferDataEXT(id, size_, 0, usageHint);
            } else {
                threadContext->cachedBindCopyWriteBuffer(id);
                threadContextGroup->functions.glBufferData(GL_COPY_WRITE_BUFFER, size_, 0, usageHint);
            }
        } else {
            //just ignore call, it is just a hind anyway...
        }
    }

    void BufferInterface::invalidate(
        uintptr_t offset,
        uintptr_t invalidateSize
    ) {
        Context::assertThreadHasActiveGlContext();
        if (threadContextGroup->extensions.GL_ARB_invalidate_subdata) {
            threadContextGroup->functions.glInvalidateBufferSubData(id, offset, invalidateSize);
        } else {
            //just ignore call, it is just a hind anyway...
        }
    }

    //TODO: we need two different functions for this, one that just removes the ID from the context states and another one that unbinds them actively!
    void BufferInterface::detachFromThreadContext() {
        Context::assertThreadHasActiveGlContext();
        /*
            TODO

            must actually be unbind if found in any current value!

            move this all into threadContext, something like
              detachBufferFromThreadContext(GLuint id);
            and make this function call it.

            should we make different functions for destructor and manual calls from users for when they want to give this object to some other thread?
              detachAndUnbindBufferFromThreadContext
        */
        if (threadContext) {
            LOOPI(threadContext->buffer_attribute_getHighestNonNull()) {
                if (threadContext->buffer_attribute_id    [i] == id) {
                    threadContext->buffer_attribute_id    [i] = 0;
                    threadContext->buffer_attribute_offset[i] = 0;
                }
            }
            if (threadContext->boundArrayBuffer) threadContext->cachedBindArrayBuffer(0);

            if (threadContext->buffer_attribute_index_id == id) {
                threadContext->buffer_attribute_index_id = 0;
            }

            if (threadContext->buffer_draw_indirect_id     == id) threadContext->buffer_draw_indirect_id     = 0;
            if (threadContext->buffer_dispatch_indirect_id == id) threadContext->buffer_dispatch_indirect_id = 0;
            if (threadContext->buffer_parameter_id         == id) threadContext->buffer_parameter_id         = 0;

            LOOPI(threadContext->buffer_uniform_getHighestNonNull()) {
                if (threadContext->buffer_uniform_id    [i] == id) {
                    threadContext->buffer_uniform_id    [i] = 0;
                    threadContext->buffer_uniform_offset[i] = 0;
                    threadContext->buffer_uniform_size  [i] = 0;
                }
            }

            if (threadContext->buffer_pixelPackId   == id) threadContext->buffer_pixelPackId   = 0;
            if (threadContext->buffer_pixelUnpackId == id) threadContext->buffer_pixelUnpackId = 0;
            if (threadContext->buffer_copyReadId    == id) threadContext->buffer_copyReadId    = 0;
            if (threadContext->buffer_copyWriteId   == id) threadContext->buffer_copyWriteId   = 0;
        }
    }

    /*void BufferInterface::setDebugLabel(const std::string& label) {
        Context::assertThreadHasActiveGlContext();
        if (threadContextGroup->extensions.GL_KHR_debug) {
            GLsizei lenght = std::min(GLsizei(label.size()), GLsizei(threadContextGroup->values.GL_MAX_LABEL_LENGTH));
            threadContextGroup->functions.glObjectLabel(GL_BUFFER, id, lenght, label.c_str());
            //int maxSize = threadContextGroup->values.GL_MAX_LABEL_LENGTH;
            //if (label.size() > maxSize) label.resize(maxSize);
            //threadContext->glObjectLabel(GL_BUFFER, id, -1, label.c_str());
        } else {
            //error?
        }
    }*/

    void* BufferInterface::create_(
        bool        clientMemoryCopyable,
        uintptr_t   size,
        bool        stagingBuffer,
        bool        sparseBuffer,
        const void* data
    ) {
        Context::assertThreadHasActiveGlContext();
        UNLIKELY_IF (stagingBuffer && !threadContextGroup->extensions.GL_ARB_buffer_storage)
            throw std::runtime_error("Staging buffer not supported (missing GL_ARB_buffer_storage)");
        UNLIKELY_IF (sparseBuffer  && !threadContextGroup->extensions.GL_ARB_sparse_buffer )
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

        if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
            threadContextGroup->functions.glCreateBuffers(1, &id);
            if (threadContextGroup->extensions.GL_ARB_buffer_storage) {
                threadContextGroup->functions.glNamedBufferStorage(id, size, data, flags);
                if (stagingBuffer) return threadContextGroup->functions.glMapNamedBufferRange(id, 0, size, stagingBufferAccessFlags);
            } else {
                threadContextGroup->functions.glNamedBufferData(id, size, data, usageHint);
            }
        } else {
            threadContextGroup->functions.glGenBuffers(1, &id);
            //Unlike for textures, the DSA EXT buffer functions can create buffers without having to bind them after glGenBuffers
            if (threadContextGroup->extensions.GL_EXT_direct_state_access) {
                if (threadContextGroup->extensions.GL_ARB_buffer_storage) {
                    threadContextGroup->functions.glNamedBufferStorageEXT(id, size, data, flags);
                    if (stagingBuffer) return threadContextGroup->functions.glMapNamedBufferRangeEXT(id, 0, size, stagingBufferAccessFlags);
                } else {
                    threadContextGroup->functions.glNamedBufferDataEXT(id, size, data, usageHint);
                }
            } else {
                threadContext->cachedBindCopyWriteBuffer(id);
                if (threadContextGroup->extensions.GL_ARB_buffer_storage) {
                    threadContextGroup->functions.glBufferStorage(GL_COPY_WRITE_BUFFER, size, data, flags);
                    if (stagingBuffer) return threadContextGroup->functions.glMapBufferRange(GL_COPY_WRITE_BUFFER, 0, size, stagingBufferAccessFlags);
                } else {
                    threadContextGroup->functions.glBufferData(GL_COPY_WRITE_BUFFER, size, data, usageHint);
                }
            }
        }
        this->size_                = size;
        this->clientMemoryCopyable = clientMemoryCopyable;
        return 0;
    }

    void BufferInterface::free() {
        //need something like this   if (std::is_static<*this>::value) return;
        //so I can ignore static objects destructor, because application is termination anyway and driver will clean up!
        if (id) {
            //if (!SDL_GL_GetCurrentContext())
            //  cout << "WARNING: glCompact::Buffer destructor called but no active OpenGL context in this thread to delete it! Leaking OpenGL object!" << endl;

            //detachFromThreadContext();
            //threadContext->glDeleteBuffers(1, &id);
            if (threadContext) detachFromThreadContext();

            if (threadContextGroup)
                threadContextGroup->functions.glDeleteBuffers(1, &id);

            id    = 0;
            size_ = 0;
        }
    }
}
