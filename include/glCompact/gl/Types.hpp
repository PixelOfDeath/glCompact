#pragma once
#include <cstddef>
#include <cstdint> //C++11

namespace glCompact {
    namespace gl {
        typedef int32_t  GLenum;
        typedef uint8_t  GLboolean;
        typedef uint32_t GLbitfield;
      //typedef void     GLvoid; //Not an actual GL type, though used in headers in the past
        typedef int8_t   GLbyte;
        typedef int16_t  GLshort;
        typedef int32_t  GLint;
        typedef int32_t  GLclampx; //Apple OpenGL ES
        typedef uint8_t  GLubyte;
        typedef uint16_t GLushort;
        typedef uint32_t GLuint;
        typedef uint32_t GLsizei;
        typedef float    GLfloat;
        typedef float    GLclampf; //depricated in Core 4.2 into GLfloat
        typedef double   GLdouble;
        typedef double   GLclampd; //depricated in Core 4.2 into GLdouble
        typedef void*    GLeglImageOES;
        typedef char     GLchar;
        typedef char     GLcharARB;

        #ifdef __APPLE__
            typedef void* GLhandleARB;
        #else
            typedef unsigned int GLhandleARB;
        #endif

        typedef uint16_t GLhalfARB;
        //Half-precision floating-point value encoded in an unsigned scalar
        //GLM includes a float-half version, maybe include that somehwo? http://glm.g-truc.net/0.9.3/api/a00143.html
        typedef uint16_t GLhalf;
        typedef int32_t  GLfixed;

        typedef size_t   GLintptr;
        typedef size_t   GLsizeiptr;
        typedef size_t   GLintptrARB;
        typedef size_t   GLsizeiptrARB;

        typedef int64_t  GLint64;
        typedef uint64_t GLuint64;
        typedef int64_t  GLint64EXT;
        typedef uint64_t GLuint64EXT;

        typedef uint16_t GLhalfNV;
        typedef GLintptr GLvdpauSurfaceNV;

        typedef void*    GLsync;

        //compatible with OpenCL cl_context
        struct _cl_context;
        //cl_event
        struct _cl_event;
    }
}
