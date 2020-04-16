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
functionDefinitionList = []
for functionName in functionNameList:
    functionNameProc = (functionName.upper() + "PROC").ljust(functionNameMaxLen + 4)
    queryFunctionString = "                " + functionNameProc + " " + functionName + ";"
    functionDefinitionList.append(queryFunctionString)

outputTemplate = """#pragma once
#include "glCompact/gl/Types.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/gl/FunctionsTypedef.hpp"
#include "glCompact/ToolsInternal.hpp"

namespace glCompact {
    namespace gl {
        class Functions {
            public:
                void init(void*(*getGlFunctionPointer)(const char* glFunctionName));

                //glNamePROC name;
                ///FUNCTION_DEFINITION_LIST
        };
    }
}
"""

output = outputTemplate.replace("                ///FUNCTION_DEFINITION_LIST", "\n".join(functionDefinitionList))
open(outputFile, 'w').write(output)
