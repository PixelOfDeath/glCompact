#include "glCompact/BufferSparse.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/Context.hpp"
#include "glCompact/ThreadContext.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/ThreadContextGroup_.hpp"

#include <stdexcept>

/**
    Needs GL_ARB_sparse_buffer (not part of any core standard)

    threadContextGroup_->values.GL_SPARSE_BUFFER_PAGE_SIZE_ARB (max. allowed value by the standard is 65536 (64 KiB))
*/

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    BufferSparse::BufferSparse(
        bool      clientMemoryCopyable,
        uintptr_t size
    ) :
        commitmentMap((size + pageSize - 1) / pageSize)
    {
        UNLIKELY_IF (size % pageSize)
            crash("size must be a multiple of pageSize(" + to_string(pageSize) + ")");
        create(clientMemoryCopyable, align(size, pageSize), false, true);
    }

    BufferSparse::BufferSparse(
        const BufferSparse& buffer
    ) {
        create(buffer.clientMemoryCopyable, buffer.size, false, true);
        copyCommitment(buffer);
        //copyFromBuffer(buffer, 0, 0, buffer.size);
        copyFromBufferCommitmentRegionOnly(buffer);
    }

    BufferSparse::BufferSparse(
        BufferSparse&& buffer
    ) {
        id                   = buffer.id;
        size                 = buffer.size;
        clientMemoryCopyable = buffer.clientMemoryCopyable;
        commitmentSize       = buffer.commitmentSize;
        commitmentMap        = move(buffer.commitmentMap);
        buffer.id                   = 0;
        buffer.size                 = 0;
        buffer.clientMemoryCopyable = false;
        buffer.commitmentSize       = 0;
    }

    BufferSparse& BufferSparse::operator=(
        const BufferSparse& buffer
    ) {
        if (this != &buffer) {
            free();
            create(buffer.clientMemoryCopyable, buffer.size, false, true);
            copyCommitment(buffer);
            //copyFromBuffer(buffer, 0, 0, buffer.size);
            copyFromBufferCommitmentRegionOnly(buffer);
        }
        return *this;
    }

    BufferSparse& BufferSparse::operator=(
        BufferSparse&& buffer
    ) {
        free();
        id                   = buffer.id;
        size                 = buffer.size;
        clientMemoryCopyable = buffer.clientMemoryCopyable;
        commitmentSize       = buffer.commitmentSize;
        commitmentMap        = move(buffer.commitmentMap);
        buffer.id                   = 0;
        buffer.size                 = 0;
        buffer.clientMemoryCopyable = false;
        buffer.commitmentSize       = 0;
        return *this;
    }

    BufferSparse::~BufferSparse() {
        free();
    }

    void BufferSparse::free() {
        BufferInterface::free();
        commitmentSize = 0;
        commitmentMap.clear();
    }

    void BufferSparse::setCommitment(
        uintptr_t offset,
        uintptr_t size,
        bool      commit
    ) {
        UNLIKELY_IF (offset % pageSize)
            crash("offset must be a multiple pageSize(" + to_string(pageSize) + ")");
        UNLIKELY_IF (size % pageSize)
            crash("size must be a multiple pageSize(" + to_string(pageSize) + ")");

        if (!size) return;

        uintptr_t commitmentSizeDiff = 0;
        int start = offset / pageSize;
        int end   = start + (size / pageSize);
        if (commit) {
            for (int i = start; i < end; ++i) {
                if (!commitmentMap[i]) commitmentSizeDiff += pageSize;
                commitmentMap[i] = true;
            }
        } else {
            for (int i = start; i < end; ++i) {
                if (commitmentMap[i]) commitmentSizeDiff -= pageSize;
                commitmentMap[i] = false;
            }
        }
        if (commitmentSizeDiff) {
            commitmentSize += commitmentSizeDiff;
            setCommitment_(offset, size, commit);
        }
    }

    void BufferSparse::setCommitment_(
        uintptr_t offset,
        uintptr_t size,
        bool      commit
    ) {
        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            threadContextGroup_->functions.glNamedBufferPageCommitmentARB(id, offset, size, commit);
        } else {
            threadContext->cachedBindCopyWriteBuffer(id);
            threadContextGroup_->functions.glBufferPageCommitmentARB(GL_COPY_WRITE_BUFFER, offset, size, commit);
        }
    }

    void BufferSparse::copyCommitment(
        const BufferSparse& buffer
    ) {
        int currentPos = 0;
        int endPos = buffer.size / pageSize;

        while (currentPos < endPos) {
            while (buffer.commitmentMap[currentPos] == false && currentPos < endPos) currentPos++;
            int commitRegionStart = currentPos;
            while (buffer.commitmentMap[currentPos] == true  && currentPos < endPos) currentPos++;
            int commitRegionEnd   = currentPos;
            if (commitRegionStart != commitRegionEnd)
                setCommitment_(commitRegionStart * pageSize, (commitRegionEnd - commitRegionStart) * pageSize, true);
        }
        commitmentSize = buffer.commitmentSize;
        commitmentMap  = buffer.commitmentMap;
    }

    void BufferSparse::copyFromBufferCommitmentRegionOnly(
        const BufferSparse& buffer
    ) {
        int currentPos = 0;
        int endPos = buffer.size / pageSize;

        while (currentPos < endPos) {
            while (buffer.commitmentMap[currentPos] == false && currentPos < endPos) currentPos++;
            int commitRegionStart = currentPos;
            while (buffer.commitmentMap[currentPos] == true  && currentPos < endPos) currentPos++;
            int commitRegionEnd   = currentPos;
            if (commitRegionStart != commitRegionEnd)
                copyFromBuffer(buffer, commitRegionStart * pageSize, commitRegionStart * pageSize, (commitRegionEnd - commitRegionStart) * pageSize);
        }
    }
}
