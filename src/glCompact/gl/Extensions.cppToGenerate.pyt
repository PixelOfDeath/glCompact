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
getExtensionList = []

for extension in extensionNameList:
    extensionName     = extension
    extensionNameLong = extensionName.ljust(extensionNameMaxLen)
    getExtensionString = "            " + extensionNameLong + ' = extensionSupported("' + extensionName + '");'
    getExtensionList.append(getExtensionString)

outputTemplate = """#include "glCompact/gl/Extensions.hpp"
#include "glCompact/ContextGroup.hpp"
#include <cstring>

namespace glCompact {
    namespace gl {
        void Extensions::init(
            const ContextGroup* contextGroup
        ) {
            //Needs at last 3.0! But unlike glGetString(GL_EXTENSIONS), this also works in core.
            int32_t extensionCount = 0;
            contextGroup->functions.glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
            auto extensionSupported = [&](const char* extensionName) -> bool {
                for (int32_t i = 0; i < extensionCount; ++i)
                    if (std::strcmp(extensionName, reinterpret_cast<const char*>(contextGroup->functions.glGetStringi(GL_EXTENSIONS, i))) == 0) return true;
                return false;
            };

            //name = extensionSupported(name);
            ///GET_EXTENSION_LIST
        }
    }
}
"""

output = outputTemplate.replace("            ///GET_EXTENSION_LIST", "\n".join(getExtensionList))
open(outputFile, 'w').write(output)
