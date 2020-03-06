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
