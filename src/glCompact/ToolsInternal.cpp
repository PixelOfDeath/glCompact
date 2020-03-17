/*
    glCompact
    Copyright (C) 2019-2020 Frederik Uje vom Hofe

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "glCompact/ToolsInternal.hpp"

#include <vector>
#include <fstream>
#include <iostream>

//#include <vector>
//#include <locale>

//#include <unordered_set>
//#include <set>
//#include <stdexcept>

using namespace std;

namespace glCompact {
    std::string fileToString(
        const std::string& fileName
    ) {
        std::ifstream fileStream(fileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
        if (!fileStream.is_open()) throw std::runtime_error("Can not open file \"" + fileName + "\" to read from it.");
        std::ifstream::pos_type fileSize = fileStream.tellg();
        //unsigned long long fileSize2 = fileSize;
        fileStream.seekg(0, std::ios::beg);
        std::vector<char> bytes(static_cast<std::size_t>(fileSize));
        fileStream.read(&bytes[0], static_cast<std::streamsize>(fileSize));
        std::string fileContend = std::string(&bytes[0], static_cast<std::size_t>(fileSize));
        return fileContend;
    }

    void crash(std::string s){
        std::cout << s << std::endl;
        abort();
    }
}
