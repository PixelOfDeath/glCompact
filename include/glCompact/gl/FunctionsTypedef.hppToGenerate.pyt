#!/usr/bin/python

import sys
import xml.etree.ElementTree as ET

if len(sys.argv) < 3:
    print("need parameter 'xml source' and 'output file'")
    exit()

xmlFile    = sys.argv[1]
outputFile = sys.argv[2]

root = ET.parse(xmlFile).getroot()

functionTypedefReturnTypeList    = []
functionTypedefNameList          = []
functionTypedefParameterListList = list(list())

for enums in root.findall("commands"):
    for enum in enums.findall("command"):
        returnType = ""
        proto = enum.find("proto")
        ptype = proto.find("ptype")
        if                      (proto.text != None) : returnType += proto.text
        if ((ptype != None) and (ptype.text != None)): returnType += ptype.text
        if ((ptype != None) and (ptype.tail != None) and (ptype.tail.strip() != "")): returnType += ptype.tail.strip()

        name = proto.find("name").text
        name = name.upper() + "PROC"

        functionTypedefReturnTypeList.append(returnType)
        functionTypedefNameList.append(name)

        functionTypedefParameterList = []
        for param in enum.findall("param"):
            ptype = param.find("ptype")
            paramType = ""
            if                      (param.text != None)                                : paramType += param.text
            if ((ptype != None) and (ptype.text != None))                               : paramType += ptype.text
            if ((ptype != None) and (ptype.tail != None) and (ptype.tail.strip() != "")): paramType += ptype.tail.strip()

            name = param.find("name")
            paramName = name.text

            functionTypedefParameterList.append(paramType + " " + paramName)
        functionTypedefParameterListList.append(functionTypedefParameterList)

functionTypedefList = []

functionTypedefReturnTypeMaxLen = max(len(x) for x in functionTypedefReturnTypeList)
functionTypedefNameMaxLen       = max(len(x) for x in functionTypedefNameList)
i = 0
while i < len(functionTypedefReturnTypeList):
    funcTypeDefReturnType = functionTypedefReturnTypeList[i].ljust(functionTypedefReturnTypeMaxLen)
    funcTypeDefNameLong   = functionTypedefNameList[i].ljust(functionTypedefNameMaxLen)
    fullTypeDef = "        EXTERNC typedef " + funcTypeDefReturnType + "(STDCALL *" + funcTypeDefNameLong + ")("
    first = True
    for param in functionTypedefParameterListList[i]:
        if (not first):
            fullTypeDef += ", "
        first = False
        fullTypeDef += param
    fullTypeDef += ");"
    functionTypedefList.append(fullTypeDef)
    i += 1

outputTemplate = """#pragma once
#include "glCompact/gl/Types.hpp"
#include "glCompact/gl/Constants.hpp"

namespace glCompact {
    namespace gl {
        //OpenGL uses stdcall, but msvc default is cdecl
        #if defined(_WIN32)
            #define EXTERNC
            #define STDCALL __stdcall
        #else
            //GCC
            #define EXTERNC extern "C"
            #define STDCALL
        #endif

        typedef void (STDCALL *GLDEBUGPROC)   (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
        typedef void (STDCALL *GLDEBUGPROCARB)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
        typedef void (STDCALL *GLDEBUGPROCKHR)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
        typedef void (STDCALL *GLDEBUGPROCAMD)(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar *message, void *userParam);

        //Part of GL_EXT_external_buffer (GLeglClientBufferEXT is an opaque handle to an EGLClientBuffer)
        typedef void *GLeglClientBufferEXT;

        //part of GL_NV_draw_vulkan_image
        typedef void (STDCALL *GLVULKANPROCNV)(void);

        //EXTERNC typedef RETURNVALUE   (STDCALL *glNamePROC)(PARAM);
        ///FUNCTION_TYPEDEF_LIST

        #undef EXTERNC
        #undef STDCALL
    }
}
"""

output = outputTemplate.replace("        ///FUNCTION_TYPEDEF_LIST", "\n".join(functionTypedefList))
open(outputFile, 'w').write(output)
