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
///\cond HIDDEN_FROM_DOXYGEN
#pragma once
#include <cstdint> //C++11
#include <string>

namespace glCompact {
    class Debug {
            friend class PipelineInterface;
        public:
            static void enableDebugOutput();

            static std::string sourceToString(int32_t source);
            static std::string typeToString(int32_t type);
            static std::string severityToString(int32_t severity);

            #if defined(_WIN32)
                #define STDCALL __stdcall
            #else
                #define STDCALL
            #endif
            static void STDCALL coutKhrDebugMessage(int32_t source, int32_t type, uint32_t id, int32_t severity, uint32_t length, const char* message, const void* userParam);
            #undef STDCALL

            typedef void(*WarningFunc)(std::string message);
            void setWarningCallback(WarningFunc warningFunc);
        private:
            static void defaultWarningFunc(std::string message);
            static WarningFunc warningFunc;
    };
}
///\endcond
