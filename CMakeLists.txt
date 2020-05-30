cmake_minimum_required(VERSION 3.9)
PROJECT(glCompact CXX C)

#Under linux, cmake leaves this empty by default. So we set it to release. This will not be set if e.g. a top level project already set it.
#Under Windows nmake this is always defaulted to debug, so we have to set it via parameter to release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

#LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)
LIST(APPEND CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

#FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(glm REQUIRED)
FIND_PACKAGE(PythonInterp REQUIRED)
if (MSVC)
    FIND_PACKAGE(DbgHelp)
endif()

#this enabled the top project to overwrite glCompact source files. E.g. include/glCompact/config.hpp
include_directories("${CMAKE_SOURCE_DIR}/include")

#this includes the hpp/cpp files, generated via python scrips from gl.xml
include_directories("${PROJECT_BINARY_DIR}/include")

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${GLM_INCLUDE_DIRS}")
if (MSVC AND DBGHELP_FOUND)
    INCLUDE_DIRECTORIES(${DBGHELP_INCLUDE_DIR})
endif()

SET(CMAKE_RELEASE_POSTFIX "${POSTFIX}")
SET(CMAKE_DEBUG_POSTFIX   "${POSTFIX}_d")

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/glCompact/gl")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src/glCompact/gl")

SET(GL_XLM "${PROJECT_SOURCE_DIR}/gl.xml")

ADD_CUSTOM_TARGET(Constants_hpp DEPENDS "include/glCompact/gl/Constants.hpp")
ADD_CUSTOM_COMMAND(
    PRE_BUILD
    DEPENDS
        ${GL_XLM}
        "${PROJECT_SOURCE_DIR}/include/glCompact/gl/Constants.hppToGenerate.pyt"
    COMMAND
        "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/include/glCompact/gl/Constants.hppToGenerate.pyt" "${GL_XLM}" "${PROJECT_BINARY_DIR}/include/glCompact/gl/Constants.hpp"
    OUTPUT
        "include/glCompact/gl/Constants.hpp"
)

ADD_CUSTOM_TARGET(FunctionsTypedef_hpp DEPENDS "include/glCompact/gl/FunctionsTypedef.hpp")
ADD_CUSTOM_COMMAND(
    PRE_BUILD
    DEPENDS
        "${GL_XLM}"
        "${PROJECT_SOURCE_DIR}/include/glCompact/gl/FunctionsTypedef.hppToGenerate.pyt"
    COMMAND
        "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/include/glCompact/gl/FunctionsTypedef.hppToGenerate.pyt" "${GL_XLM}" "${PROJECT_BINARY_DIR}/include/glCompact/gl/FunctionsTypedef.hpp"
    OUTPUT
        "include/glCompact/gl/FunctionsTypedef.hpp"
)

ADD_CUSTOM_TARGET(Functions_hpp DEPENDS "include/glCompact/gl/Functions.hpp")
ADD_CUSTOM_COMMAND(
    PRE_BUILD
    DEPENDS
        "${GL_XLM}"
        "${PROJECT_SOURCE_DIR}/include/glCompact/gl/Functions.hppToGenerate.pyt"
    COMMAND
        "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/include/glCompact/gl/Functions.hppToGenerate.pyt" "${GL_XLM}" "${PROJECT_BINARY_DIR}/include/glCompact/gl/Functions.hpp"
    OUTPUT
        "include/glCompact/gl/Functions.hpp"
)

ADD_CUSTOM_TARGET(Extensions_hpp DEPENDS "include/glCompact/gl/Extensions.hpp")
ADD_CUSTOM_COMMAND(
    PRE_BUILD
    DEPENDS
        "${GL_XLM}"
        "${PROJECT_SOURCE_DIR}/include/glCompact/gl/Extensions.hppToGenerate.pyt"
    COMMAND
        "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/include/glCompact/gl/Extensions.hppToGenerate.pyt" "${GL_XLM}" "${PROJECT_BINARY_DIR}/include/glCompact/gl/Extensions.hpp"
    OUTPUT
        "include/glCompact/gl/Extensions.hpp"
)

ADD_CUSTOM_COMMAND(
    PRE_LINK
    DEPENDS
        "${GL_XLM}"
        "${PROJECT_SOURCE_DIR}/src/glCompact/gl/Functions.cppToGenerate.pyt"
    COMMAND
        "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/src/glCompact/gl/Functions.cppToGenerate.pyt" "${GL_XLM}" "${PROJECT_BINARY_DIR}/src/glCompact/gl/Functions.cpp"
    OUTPUT
        "src/glCompact/gl/Functions.cpp"
)

ADD_CUSTOM_COMMAND(
    PRE_LINK
    DEPENDS
        "${GL_XLM}"
        "${PROJECT_SOURCE_DIR}/src/glCompact/gl/Extensions.cppToGenerate.pyt"
    COMMAND
        "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/src/glCompact/gl/Extensions.cppToGenerate.pyt" "${GL_XLM}" "${PROJECT_BINARY_DIR}/src/glCompact/gl/Extensions.cpp"
    OUTPUT
        "src/glCompact/gl/Extensions.cpp"
)

file(GLOB SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/src/glCompact/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/glCompact/gl/*.cpp"
)
SET(SOURCE_FILES
    ${SOURCE_FILES}
    "${PROJECT_BINARY_DIR}/src/glCompact/gl/Extensions.cpp"
    "${PROJECT_BINARY_DIR}/src/glCompact/gl/Functions.cpp"
)
file(GLOB INCLUDE_FILES
    "${PROJECT_SOURCE_DIR}/include/glCompact/*.hpp"
    #"${PROJECT_SOURCE_DIR}/include/glCompact/gl/*.hpp"
)
ADD_LIBRARY(${PROJECT_NAME} STATIC ${SOURCE_FILES} ${INCLUDE_FILES})
ADD_DEPENDENCIES(
    ${PROJECT_NAME}
    Constants_hpp
    FunctionsTypedef_hpp
    Functions_hpp
    Extensions_hpp
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    #We are the top-level project
    message(STATUS "glCompact is the top level project - enforcing C++11 standard")
    SET(ENFORCINGCPLUSPLUS11 YES)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )
else()
    #called from other project with add_subdirectory()
    message(STATUS "glCompact is NOT the top level project - NOT enforcing C++11 standard to prevent LTO breakage")
    SET(ENFORCINGCPLUSPLUS11 NO)
endif()

if(NOT ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug"))
    include(CheckIPOSupported)
    check_ipo_supported(RESULT LTOAvailable OUTPUT LtoError)
    if(LTOAvailable)
        message(STATUS "IPO / LTO enabled")
        set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "IPO / LTO not supported by this build chain: <${LtoError}>")
    endif()
else()
    message(STATUS "IPO / LTO disabled in debug build")
    SET(LTOAvailable NO)
endif()

message("-- Compiler is ${CMAKE_CXX_COMPILER_ID}")
IF("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    IF("${CMAKE_BUILD_TYPE}" MATCHES "Release")
        target_compile_options(glCompact PUBLIC "-O2")
        target_compile_options(glCompact PUBLIC "-Wall")

        #only relevant for shared library:
        #target_compile_options(glCompact PUBLIC "-ftls-model=local-exec")     #can not link with this. Probably have to cleanly mark functions as shared and non-shared first. So there is no direct access to the TLS? Linker error is kind of nonsense.
        #target_compile_options(glCompact PUBLIC "-ftls-model=initial-exec")   #can't be loaded via dlopen/dlsym? Could be an issue for application that already load a lot of libs with static TLS?! (is this a slot or a memory limitation?)
        #target_compile_options(glCompact PUBLIC "-ftls-model=local-dynamic")
        #target_compile_options(glCompact PUBLIC "-ftls-model=global-dynamic") #default for shared lib, garanties that library can be dynamicaly loaded. But accessing TLS costs more!

        #costly performane optimisation
        #target_compile_options(glCompact PUBLIC "-flto")                #Slightly faster, but way slower to link! Probably do not need this anymore when going only static linked for the C++ part.
        #target_compile_options(glCompact PUBLIC "-fomit-frame-pointer") #If no stack frame is needed, frees a register in function calls. But can make code way harder to debug!
        #target_compile_options(glCompact PUBLIC "-fwhole-program")      #NOT usable for lib, will remove to much stuff!
    ELSE()
        target_compile_options(glCompact PUBLIC "-Wall")
        target_compile_options(glCompact PUBLIC "-g3")
    ENDIF()
ELSEIF("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    #We can compile for windows XP as target, but debug executable does NOT work there!
    #SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE,5.01")
    #SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS,5.01")

    #target_compile_options(glCompact PUBLIC "$<$<STREQUAL:$<CONFIGURATION>,Release>:/GL>")
    #target_compile_options(glCompact PUBLIC "/MT$<$<STREQUAL:$<CONFIGURATION>,Debug>:d>")

    IF("${CMAKE_BUILD_TYPE}" MATCHES "Release")
        #target_compile_options(glCompact PUBLIC "/Od")
        #target_compile_options(glCompact PUBLIC "/MD") #this is CMake default
        #target_compile_options(glCompact PUBLIC "/GL>") #Whole Program Optimization; increases link time by a lot! But only matters for static linked libraries
    ELSE()
        #target_compile_options(glCompact PUBLIC "/MDd>") #this is CMake default
    ENDIF()
#ELSEIF("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
#ELSEIF("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
ELSE()
    message(FATAL_ERROR "Compiler is not supported!")
ENDIF()

#********** ********** DOXYGEN ********** **********
FIND_PACKAGE(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(
        doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif()

IF(WIN32)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}")
    INSTALL(
        TARGETS glCompact
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        RUNTIME DESTINATION "bin"
    )
    #Auto generated header files
    INSTALL(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/
        DESTINATION generatedInclude
        FILES_MATCHING PATTERN "*.hpp"
    )
ELSEIF(UNIX)
    INSTALL(
        TARGETS glCompact
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        RUNTIME DESTINATION "bin"
    )
    INSTALL(
        DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp"
    )
    #Auto generated header files
    INSTALL(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp"
    )
ELSE()
    MESSAGE(FATAL_ERROR "No cmake installer path for this platform!")
ENDIF()

IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(BUILDBIT 64)
ELSE()
    set(BUILDBIT 32)
ENDIF()

MESSAGE("--")
MESSAGE("-- glCompact (${BUILDBIT}bit) (${CMAKE_BUILD_TYPE}) (LTO=${LTOAvailable}) (Enforcing C++11=${ENFORCINGCPLUSPLUS11}) build will be INSTALLED in: ${CMAKE_INSTALL_PREFIX}")
if(NOT DOXYGEN_FOUND)
message("-- Could not find Doxygen, install doxygen to be able to use build target doc to generate documentation!")
endif()
MESSAGE("--")
