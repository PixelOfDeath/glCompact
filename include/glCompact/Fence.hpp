#pragma once
#include <cstdint> //C++11

namespace glCompact {
    class Fence {
        public:
            Fence() = default;
            ~Fence();

            void insert();
            void free();

            bool isSignaled();
            bool isSignaledOrWait(uint64_t timeoutInNanoseconds = ~0);
            void isSignaledOrStallCommandStream();
        private:
            void* fenceObj = 0;
            bool glClientWaitSyncDidFlush = false;
    };
}
