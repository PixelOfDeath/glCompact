#pragma once
#include "glCompact/BufferInterface.hpp"

#include <vector>

namespace glCompact {
    class BufferGpuVirtual;
    class BufferGpuVirtual : public BufferInterface {
        public:
            BufferGpuVirtual           () = default;
            BufferGpuVirtual           (bool clientMemoryCopyable, uintptr_t size);
            BufferGpuVirtual           (const BufferGpuVirtual&  buffer);
            BufferGpuVirtual           (      BufferGpuVirtual&& buffer);
            BufferGpuVirtual& operator=(const BufferGpuVirtual&  buffer);
            BufferGpuVirtual& operator=(      BufferGpuVirtual&& buffer);
            ~BufferGpuVirtual();
            void free();

            void setCommitment(uintptr_t offset, uintptr_t size, bool commit);
            uintptr_t getCommitmentSize() const {return commitmentSize;}

            static constexpr uintptr_t pageSize = 65536;
        private:
            uintptr_t commitmentSize = 0;
            std::vector<bool> commitmentMap;

            void setCommitment_(uintptr_t offset, uintptr_t size, bool commit);
            void copyCommitment(const BufferGpuVirtual& buffer);
            void copyFromBufferCommitmentRegionOnly(const BufferGpuVirtual& buffer);
    };
}
