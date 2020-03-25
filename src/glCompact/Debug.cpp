#include "glCompact/Debug.hpp"
//#include "glCompact/ThreadContext.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include <iostream>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    std::string Debug::sourceToString(
        GLenum source
    ) {
        switch (source) {
            case GL_DEBUG_SOURCE_API:               return "API";             //opengl calls
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     return "window system";   //glx/wgl
            case GL_DEBUG_SOURCE_SHADER_COMPILER:   return "shader compiler";
            case GL_DEBUG_SOURCE_THIRD_PARTY:       return "third party";
            case GL_DEBUG_SOURCE_APPLICATION:       return "application";     //self injected
            case GL_DEBUG_SOURCE_OTHER:             return "other";
            default: return "unknown source(" + to_string(source) + ")";
        }
    }

    std::string Debug::typeToString(
        GLenum type
    ) {
        switch (type) {
            case GL_DEBUG_TYPE_ERROR:               return "error";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "deprecated behavior";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "undefined behavior";
            case GL_DEBUG_TYPE_PORTABILITY:         return "portability";
            case GL_DEBUG_TYPE_PERFORMANCE:         return "performance";
            case GL_DEBUG_TYPE_MARKER:              return "marker"; //Command stream annotation?
            case GL_DEBUG_TYPE_PUSH_GROUP:          return "push group";
            case GL_DEBUG_TYPE_POP_GROUP:           return "pop group";
            case GL_DEBUG_TYPE_OTHER:               return "other";
            default: return "unknown type(" + to_string(type) + ")";
        }
    }

    std::string Debug::severityToString(
        GLenum severity
    ) {
        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:            return "high";         //An error, typically from the API
            case GL_DEBUG_SEVERITY_MEDIUM:          return "medium";       //Some behavior marked deprecated has been used
            case GL_DEBUG_SEVERITY_LOW:             return "low";          //Something has invoked undefined behavior
            case GL_DEBUG_SEVERITY_NOTIFICATION:    return "notification"; //Some functionality the user relies upon is not portable
            default: return "unknown severity(" + to_string(severity) + ")";
        }
    }

    #if defined(_WIN32)
        #define IF_MSVC_THEN_STDCALL_HERE __stdcall
    #else
        #define IF_MSVC_THEN_STDCALL_HERE
    #endif
    void IF_MSVC_THEN_STDCALL_HERE Debug::coutKhrDebugMessage(
        GLenum        source,
        GLenum        type,
        GLuint        id,
        GLenum        severity,
        GLsizei       length,
        const GLchar* message,
        const void*   userParam
    ) {
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
        throw std::runtime_error("glDebugMessageCallback source(" + sourceToString(source) + ") type(" + typeToString(type) + ") severity(" + severityToString(severity) + ")\n" + message);
    }

    //NOTE: if the GL context is not a debug context you maybe get no messages at all, even with glEnable(GL_DEBUG_OUTPUT)!
    //TODO: Also use GL_KHR_no_error if available for better performance if not in debug mode!
    void Debug::enableDebugOutput() {
        //There also is ATI_debug_output and ARB_debug_output, but we may never use them because GL_KHR_debug got implemented by all current drivers and is part of core.
        if (threadContextGroup_->extensions.GL_KHR_debug) {
            cout << "GL_KHR_debug found, registering debug callback function" << endl;
            threadContextGroup_->functions.glDebugMessageCallback(&coutKhrDebugMessage, 0);
            threadContextGroup_->functions.glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); //supposedly ruins performance, but gives callback in same thread as context after API call. So we are able to get a nice backtrace from where the call came from.

            //MESA also needs this to be enabled in debug context to generate output
            //In non debug context the driver is free to chose if he enables output at all. Or if the driver even exposes GL_KHR_debug string in the first place.
            threadContextGroup_->functions.glEnable(GL_DEBUG_OUTPUT);
        } else {
            cout << "GL_KHR_debug not available" << endl;
        }
    }

    void Debug::setWarningCallback(
        WarningFunc warningFunc
    ) {
        Debug::warningFunc = warningFunc;
    }

    void Debug::defaultWarningFunc(
        std::string message
    ) {
        cout << message << endl;
    }

    Debug::WarningFunc Debug::warningFunc = Debug::defaultWarningFunc;
}
