#!/usr/bin/python

import sys
import xml.etree.ElementTree as ET

if len(sys.argv) < 3:
    print("need parameter 'xml source' and 'output file'")
    exit()

xmlFile    = sys.argv[1]
outputFile = sys.argv[2]

root = ET.parse(xmlFile).getroot()

extensionNameList = []
for extension in root.find("extensions").findall("extension"):
    extensionName = extension.get("name")
    extensionNameList.append(extensionName);

extensionNameMaxLen = max(len(x) for x in extensionNameList)
extensionList = []
for extensionName in extensionNameList:
    extensionList.append("                    bool " + extensionName + ";")

outputTemplate = """#pragma once
#include "glCompact/gl/Types.hpp"

namespace glCompact {
    class ContextGroup_;
    namespace gl {
            class Extensions {
                public:
                    void init(const ContextGroup_* contextGroup_);

                    //bool name;
                    ///EXTENSION_LIST
            };
    }
}
"""

output = outputTemplate.replace("                    ///EXTENSION_LIST", "\n".join(extensionList))
open(outputFile, 'w').write(output)
