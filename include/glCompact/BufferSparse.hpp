#pragma once
#include "glCompact/BufferInterface.hpp"

#include <vector>

namespace glCompact {
    class BufferSparse;
    class BufferSparse : public BufferInterface {
        public:
            BufferSparse           () = default;
            BufferSparse           (bool clientMemoryCopyable, uintptr_t size);
            BufferSparse           (const BufferSparse&  buffer);
            BufferSparse           (      BufferSparse&& buffer);
            BufferSparse& operator=(const BufferSparse&  buffer);
            BufferSparse& operator=(      BufferSparse&& buffer);
            virtual ~BufferSparse() final;
            void free();

            void setCommitment(uintptr_t offset, uintptr_t size, bool commit);
            uintptr_t getCommitmentSize() const {return commitmentSize;}

            static constexpr uintptr_t pageSize = 65536;
        private:
            uintptr_t commitmentSize = 0;
            std::vector<bool> commitmentMap;

            void setCommitment_(uintptr_t offset, uintptr_t size, bool commit);
            void copyCommitment(const BufferSparse& buffer);
            void copyFromBufferCommitmentRegionOnly(const BufferSparse& buffer);
    };
}
