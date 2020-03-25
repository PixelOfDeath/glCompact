#include "glCompact/BufferSparse.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/Context.hpp"
#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup_.hpp"

#include <stdexcept>

/**
    Needs GL_ARB_sparse_buffer (not part of any core standard)
*/

using namespace glCompact::gl;

namespace glCompact {
    BufferSparse::BufferSparse(
        bool      clientMemoryCopyable,
        uintptr_t size
    ) {
        create(clientMemoryCopyable, size);
    }

    BufferSparse::BufferSparse(
        const BufferSparse& buffer
    ) {
        create(buffer.clientMemoryCopyable, buffer.size_);
        copyFromBuffer(buffer, 0, 0, buffer.size_);
    }

    BufferSparse::BufferSparse(BufferSparse&& buffer) {
        id           = buffer.id;
        size_        = buffer.size_;
        buffer.id    = 0;
        buffer.size_ = 0;
    }

    BufferSparse& BufferSparse::operator=(
        const BufferSparse& buffer
    ) {
        if (this != &buffer) {
            create(buffer.clientMemoryCopyable, buffer.size_);
            copyFromBuffer(buffer, 0, 0, buffer.size_);
        }
        return *this;
    }

    BufferSparse& BufferSparse::operator=(
        BufferSparse&& buffer
    ) {
        if (this != &buffer) {
            free();
            id           = buffer.id;
            size_        = buffer.size_;
            buffer.id    = 0;
            buffer.size_ = 0;
        }
        return *this;
    }

    BufferSparse::~BufferSparse() {
        free();
    }

    void BufferSparse::free() {
        BufferInterface::free();
    }

    //Max size is 64 KiB. Also what AMD drivers use on 290.
    uintptr_t BufferSparse::getPageSize() {
        Context::assertThreadHasActiveGlContext();
        return threadContextGroup_->values.GL_SPARSE_BUFFER_PAGE_SIZE_ARB;
    }

    void BufferSparse::commitment(
        uintptr_t offset,
        uintptr_t size,
        bool      commit
    ) {
        Context::assertThreadHasActiveGlContext();
        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            threadContextGroup_->functions.glNamedBufferPageCommitmentARB(id, offset, size, commit);
        } else if (threadContextGroup_->extensions.GL_EXT_direct_state_access) {
            threadContextGroup_->functions.glNamedBufferPageCommitmentEXT(id, offset, size, commit);
        } else {
            threadContext->cachedBindCopyWriteBuffer(id);
            threadContextGroup_->functions.glBufferPageCommitmentARB(GL_COPY_WRITE_BUFFER, offset, size, commit);
        }
    }

    void BufferSparse::create(
        bool      clientMemoryCopyable,
        uintptr_t size
    ) {
        free();
        create_(clientMemoryCopyable, size, false, true);
    }
}
