#pragma once
#include "glCompact/BufferInterface.hpp"

#include <vector>

namespace glCompact {
    class BufferGpuSparse;
    class BufferGpuSparse : public BufferInterface {
        public:
            BufferGpuSparse           () = default;
            BufferGpuSparse           (bool clientMemoryCopyable, uintptr_t size);
            BufferGpuSparse           (const BufferGpuSparse&  buffer);
            BufferGpuSparse           (      BufferGpuSparse&& buffer);
            BufferGpuSparse& operator=(const BufferGpuSparse&  buffer);
            BufferGpuSparse& operator=(      BufferGpuSparse&& buffer);
            ~BufferGpuSparse();
            void free();

            void setCommitment(uintptr_t offset, uintptr_t size, bool commit);
            uintptr_t getCommitmentSize() const {return commitmentSize;}

            static constexpr uintptr_t pageSize = 65536;
        private:
            uintptr_t commitmentSize = 0;
            std::vector<bool> commitmentMap;

            void setCommitment_(uintptr_t offset, uintptr_t size, bool commit);
            void copyCommitment(const BufferGpuSparse& buffer);
            void copyFromBufferCommitmentRegionOnly(const BufferGpuSparse& buffer);
    };
}
