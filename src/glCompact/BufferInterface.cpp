#include "glCompact/BufferInterface.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/BufferGpu.hpp"
#include "glCompact/Context_.hpp"
#include "glCompact/threadContext_.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/threadContextGroup_.hpp"
#include "glCompact/Tools_.hpp"
#include "glCompact/PipelineCompute.hpp"
#include "glCompact/Debug.hpp"
#include "glCompact/minimumMaximum.hpp"

#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    /**
        \ingroup API
        \class glCompact::BufferInterface

        \brief OpenGL managed memory object

        \details This is the interface for all glCompact Buffer classes.
        It can manipulate the content of any buffer, but not the buffer allocation itself.
    */

    /**
       \brief Copy data from another buffer object.
    */
    void BufferInterface::copyFromBuffer(
        const BufferInterface& srcBuffer,
        uintptr_t              srcOffset,
        uintptr_t              dstOffset,
        uintptr_t              copySize
    ) {
        Debug::assertThreadHasActiveGlContext();
        auto throwWithInfo = [&](string error) {
            throw runtime_error(string() + "Error in\n"
                + "BufferInterface\n"
                + " .size = " + to_string(size) + "\n"
                + "::copyFromBuffer(\n"
                + " srcBuffer (.size = " + to_string(srcBuffer.size) + "),\n"
                + " srcOffset = " + to_string(srcOffset) + ",\n"
                + " dstOffset = " + to_string(dstOffset) + ",\n"
                + " copySize  = " + to_string(copySize) + "\n"
                + ")\n"
                + error);
        };
        UNLIKELY_IF (srcOffset            > srcBuffer.size) throwWithInfo("source offset is bayond source buffer size");
        UNLIKELY_IF (dstOffset            > this->size)     throwWithInfo("destination offset is bayond destination buffer size");
        UNLIKELY_IF (srcOffset + copySize > srcBuffer.size) throwWithInfo("offset + size is bayond source buffer size");
        UNLIKELY_IF (dstOffset + copySize > this->size)     throwWithInfo("offset + size is bayond destination buffer size");
        UNLIKELY_IF (copySize == 0) return;

        if (threadContextGroup_->extensions.GL_ARB_direct_state_access)
            threadContextGroup_->functions.glCopyNamedBufferSubData(srcBuffer.id, id, srcOffset, dstOffset, copySize);
        else {
            threadContext_->cachedBindCopyReadBuffer(srcBuffer.id);
            threadContext_->cachedBindCopyWriteBuffer(id);
            threadContextGroup_->functions.glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, copySize);
        }
    }

    string pipelineComputeCopyShader32String = R"""(#version 430
layout(local_size_x = 1024) in;

uniform uint srcOffset;
uniform uint dstOffset;
uniform uint size;

readonly layout(std430, binding=0) buffer srcBuffer {
    uint src[];
};
layout(std430, binding=1) buffer dstBuffer {
    uint dst[];
};

void main() {
    const uint groupIndex =
        gl_WorkGroupID.z * gl_NumWorkGroups.y * gl_NumWorkGroups.x +
        gl_WorkGroupID.y * gl_NumWorkGroups.x +
        gl_WorkGroupID.x;

    const uint localIndex =
        gl_LocalInvocationID.z * gl_WorkGroupSize.y * gl_WorkGroupSize.x +
        gl_LocalInvocationID.y * gl_WorkGroupSize.x +
        gl_LocalInvocationID.x;

    const uint globalIndex =
        localIndex +
        groupIndex * (gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z);

    const uint dstOffsetUint      = dstOffset / 4;
    const uint newSrcOffset       = srcOffset - (dstOffset % 4);
    const uint srcOffsetUint      = newSrcOffset / 4;
    const uint srcToDstRightShift = newSrcOffset % 4 * 8;
    const uint dstOffsetUintLast  = (dstOffset + size - 1) / 4;

    if (dstOffsetUint + globalIndex > dstOffsetUintLast) return;

    uint value;
    if (srcToDstRightShift == 0) {
        value = src[srcOffsetUint + globalIndex    ] >> (     srcToDstRightShift);
    } else {
        value = src[srcOffsetUint + globalIndex    ] >> (     srcToDstRightShift)
              | src[srcOffsetUint + globalIndex + 1] << (32 - srcToDstRightShift);
    }

    const uint maskFirstOffset = (dstOffset       ) % 4 * 8;
    const uint maskLastOffset  = (dstOffset + size) % 4 * 8;
    const bool maskFirst = (                globalIndex ==                 0) && (maskFirstOffset != 0);
    const bool maskLast  = (dstOffsetUint + globalIndex == dstOffsetUintLast) && (maskLastOffset  != 0);

    if (maskFirst || maskLast) {
        uint mask = 0xFFFFFFFF;
        if (maskFirst) mask =        uint(0xFFFFFFFF) << (     maskFirstOffset);
        if (maskLast ) mask = mask & uint(0xFFFFFFFF) >> (32 - maskLastOffset );
        if (mask != 0xFFFFFFFF)
            value = (value & mask) + (dst[dstOffsetUint + globalIndex] & ~mask);
    }

    dst[dstOffsetUint + globalIndex] = value;
})""";

    class PipelineComputeCopy32 : public PipelineCompute {
        public:
            using PipelineCompute::PipelineCompute;
            UniformSetter<uint32_t> srcOffset{this, "srcOffset"};
            UniformSetter<uint32_t> dstOffset{this, "dstOffset"};
            UniformSetter<uint32_t> size     {this, "size"};
    };

    /**
        \brief Copy data from another buffer object via compute shader

        GPUs often contain didicated parts for data transfer between system memory and VRAM. (BufferStaging and Buffer objects)
        This enables them to stream data from/to system memory while executing other workload in parallel.

        If no other workload is executed, it can be significantly faster bypassing this didicated parts and instead use the compute parts to directly copy the memory.

        Note that this is just a polished interface for a compute shader. This command depends on memory barriers to work correctly!
    */
    void BufferInterface::copyFromBufferViaPipelineCompute(
        const BufferInterface& srcBuffer,
        uintptr_t              srcOffset,
        uintptr_t              dstOffset,
        uintptr_t              copySize
    ) {
        Debug::assertThreadHasActiveGlContext();
        auto throwWithInfo = [&](string error) {
            throw runtime_error(string() + "Error in\n"
                + "BufferInterface\n"
                + " .size = " + to_string(size) + "\n"
                + "::copyFromBufferViaPipelineCompute(\n"
                + " srcBuffer (.size = " + to_string(srcBuffer.size) + "),\n"
                + " srcOffset = " + to_string(srcOffset) + ",\n"
                + " dstOffset = " + to_string(dstOffset) + ",\n"
                + " copySize  = " + to_string(copySize) + "\n"
                + ")\n"
                + error);
        };
        //TODO: Check for compute shader support
        UNLIKELY_IF (srcOffset            > srcBuffer.size) throwWithInfo("source offset is bayond source buffer size");
        UNLIKELY_IF (dstOffset            > this->size)     throwWithInfo("destination offset is bayond destination buffer size");
        UNLIKELY_IF (srcOffset + copySize > srcBuffer.size) throwWithInfo("offset + size is bayond source buffer size");
        UNLIKELY_IF (dstOffset + copySize > this->size)     throwWithInfo("offset + size is bayond destination buffer size");
        UNLIKELY_IF (copySize == 0) return;

        if (!threadContext_->pipelineComputeCopy) threadContext_->pipelineComputeCopy = new PipelineComputeCopy32(pipelineComputeCopyShader32String);
        PipelineComputeCopy32* pipelineComputeCopy32 = reinterpret_cast<PipelineComputeCopy32*>(threadContext_->pipelineComputeCopy);
        pipelineComputeCopy32->setShaderStorageBuffer(0, const_cast<BufferInterface&>(srcBuffer));
        pipelineComputeCopy32->setShaderStorageBuffer(1, *this);
        pipelineComputeCopy32->srcOffset = srcOffset;
        pipelineComputeCopy32->dstOffset = dstOffset;
        pipelineComputeCopy32->size      = copySize;
        pipelineComputeCopy32->dispatchMinGroupCount((copySize + 4096 - 1) / 4 / 1024);
    }

    /**
        \brief Copy data from client memory to a buffer object.

        This copy command takes care of any kind of syncronisation in this threads OpenGL context.
        Drivers might copy the client memory to OpenGL managed system memory to return from this function as fast as possible. To then continue to copy the data to device memory as soon
        after any other workload reading or writing to this buffer is done.
    */
    void BufferInterface::copyFromMemory(
        const void* srcMem,
        uintptr_t   thisOffset,
        uintptr_t   copySize
    ) {
        Debug::assertThreadHasActiveGlContext();
        auto throwWithInfo = [&](string error) {
            throw runtime_error(string() + "Error in\n"
                + "BufferInterface\n"
                + " .size = " + to_string(size) + "\n"
                + "::copyFromMemory(\n"
                + " srcMem     = " + to_string(uintptr_t(srcMem)) + ",\n"
                + " thisOffset = " + to_string(thisOffset) + ",\n"
                + " copySize   = " + to_string(copySize) + "\n"
                + ")\n"
                + error);
        };
        UNLIKELY_IF (thisOffset            >= this->size) throwWithInfo("offset is bayond buffer size");
        UNLIKELY_IF (thisOffset + copySize >  this->size) throwWithInfo("memory copy is bayond buffer size");
        UNLIKELY_IF (!clientMemoryCopyable)               throwWithInfo("buffer is not clientMemoryCopyable!");
        UNLIKELY_IF (copySize == 0) return;

        if (threadContextGroup_->extensions.GL_ARB_direct_state_access)
            threadContextGroup_->functions.glNamedBufferSubData(id, thisOffset, copySize, srcMem);
        else {
            threadContext_->cachedBindCopyWriteBuffer(id);
            threadContextGroup_->functions.glBufferSubData(GL_COPY_WRITE_BUFFER, thisOffset, copySize, srcMem);
        }
    }

    /**
        \brief Copy data from this buffer object to client memory

        This copy command takes care of any kind of synchronisation inside this threads OpenGL context.
        If any issued commands still write to the buffer, this function will stall until it can safely copy the data.
        For performance reasons it is best to use a fence after the last command writing to the buffer. And only after the fence signals, call this copy function.
    */
    void BufferInterface::copyToMemory(
        void*     destMem,
        uintptr_t thisOffset,
        uintptr_t copySize
    ) const {
        Debug::assertThreadHasActiveGlContext();
        auto throwWithInfo = [&](string error) {
            throw runtime_error(string() + "Error in\n"
                + "BufferInterface\n"
                + " .size = " + to_string(size) + "\n"
                + "::copyToMemory(\n"
                + " destMem    = " + to_string(uintptr_t(destMem)) + ",\n"
                + " thisOffset = " + to_string(thisOffset) + ",\n"
                + " copySize   = " + to_string(copySize) + "\n"
                + ")\n"
                + error);
        };
        UNLIKELY_IF (thisOffset            >= this->size) throwWithInfo("offset is bayond buffer size");
        UNLIKELY_IF (thisOffset + copySize >  this->size) throwWithInfo("memory copy is bayond buffer size");
        UNLIKELY_IF (!clientMemoryCopyable)               throwWithInfo("buffer is not clientMemoryCopyable!");
        UNLIKELY_IF (copySize == 0) return;

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
        Debug::assertThreadHasActiveGlContext();
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
        Debug::assertThreadHasActiveGlContext();
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
        Debug::assertThreadHasActiveGlContext();
        auto throwWithInfo = [&](string error) {
            throw runtime_error(string() + "Error in\n"
                + "BufferInterface\n"
                + " .size = " + to_string(size) + "\n"
                + "::clear(\n"
                + " offset        = " + to_string(offset) + ",\n"
                + " clearSize     = " + to_string(clearSize) + ",\n"
                + " fillValueSize = " + to_string(fillValueSize) + ")\n"
                + " fillValue     = " + to_string(uintptr_t(fillValue)) + ")\n"
                + error);
        };
        UNLIKELY_IF (offset      % fillValueSize != 0) throwWithInfo("offset must be aligned to size of fillValue!");
        UNLIKELY_IF (clearSize   % fillValueSize != 0) throwWithInfo("clearSize must be aligned to size of fillValue!");
        UNLIKELY_IF (offset             >= this->size) throwWithInfo("offset is bayond size of buffer");
        UNLIKELY_IF (offset + clearSize >  this->size) throwWithInfo("trying to clear bayond buffer size");
        UNLIKELY_IF (clearSize == 0) return;

        if (threadContextGroup_->extensions.GL_ARB_clear_buffer_object) {
            struct {
                GLenum internalFormat;
                GLenum componentArrangement;
                GLenum componentTypes;
            } param;
            switch (fillValueSize) {
                case  1: param = {GL_R8,       GL_RED,          GL_UNSIGNED_BYTE }; break;
                case  2: param = {GL_RG8,      GL_RED,          GL_UNSIGNED_BYTE }; break;
                case  4: param = {GL_RGBA8,    GL_RGBA,         GL_UNSIGNED_BYTE }; break;
                case  8: param = {GL_RGBA16,   GL_RGBA,         GL_UNSIGNED_SHORT}; break;
                case 12: param = {GL_RGB32UI,  GL_RGB_INTEGER,  GL_UNSIGNED_INT  }; break; //GL_RGB_INTEGER: does not exist in 3.0, is there a non integer format with 12 bytes?
                case 16: param = {GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT  }; break;
                default:
                    throw std::runtime_error("fillValueSize must be 1, 2, 4, 8, 12 or 16!");
            }
            if (threadContextGroup_->extensions.GL_ARB_clear_buffer_object) {
                if (threadContextGroup_->extensions.GL_ARB_direct_state_access)
                    threadContextGroup_->functions.glClearNamedBufferSubData       (this->id, param.internalFormat, offset, clearSize, param.componentArrangement, param.componentTypes, fillValue);
                else {
                    threadContext_->cachedBindCopyWriteBuffer(this->id);
                    threadContextGroup_->functions.glClearBufferSubData(GL_COPY_WRITE_BUFFER, param.internalFormat, offset, clearSize, param.componentArrangement, param.componentTypes, fillValue);
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
                BufferGpu buffer(false, fillValueSize, fillValue);
                copyFromBuffer(buffer, 0, offset, fillValueSize);
            }
            uintptr_t writeOffset  = offset    + fillValueSize;
            uintptr_t bytesToWrite = clearSize - fillValueSize;
            while (bytesToWrite) {
                uintptr_t copySize = minimum(writeOffset - offset, bytesToWrite);
                copyFromBuffer(*this, offset, writeOffset, copySize);
                writeOffset  += copySize;
                bytesToWrite -= copySize;
            }
        }
    }

    /**
        \brief invalidate the content of this buffer

        This is a performance hint that depends on GL_ARB_invalidate_subdata (core since GL4.3) being present. Without the extension this function will currently do nothing.
        TODO: Might emulte invalidating by recreating the buffer object if using non-immutable buffers?

        It allowes the driver:
            - to free the memory when under memory pressure
            - to allocate new memory to immediately start following commands when the buffer is still in use by previous commands

        After this the content of the buffer is undefined until something is written to it.
    */
    void BufferInterface::invalidate() {
        Debug::assertThreadHasActiveGlContext();
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
        Debug::assertThreadHasActiveGlContext();
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
            GLsizei lenght = minimum(GLsizei(label.size()), GLsizei(threadContextGroup_->values.GL_MAX_LABEL_LENGTH));
            threadContextGroup_->functions.glObjectLabel(GL_BUFFER, id, lenght, label.c_str());
            //int maxSize = threadContextGroup_->values.GL_MAX_LABEL_LENGTH;
            //if (label.size() > maxSize) label.resize(maxSize);
            //threadContext->glObjectLabel(GL_BUFFER, id, -1, label.c_str());
        } else {
            //error?
        }
    }*/

    /*
        TODO: what happens with sparse buffers that get data to initalize? Is the data ignored or does the full buffer size gets commited automaticaly?

        Drivers mostly ignore all the hint parameters and do what they want depending on buffer usage patterns.
    */
    void* BufferInterface::create(
        bool        clientMemoryCopyable,
        uintptr_t   size,
        bool        stagingBuffer,
        bool        sparseBuffer,
        const void* data
    ) {
        Debug::assertThreadHasActiveGlContext();
        UNLIKELY_IF (stagingBuffer && !threadContextGroup_->extensions.GL_ARB_buffer_storage) crash("Staging buffer not supported (missing GL_ARB_buffer_storage)");
        UNLIKELY_IF (sparseBuffer  && !threadContextGroup_->extensions.GL_ARB_sparse_buffer ) crash("Sparse buffer is not supported (missing GL_ARB_sparse_buffer)");

        uint32_t flags =
            clientMemoryCopyable ? GL_DYNAMIC_STORAGE_BIT    : 0 //With this bit set, glBufferSubData can change the content of the buffer
        |   sparseBuffer         ? GL_SPARSE_STORAGE_BIT_ARB : 0
        |   stagingBuffer        ? GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_CLIENT_STORAGE_BIT : 0; //GL_CLIENT_STORAGE_BIT is just a hint!
        uint32_t stagingBufferAccessFlags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;
        uint32_t usageHint                = GL_DYNAMIC_DRAW;
        this->size                 = size;
        this->clientMemoryCopyable = clientMemoryCopyable;
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
        return 0;
    }

    void BufferInterface::free() {
        //need something like this   if (std::is_static<*this>::value) return;
        //so I can ignore destructors from static objects, because the application is termination anyway and driver will clean up!
        //TODO: When do static objects from a closing thread get destroyed when the application keeps running?
        if (id) {
            UNLIKELY_IF (!threadContextGroup_) crash("glCompact::Buffer destructor called but thread has no reference to threadContextGroup_! Leaking OpenGL object!");
            #ifdef GLCOMPACT_DEBUG_ASSERT_THREAD_HAS_ACTIVE_CONTEXT
                UNLIKELY_IF (!threadContextGroup_->functions.glGetString(GL_VERSION)) crash("glCompact::Buffer destructor called but thread has no active OpenGL context! glDeleteBuffers without effect! Leaking OpenGL object!");
            #endif

            if (threadContext_) threadContext_->forgetBufferId(id);
            threadContextGroup_->functions.glDeleteBuffers(1, &id);

            id    = 0;
            size = 0;
            clientMemoryCopyable = false;
        }
    }
}
