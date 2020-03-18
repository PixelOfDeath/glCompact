#pragma once
#include <cstdint> //C++11

namespace glCompact {
    class Sync {
            friend class ContextInterface;
        public:
            Sync();
            ~Sync();

            void insert();
            void free();

            bool waitForSync(bool flushIfUnsignaled, uint64_t timeout = 0xFFFFFFFFFFFFFFFF); //GL_TIMEOUT_IGNORED
            bool isSignaled();
            void withholdFutureCommandsUntilSync();
        private:
            void* syncObj;
            bool glClientWaitSyncDidFlushAlready;
    };
}
