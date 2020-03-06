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
///\endcond
