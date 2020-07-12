#include "glCompact/Tools_.hpp"

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

    [[noreturn]] void crash(std::string s){
        std::cout << s << std::endl;
        abort();
    }
}
