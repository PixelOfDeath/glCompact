#!/usr/bin/python

import sys
import xml.etree.ElementTree as ET

if len(sys.argv) < 3:
    print("need parameter 'xml source' and 'output file'")
    exit()

xmlFile    = sys.argv[1]
outputFile = sys.argv[2]

root = ET.parse(xmlFile).getroot()

functionNameList = []

for enums in root.findall("commands"):
    for enum in enums.findall("command"):
        name = enum.find("proto").find("name")
        nameRaw  = name.text
        functionNameList.append(nameRaw)
        #nameProc = nameRaw.upper() + "PROC"

functionNameMaxLen = max(len(x) for x in functionNameList)
getFunctionPointerList = []
for functionName in functionNameList:
    functionNameLong = functionName.ljust(functionNameMaxLen)
    functionNameProc = (functionName.upper() + "PROC").ljust(functionNameMaxLen + 4)
    getFunctionPointerString = "            " + functionNameLong + " = reinterpret_cast<" + functionNameProc + '>(getGlFunctionPointer("' + functionName + '"));'
    getFunctionPointerList.append(getFunctionPointerString)

outputTemplate = """#include "glCompact/gl/Functions.hpp"
#include "glCompact/gl/FunctionsTypedef.hpp"

namespace glCompact {
    namespace gl {
        void Functions::init(
            void*(*getGlFunctionPointer)(const char* glFunctionName)
        ) {
            //glName = reinterpret_cast<glNamePROC>(getGlFunctionPointer("glName"));
            ///GET_FUNCTION_POINTER_LIST
        }
    }
}
"""

output = outputTemplate.replace("            ///GET_FUNCTION_POINTER_LIST", "\n".join(getFunctionPointerList))
open(outputFile, 'w').write(output)
