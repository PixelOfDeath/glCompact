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
#include "glCompact/BufferSparse.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"

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
        //TODO...
    }

    void BufferSparse::free() {
        BufferInterface::free();
    }

    //Max size is 64 KiB. Also what AMD drivers use on 290.
    uintptr_t BufferSparse::getPageSize() {
        Context::throwIfThreadHasNoActiveContext();
        return threadContextGroup->values.GL_SPARSE_BUFFER_PAGE_SIZE_ARB;
    }

    void BufferSparse::commitment(
        uintptr_t offset,
        uintptr_t size,
        bool      commit
    ) {
        Context::throwIfThreadHasNoActiveContext();
        if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
            threadContextGroup->functions.glNamedBufferPageCommitmentARB(id, offset, size, commit);
        } else if (threadContextGroup->extensions.GL_EXT_direct_state_access) {
            threadContextGroup->functions.glNamedBufferPageCommitmentEXT(id, offset, size, commit);
        } else {
            threadContext->cachedBindCopyWriteBuffer(id);
            threadContextGroup->functions.glBufferPageCommitmentARB(GL_COPY_WRITE_BUFFER, offset, size, commit);
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
