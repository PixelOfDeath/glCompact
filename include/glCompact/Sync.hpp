/*
    This file is part of glCompact.
    Copyright (C) 2019-2020 Frederik Uje vom Hofe

    glCompact is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    glCompact is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
*/
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
        ///\cond HIDDEN_FROM_DOXYGEN
        private:
            void* syncObj;
            bool glClientWaitSyncDidFlushAlready;
        ///\endcond
    };
}
