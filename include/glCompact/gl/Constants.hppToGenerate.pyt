#!/usr/bin/python

import sys
import xml.etree.ElementTree as ET

if len(sys.argv) < 3:
    print("need parameter 'xml source' and 'output file'")
    exit()

xmlFile    = sys.argv[1]
outputFile = sys.argv[2]

root = ET.parse(xmlFile).getroot()

enumTypeList  = []
enumNameList  = []
enumValueList = []

for enums in root.findall("enums"):
    for enum in enums.findall("enum"):
        api = enum.get("api")
        if ((api != None) and (api != "gl")):
            continue

        typeName = None
        typeAttr = enum.get("type")
        if (typeAttr != None):
            if (typeAttr == "u"):
                typeName = "GLuint"
            elif (typeAttr == "ull"):
                typeName = "GLuint64"
            else:
                raise ValueError("Unexpected type from xml file: " + typeAttr)
        else:
            typeName = "GLint"

        enumTypeList.append(typeName)
        enumNameList.append(enum.get("name"))
        enumValueList.append(enum.get("value"))

enumTypeMaxLen  = max(len(x) for x in enumTypeList)
enumNameMaxLen  = max(len(x) for x in enumNameList)

enumList = []
i = 0
while i < len(enumTypeList):
    enumType  = enumTypeList [i].ljust(enumTypeMaxLen)
    enumName  = enumNameList [i].ljust(enumNameMaxLen)
    enumValue = enumValueList[i]
    enumList.append("        constexpr " + enumType + " " + enumName + " = " + enumValue + ";")
    i += 1

outputTemplate = """#pragma once
#include "glCompact/gl/Types.hpp"

namespace glCompact {
    namespace gl {
        //constexpr TYPE NAME = VALUE;
        ///CONSTAND_LIST
    }
}
"""

output = outputTemplate.replace("        ///CONSTAND_LIST", "\n".join(enumList))
open(outputFile, 'w').write(output)
