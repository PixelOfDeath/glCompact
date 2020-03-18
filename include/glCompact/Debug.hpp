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
