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
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>

#include <sstream>
#include "tinyxml2.h"

using namespace std;

#define LOOPINT(v, m_m) for(int v = 0; v < int(m_m); v++)
#define LOOPI(m_m) LOOPINT(i,m_m)
#define LOOPJ(m_m) LOOPINT(j,m_m)

string fileToString(const string& fileName) {
    ifstream fileStream(fileName.c_str(), ios::in | ios::binary | ios::ate);
    if (!fileStream.is_open()) throw runtime_error("Can not open file \"" + fileName + "\" to read from it.");
    ifstream::pos_type fileSize = fileStream.tellg();
    //unsigned long long fileSize2 = fileSize;
    fileStream.seekg(0, ios::beg);
    vector<char> bytes(static_cast<size_t>(fileSize));
    fileStream.read(&bytes[0], static_cast<streamsize>(fileSize));
    string fileContend = string(&bytes[0], static_cast<size_t>(fileSize));
    return fileContend;
}

void stringToFile(string filename, string outputString) {
    cout << "  " << filename << endl;
    ofstream file;
    file.open(filename);
    file << outputString;
    file.close();
}

bool isWhitespace(const char c) {
    //tab is 9
    if (
           (c == ' ')
        || (c == char(9)) //TABULATOR
        //|| (c == '\n')
        //|| (c == '\r')
        ) return true;
    return false;
}

string trim(const string &s) {
    size_t length = s.length();
    size_t startPos = 0;
    while((startPos < length) && (isWhitespace(s[startPos]))) startPos++;
    size_t endPos = length;
    while((endPos > startPos + 1) && (isWhitespace(s[endPos - 1]))) endPos--;
    return s.substr(startPos, endPos - startPos);
}



struct functionParam {
    string type;
    string name;
    string typeAndName;
};
struct functionDef {
    string returnType;
    string name;
    //string glName;
    vector<functionParam> param;
};
vector<functionDef> functionList;

struct extension {
    string name;
    string coreInGlVersionString; //something like VERSION_GL_1_0 or empty
    bool isGl;
    bool isGlcore;
};
vector<extension> extensionList;

struct structEnum {
    string name;
    string type;
    string value;
};
vector<structEnum> enumList;

bool hasAttribute(tinyxml2::XMLElement *e, string name) {
    return e->QueryIntAttribute(name.c_str(), NULL) != tinyxml2::XML_NO_ATTRIBUTE;
}

string stringToUpper(string strToConvert) {
    std::transform(strToConvert.begin(), strToConvert.end(), strToConvert.begin(), ::toupper);
    return strToConvert;
}

string stringToLower(string strToConvert) {
    std::transform(strToConvert.begin(), strToConvert.end(), strToConvert.begin(), ::tolower);
    return strToConvert;
}

string toTypedefName(string input) {
    //return "PFN" + stringToUpper(input) + "PROC";
    return stringToUpper(input) + "PROC";
}

string cutOfGlMakeFirstLower(string input) {
    if (input.size() < 3) throw std::runtime_error("cutOfGlMakeFirstLower input must be at last 3 chars long");
    //if (input.size() < 3) return "NAMETHATISSMALLERTHEN3";
    return stringToLower(input.substr(2, 1)) + input.substr(3);
}

string xmlObjOnlyText(tinyxml2::XMLNode *n) {
    using namespace tinyxml2;
    string s;
    XMLNode *nc = n->FirstChild();
    while (nc)
    {
        XMLNode *subChild = nc->FirstChild();
        if (subChild)
            s += xmlObjOnlyText(nc);
        else
            s += string(nc->Value());
        nc = nc->NextSibling();
        if (nc) s += " ";
    }
    return s;
}

string removeSubString(string inputString, string removeString) {
    std::string::size_type i = inputString.find(removeString);
    if (i != std::string::npos) inputString.erase(i, removeString.length());
    return inputString;
}

string xmlObjExtractReturnTypeTextFromFunction(tinyxml2::XMLElement *e) {
    string functionDefinitionString = xmlObjOnlyText(e);
    string functionNameOnlyString   = xmlObjOnlyText(e->FirstChildElement("name"));
    return removeSubString(functionDefinitionString, functionNameOnlyString);
}

void getFunctionsFromXmlFile(string filename) {
    using namespace tinyxml2;
    XMLDocument doc;
    doc.LoadFile(filename.c_str());

    XMLElement* eCommand = doc.FirstChildElement("registry")->FirstChildElement("commands")->FirstChildElement("command");
    while (eCommand != 0)
    {
        functionDef f;
        /*XMLElement *eReturnType = eCommand->FirstChildElement("proto")->FirstChildElement("ptype");
        if (eReturnType && eReturnType->GetText())
            f.returnType = eReturnType->GetText();
        else if (eCommand->FirstChildElement("proto")->GetText())
            f.returnType = eCommand->FirstChildElement("proto")->GetText();
        else
            f.returnType = "void";*/

        f.returnType = xmlObjExtractReturnTypeTextFromFunction(eCommand->FirstChildElement("proto"));

        f.name   = eCommand->FirstChildElement("proto")->FirstChildElement("name")->GetText();
        //f.glName = eCommand->FirstChildElement("proto")->FirstChildElement("name")->GetText();
        //f.name   = cutOfGlMakeFirstLower(f.glName);

        XMLElement* p = eCommand->FirstChildElement("param");
        while (p != 0)
        {
            functionParam fp;
            XMLElement* ptype = p->FirstChildElement("ptype");
            fp.type = ptype ? fp.type = ptype->GetText() : p->GetText();
            fp.name = p->FirstChildElement("name")->GetText();
            fp.typeAndName = xmlObjOnlyText(p);
            f.param.push_back(fp);
            p = p->NextSiblingElement("param");
        }
        functionList.push_back(f);
        eCommand = eCommand->NextSiblingElement();
    }
}

void stringToListBySeperatorChar(const string &input, vector<string> &list, char seperator) {
    //vector<string> list;
    list.clear();
    int start = 0;
    int count = 0;
    LOOPI(input.size())
    {
        if (input[i] == seperator)
        {
            string ss = input.substr(start, count);
            list.push_back(ss);
            start += count + 1;
            count = 0;
        }
        else count++;
    }
    string ss = input.substr(start);
    list.push_back(ss);
}

void getExtensionsFromXmlFile(string filename) {
    tinyxml2::XMLDocument doc;
    doc.LoadFile(filename.c_str());

    tinyxml2::XMLElement* eExtension = doc.FirstChildElement("registry")->FirstChildElement("extensions")->FirstChildElement("extension");
    while (eExtension)
    {
        extension e;
        e.name = eExtension->Attribute("name"); //TODO: change to all upper case?

        e.isGl = false;
        e.isGlcore = false;

        if (hasAttribute(eExtension, "supported"))
        {
            string s = eExtension->Attribute("supported");
            vector<string> sList;
            stringToListBySeperatorChar(s, sList, '|');
            //cout << s << " -> ";
            LOOPI(sList.size())
            {
                if      (sList[i] == "gl"    ) e.isGl = true;
                else if (sList[i] == "glcore") e.isGlcore = true;
                else if (sList[i] == "gles1" ) {}
                else if (sList[i] == "gles2" ) {}
                else if (sList[i] == "glsc2" ) {}
                else if (sList[i] == "disabled" ) {} //only for GL_AMD_framebuffer_sample_positions, no idea what it is supposed to mean
                else cout << "error \"" << e.name << "\" \"supported\" has unknown substr (" << sList[i] << ")" << endl;
            }
        }
        else
        {
            cout << "error extension \"" << e.name << "\" has no \"supported\" string" << endl;
        }


        /*tinyxml2::XMLElement *eExtensionRequire = eExtension->FirstChildElement("require");
        if (eExtensionRequire != NULL)
        {
            tinyxml2::XMLElement *eCommand = eExtensionRequire->FirstChildElement("command");
            //while (eCommand != 0)
            //{
            //        e.functionNameList.push_back(eCommand->Attribute("name"));
            //        eCommand = eCommand->NextSiblingElement("command");
            //}
        }*/
        extensionList.push_back(e);
        eExtension = eExtension->NextSiblingElement();
    }

    //this looks in what core versions some of the extensions got integrated. So we later can set them to true by default depending on the supported GL version
    //Core extensions don't need to be listed in core mode. e.g. mesa drivers don't list them.
    //this is a hack because the information we need is only inside commants. There also is other text in it so we look for ARB_* HKR_* string parts.
    //This may find wrong relationship or wont find some at all... its a hack after all... TODO
    //note, also non core extensions (core = functions have same name as extension and core without any postfixes like myfuncARB) wont be listed here
    //for example ARB_texture_buffer_object_rgb32 is core, but ARB_texture_buffer_object is not!
    tinyxml2::XMLElement* eFeature = doc.FirstChildElement("registry")->FirstChildElement("feature");
    while (eFeature)
    {
        string currentGlVersionString = eFeature->Attribute("name"); //something like VERSION_GL_1_0
        //transform GL_VERSION_* to VERSION_GL_*
        if (currentGlVersionString.size() > 0)
            //currentGlVersionString = "VERSION_GL_" + currentGlVersionString.substr(11, string::npos);
            currentGlVersionString = "version.gl_" + currentGlVersionString.substr(11, string::npos);

        tinyxml2::XMLElement* eRequire = eFeature->FirstChildElement("require");
        while (eRequire) {
            if (!hasAttribute(eRequire, "profile") && hasAttribute(eRequire, "comment") /*&& string(eRequire->Attribute("comment")).substr(0, 6) == "Reuse "*/) {
                string pareseThis = string(eRequire->Attribute("comment"));
                size_t start = pareseThis.find("ARB_", 0);
                if (start == string::npos) start = pareseThis.find("KHR_", 0);
                size_t end = pareseThis.find(" ", start);
                string extensionName;
                if (end - start != 0) extensionName= "GL_" + pareseThis.substr(start, end - start);

                if (extensionName.size() == 0) {
                    cout << "  NOTE: comment found during search of extension-to-core-version-info that could not be parsed: \"" << pareseThis << "\"" << endl;
                } else {
                    LOOPI(extensionList.size()) if (extensionList[i].name == extensionName) {
                        extensionList[i].coreInGlVersionString = currentGlVersionString;
                        break;
                        if (extensionList.size() == unsigned(i)) cout << "  NOTE: Could not find extension \"" + extensionName + "\" to set core version \"" + currentGlVersionString + "\"" << endl;
                    }
                }
            }
            eRequire = eRequire->NextSiblingElement("require");
        }
        eFeature = eFeature->NextSiblingElement("feature");
    }
}


vector<string> featureInfoIfStringList;
struct featureInfoStruct {
    featureInfoStruct(string iType, string iTypeGetFunction, string iName, int ifStringIndex_)
        {type = iType;typeGetFunction = iTypeGetFunction;name = iName;ifStringIndex = ifStringIndex_;}
    string type;
    string typeGetFunction;
    string name;
    int ifStringIndex;
};
vector<featureInfoStruct> featureInfoList;

string firstDependancyString = "true";

void getFeatureInfoList(string filename) {
    string listText = fileToString(filename);
    stringstream ss(listText);
    string sLine = "";
    featureInfoIfStringList.push_back("true");
    int ifStringIndex = 0;
    while (getline(ss, sLine)) if (trim(sLine).size() != 0) {
        if (sLine[0] != ' ') {
            ifStringIndex++;
            featureInfoIfStringList.push_back(sLine);
        } else {
            sLine = trim(sLine);
            size_t loc = sLine.find(" ");
            string type = sLine.substr(0, loc);
            //TODO: maybe include all possible type functions here?
            string typeGetFunction;
            if (type == "GLuint64" || type == "GLint64")
                typeGetFunction = "glGetInteger64v";
            else
                typeGetFunction = "glGetIntegerv";
            string name = sLine.substr(loc + 1, string::npos);
            featureInfoList.push_back(featureInfoStruct(type, typeGetFunction, name, ifStringIndex));
        }
    }
}

vector<string> coreExtensionIfStringList;
struct coreExtensionStruct {
    coreExtensionStruct(string iName, int ifStringIndex_)
        {name = iName; ifStringIndex = ifStringIndex_;}
    string name;
    int ifStringIndex;
};
vector<coreExtensionStruct> coreExtensionInfoList;

void getCoreExtensionsList(string filename) {
    string listText = fileToString(filename);
    stringstream ss(listText);
    string sLine;
    coreExtensionIfStringList.push_back("true");
    int ifStringIndex = 0;
    while (getline(ss, sLine)) if (trim(sLine).size() != 0) {
        if (sLine[0] != ' ') {
            ifStringIndex++;
            coreExtensionIfStringList.push_back(sLine);
        } else {
            string name = trim(sLine);
            coreExtensionInfoList.push_back(coreExtensionStruct(name, ifStringIndex));
        }
    }
}

void getEnumsFromXmlFile(string filename) {
    using namespace tinyxml2;
    XMLDocument doc;
    doc.LoadFile(filename.c_str());

    XMLElement* eEnums = doc.FirstChildElement("registry")->FirstChildElement("enums");
    while (eEnums) {
        XMLElement* eEnum = eEnums->FirstChildElement("enum");
        while (eEnum) {
            if (hasAttribute(eEnum, "api") && (string(eEnum->Attribute("api")) != "gl")) {
                eEnum = eEnum->NextSiblingElement("enum");
                continue;
            }
            structEnum e;
            e.name  = eEnum->Attribute("name");
            if (hasAttribute(eEnum, "type")) {
                string s = eEnum->Attribute("type");
                if (s == "u") e.type = "GLuint";
                else if (s == "ull") e.type = "GLuint64";
                else e.type = "UNKNOWN_TYPE_FROM_XML_FILE(" + s + ")";
            }
            else {
                e.type = "GLint";
            }
            e.value = eEnum->Attribute("value");
            enumList.push_back(e);
            eEnum = eEnum->NextSiblingElement("enum");
        }
        eEnums = eEnums->NextSiblingElement("enums");
    }
}


string xmlObjAllToText(tinyxml2::XMLNode *n) {
    using namespace tinyxml2;
    /*string s;
    if (e->GetText()) s = e->GetText();
    XMLElement *ec = e->FirstChildElement();
    while(ec)
    {
        if (ec->GetText())
        {
        s += " ";
        s += ec->GetText();
        }
        ec = ec->NextSiblingElement();
    }
    return s;*/
    /*string s;
    XMLNode *nc = n->FirstChild();
    while (nc)
    {
        if (nc->ToText)
        {
            if (nc->ToText->getText())
        }
        nc = nc->NextSibling();
    }*/
    /*string s;
    XMLText *t = n->ToText();
    if (t)
    {
        XMLPrinter printer;
        t->Accept( &printer );
        s = string(printer.CStr());
    }*/

    /*string s;
    if (n->Value())
    {
        s = string(n->Value());
    }
    return s;*/
    string s;
    XMLNode *nc = n->FirstChild();
    while (nc)
    {
        s += "|" + string(nc->Value());
        XMLNode *subChild = nc->FirstChild();
        if (subChild) s += "(" + xmlObjAllToText(nc) + ")";
        nc = nc->NextSibling();
    }
    return s;

    /*string s;
    if (n->Value()) s = string(n->Value());
    return s;*/
}

class glVersion {
    public:
        glVersion(int major, int minor){this->major = major; this->minor = minor;};
        int major, minor;
};

vector<glVersion> versionList = {
    {1, 1},
    {1, 2},
    {1, 3},
    {1, 4},
    {1, 5},

    {2, 0},
    {2, 1},

    {3, 0},
    {3, 1},
    {3, 2},
    {3, 3},

    {4, 0},
    {4, 1},
    {4, 2},
    {4, 3},
    {4, 4},
    {4, 5}
};

void replaceStringKeepSpacing(string &target, string at, string insertThis) {
    size_t loc = target.find(at);
    if (loc == string::npos) throw std::runtime_error("Cannot find string in string");
    size_t locLineBegin = target.rfind("\n", loc);
    if (locLineBegin == string::npos)
        locLineBegin = 0;
    else
        locLineBegin++;
    size_t spaceCount = loc - locLineBegin;
    target.erase(locLineBegin, spaceCount + at.size() + 1);

    string insertThisWithSpacing;
    std::stringstream readStream(insertThis);
    string stringLine;
    while(std::getline(readStream, stringLine)) {
        LOOPI(spaceCount) insertThisWithSpacing += ' ';
        insertThisWithSpacing += stringLine + '\n';
    }
    target.insert(locLineBegin, insertThisWithSpacing);
}

string addMinPadding(string s, size_t minPadding) {
    if (minPadding <= s.size()) return s;
    return s.append(minPadding - s.length(), ' ');
}

string makeGlConstandList() {
    string s;
    size_t maxSizeType  = 0;
    size_t maxSizeName  = 0;
    for (auto e : enumList) {
        maxSizeType  = max<size_t>(maxSizeType, e.type.size());
        maxSizeName  = max<size_t>(maxSizeName, e.name.size());
    }
    for (auto e : enumList) {
        string type  = addMinPadding(e.type, maxSizeType);
        string name  = addMinPadding(e.name, maxSizeName);
        string value = e.value;
        s += "constexpr " + type + " " + name + " = " + value + ";" + '\n';
    }
    return s;
}

string makeGlFunctionTypedefList() {
    string s;
    size_t maxSizeReturnType = 0;
    size_t maxSizeName       = 0;
    for (auto e : functionList) {
        maxSizeReturnType  = max<size_t>(maxSizeReturnType, e.returnType.size());
        maxSizeName        = max<size_t>(maxSizeName      , toTypedefName(e.name).size());
    }
    for (auto e : functionList) {
        string returnType  = addMinPadding(e.returnType, maxSizeReturnType);
        string typedefName = addMinPadding(toTypedefName(e.name), maxSizeName);
        s += "EXTERNC typedef " + returnType + " (STDCALL *" + typedefName + ")(";
        LOOPJ(e.param.size()) {
            if (j>0) s += ", ";
            s += e.param[j].typeAndName;
        }
        s += ");\n";
    }
    return s;
}

string makeGlExtensionList() {
    string s;
    size_t maxSizeName = 0;
    for (auto e : extensionList) {
        maxSizeName = max<size_t>(maxSizeName, e.name.size());
    }
    for (auto e : extensionList) {
        string name = addMinPadding(e.name, maxSizeName);
        s += "bool " + name + " = false;\n";
    }
    /*LOOPI(extensionList.size())
        s += "bool " + extensionList[i].name + " = false;\n";*/
    return s;
}

string makeGlValueDefinitionList() {
    string s;
    size_t maxSizeType = 0;
    size_t maxSizeName = 0;
    for (auto e : featureInfoList) {
        maxSizeType = max<size_t>(maxSizeType, e.type.size());
        maxSizeName = max<size_t>(maxSizeName, e.name.size());
    }
    for (auto e : featureInfoList) {
        string type = addMinPadding(e.type, maxSizeType);
        string name = addMinPadding(e.name, maxSizeName);
        s += type + " GL_" + name + " = 0;\n";
    }
    /*LOOPI(featureInfoList.size())
        s += featureInfoList[i].type + " GL_" + featureInfoList[i].name + " = 0;\n";*/
    return s;
}

string makeGlFunctionDefinitionList() {
    string s;
    size_t maxSizeTypedefName = 0;
    for (auto e : functionList) {
        maxSizeTypedefName = max<size_t>(maxSizeTypedefName, toTypedefName(e.name).size());
    }
    for (auto e : functionList) {
        string typedefName = addMinPadding(toTypedefName(e.name), maxSizeTypedefName);
        //string name        = e.name;
        s += typedefName + " " + e.name + " = reinterpret_cast<" + toTypedefName(e.name) + ">(0);\n";
    }
    /*LOOPI(functionList.size())
        s += "GL_CONTEXT_INTERFACE_FUNCTION_SETTING_STATIC " + toTypedefName(functionList[i].name) + " " + functionList[i].name + ";\n";*/
    return s;
}

string makeGlFunctionDefinitionStatic() {
    string s;
    size_t maxSizeTypedefName = 0;
    size_t maxSizeName        = 0;
    for (auto e : functionList) {
        maxSizeTypedefName = max<size_t>(maxSizeTypedefName, toTypedefName(e.name).size());
        maxSizeName        = max<size_t>(maxSizeName, e.name.size());
    }
    for (auto e : functionList) {
        string typedefName = addMinPadding(toTypedefName(e.name), maxSizeTypedefName);
        string name        = addMinPadding(e.name, maxSizeName);
        s += typedefName + " glContext::" + name + " = reinterpret_cast<" + toTypedefName(e.name) + ">(0);\n";
    }
    //LOOPI(functionList.size())
    //    s += toTypedefName(functionList[i].name) + " glContext::" + functionList[i].name + " = reinterpret_cast<" + toTypedefName(functionList[i].name) + ">(0);\n";
    return s;
}

string makeGlGetFunctionPointerList() {
    string s;
    size_t maxSizeTypedefName = 0;
    size_t maxSizeName        = 0;
    for (auto e : functionList) {
        maxSizeTypedefName = max<size_t>(maxSizeTypedefName, toTypedefName(e.name).size());
        maxSizeName        = max<size_t>(maxSizeName, e.name.size());
    }
    for (auto e : functionList) {
        string typedefName = addMinPadding(toTypedefName(e.name), maxSizeTypedefName);
        string name        = addMinPadding(e.name, maxSizeName);
        s += name + " = reinterpret_cast<" + typedefName + ">(getGlFunctionPointer(\"" + e.name + "\"));\n";
    }
    //LOOPI(functionList.size())
    //    s += functionList[i].name + " = reinterpret_cast<" + toTypedefName(functionList[i].name) + ">(getProcAddressProc(\"" + functionList[i].name + "\"));\n";
    return s;
}

string makeGlGetExtensionList() {
    string s;
    size_t maxSizeName = 0;
    for (auto e : extensionList) {
        maxSizeName = max<size_t>(maxSizeName, e.name.size());
    }
    for (auto e : extensionList) {
        string name = addMinPadding(e.name, maxSizeName);
        //s += "extension_." + name + " = (SDL_TRUE == SDL_GL_ExtensionSupported(\"" + e.name + "\"));\n";
        s += name + " = extensionSupported(\"" + e.name + "\");\n";

    }
    //LOOPI(extensionList.size())
    //    s += "extension_." + extensionList[i].name + " = (SDL_TRUE == SDL_GL_ExtensionSupported(\"" + extensionList[i].name + "\"));\n";
    return s;
}

string makeGlGetExtensionsInGlVersion() {
    string s;
    int ifStringIndex = -1;
    LOOPI(coreExtensionInfoList.size()) {
        if (coreExtensionInfoList[i].ifStringIndex != ifStringIndex) {
            ifStringIndex = coreExtensionInfoList[i].ifStringIndex;
            s += "if (" + coreExtensionIfStringList[ifStringIndex] + ") {\n";
        }
        s += "    extension_." + coreExtensionInfoList[i].name + " = true;\n";
        if ((unsigned(i) == coreExtensionInfoList.size() - 1) || coreExtensionInfoList[i].ifStringIndex != coreExtensionInfoList[i + 1].ifStringIndex)
            s += "}\n";
    }
    return s;
}

string makeGlGetValueList() {
    string s;
    int ifStringIndex = -1;
    LOOPI(featureInfoList.size()) {
        if (featureInfoList[i].ifStringIndex != ifStringIndex) {
            ifStringIndex = featureInfoList[i].ifStringIndex;
            s += "if (" + featureInfoIfStringList[ifStringIndex] + ") {\n";
        }
        s += "   " + featureInfoList[i].typeGetFunction + "(GL_" + featureInfoList[i].name + ", &value_.GL_" + featureInfoList[i].name + ");\n";

        if ((unsigned(i) == featureInfoList.size() - 1) || featureInfoList[i].ifStringIndex != featureInfoList[i + 1].ifStringIndex)
            s += "}\n";
    }
    return s;
}

string makeGlSetFunctionsPointer0() {
    string s;
    LOOPI(functionList.size())
        s += functionList[i].name + " = reinterpret_cast<" + toTypedefName(functionList[i].name) + ">(0);\n";
    return s;
}

string makeGlSetConstandsExtension0() {
    string s;
    LOOPI(extensionList.size())
        s += "extension_." + extensionList[i].name + " = false;\n";
    return s;
}

string makeGlSetConstandsValue0() {
    string s;
    LOOPI(featureInfoList.size())
        s += "value_.GL_" + featureInfoList[i].name + " = 0;\n";
    return s;
}

int main(int argc, char *argv[]) {
    cout << "gl file generator for glCompact library" << endl;
    if (argc != 2) {
        //cout << " Usage <resouces directory> <hpp output directory> <cpp output directory>" << endl;
        cout << " Usage <base directory>" << endl;
        return -1;
    }

    const string baseDir          = string(argv[1]) + "/";
    //const string resoucesDir  = string(argv[1]) + "/";
    //const string outputDirHpp = string(argv[2]) + "/";
    //const string outputDirCpp = string(argv[3]) + "/";

    const string xmlDir           = baseDir + "glFileGeneratorRescources/xml/";
    const string listDir          = baseDir + "glFileGeneratorRescources/";
    const string toGenerateDirHpp = baseDir + "include/glCompact/gl/";
    const string toGenerateDirCpp = baseDir + "src/glCompact/gl/";

    const string outputDirHpp     = "include/glCompact/gl/";
    const string outputDirCpp     = "src/glCompact/gl/";

    cout << " parsing \"" + xmlDir + "gl.xml\" ..." << endl;
    string xmlFile = xmlDir + "gl.xml";
    getEnumsFromXmlFile     (xmlFile.c_str());
    getFunctionsFromXmlFile (xmlFile.c_str());
    getExtensionsFromXmlFile(xmlFile.c_str());
    getFeatureInfoList   (listDir + "featureInfoList.txt");
    getCoreExtensionsList(listDir + "coreExtensionList.txt");

    cout << " generating files..." << endl;

  //Constant.hpp
    string constantString = fileToString(toGenerateDirHpp + "Constants.hppToGenerate");
    replaceStringKeepSpacing(constantString, "///CONSTAND_LIST", makeGlConstandList());
    stringToFile(outputDirHpp + "Constants.hpp", constantString);
  //FunctionTypedef.hpp
    string FunctionTypedefHppString = fileToString(toGenerateDirHpp + "FunctionsTypedef.hppToGenerate");
    replaceStringKeepSpacing(FunctionTypedefHppString, "///FUNCTION_TYPEDEF_LIST",    makeGlFunctionTypedefList());
    stringToFile(outputDirHpp + "FunctionsTypedef.hpp", FunctionTypedefHppString);

  //Value.hpp
    //string ValueHppString = fileToString(toGenerateDirHpp + "Values.hppToGenerate");
    //replaceStringKeepSpacing(ValueHppString, "///VALUES_DEFINITION_LIST", makeGlValueDefinitionList());
    //stringToFile(outputDirHpp + "Values.hpp", ValueHppString);

  //Function.hpp
    string FunctionHppString = fileToString(toGenerateDirHpp + "Functions.hppToGenerate");
    replaceStringKeepSpacing(FunctionHppString, "///FUNCTION_DEFINITION_LIST", makeGlFunctionDefinitionList());
    stringToFile(outputDirHpp + "Functions.hpp", FunctionHppString);
  //Extension.hpp
    string ExtensionHppString = fileToString(toGenerateDirHpp + "Extensions.hppToGenerate");
    replaceStringKeepSpacing(ExtensionHppString, "///EXTENSION_LIST", makeGlExtensionList());
    stringToFile(outputDirHpp + "Extensions.hpp", ExtensionHppString);

  //Function.cpp
    string FunctionCppString = fileToString(toGenerateDirCpp + "Functions.cppToGenerate");
    replaceStringKeepSpacing(FunctionCppString, "///GET_FUNCTION_POINTER_LIST", makeGlGetFunctionPointerList());
    stringToFile(outputDirCpp + "Functions.cpp", FunctionCppString);
  //Extension.cpp
    string ExtensionCppString = fileToString(toGenerateDirCpp + "Extensions.cppToGenerate");
    replaceStringKeepSpacing(ExtensionCppString, "///GET_EXTENSION_LIST", makeGlGetExtensionList());
    stringToFile(outputDirCpp + "Extensions.cpp", ExtensionCppString);

    return 0;
}
