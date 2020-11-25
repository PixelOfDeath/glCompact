CMAKE_MINIMUM_REQUIRED(VERSION 3.9)
PROJECT(glCompact CXX C)

#This only should be set when developing glCompact. It can break LTO when the parent project uses a higher C++ standard
SET(ENFORCINGCPLUSPLUS11 NO)

#Under linux, cmake leaves this empty by default. So we set it to release. This will not be set if e.g. a top level project already set it.
#Under Windows nmake this is always defaulted to debug, so we have to set it via parameter to release
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()

#LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)
#LIST(APPEND CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

#FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(glm CONFIG REQUIRED)
FIND_PACKAGE(PythonInterp REQUIRED)
IF(MSVC)
    FIND_PACKAGE(DbgHelp)
ENDIF()
FIND_PACKAGE(Doxygen)

#this enables the top level project to overwrite glCompact source files. E.g. include/glCompact/config.hpp
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/include")

#this includes the hpp/cpp files, generated via python scrips from gl.xml
INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/include")

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include")
INCLUDE_DIRECTORIES("${GLM_INCLUDE_DIRS}")
IF(MSVC AND DBGHELP_FOUND)
    INCLUDE_DIRECTORIES(${DBGHELP_INCLUDE_DIR})
ENDIF()

SET(CMAKE_RELEASE_POSTFIX "${POSTFIX}")
SET(CMAKE_DEBUG_POSTFIX   "${POSTFIX}_d")

FILE(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/glCompact/gl")
FILE(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src/glCompact/gl")

SET(GL_XLM "${PROJECT_SOURCE_DIR}/gl.xml")

ADD_CUSTOM_TARGET(Constants_hpp DEPENDS "${PROJECT_BINARY_DIR}/include/glCompact/gl/Constants.hpp")
ADD_CUSTOM_COMMAND(
    PRE_BUILD VERBATIM
    DEPENDS                        "${PROJECT_SOURCE_DIR}/include/glCompact/gl/Constants.hppToGenerate.pyt" "${GL_XLM}"
    COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/include/glCompact/gl/Constants.hppToGenerate.pyt" "${GL_XLM}" "${PROJECT_BINARY_DIR}/include/glCompact/gl/Constants.hpp"
    OUTPUT                                                                                                              "${PROJECT_BINARY_DIR}/include/glCompact/gl/Constants.hpp"
)

ADD_CUSTOM_TARGET(FunctionsTypedef_hpp DEPENDS "${PROJECT_BINARY_DIR}/include/glCompact/gl/FunctionsTypedef.hpp")
ADD_CUSTOM_COMMAND(
    PRE_BUILD VERBATIM
    DEPENDS                        "${PROJECT_SOURCE_DIR}/include/glCompact/gl/FunctionsTypedef.hppToGenerate.pyt" "${GL_XLM}"
    COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/include/glCompact/gl/FunctionsTypedef.hppToGenerate.pyt" "${GL_XLM}" "${PROJECT_BINARY_DIR}/include/glCompact/gl/FunctionsTypedef.hpp"
    OUTPUT                                                                                                                     "${PROJECT_BINARY_DIR}/include/glCompact/gl/FunctionsTypedef.hpp"
)

ADD_CUSTOM_TARGET(Functions_hpp DEPENDS "${PROJECT_BINARY_DIR}/include/glCompact/gl/Functions.hpp")
ADD_CUSTOM_COMMAND(
    PRE_BUILD VERBATIM
    DEPENDS                        "${PROJECT_SOURCE_DIR}/include/glCompact/gl/Functions.hppToGenerate.pyt" "${GL_XLM}"
    COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/include/glCompact/gl/Functions.hppToGenerate.pyt" "${GL_XLM}" "${PROJECT_BINARY_DIR}/include/glCompact/gl/Functions.hpp"
    OUTPUT                                                                                                              "${PROJECT_BINARY_DIR}/include/glCompact/gl/Functions.hpp"
)

ADD_CUSTOM_TARGET(Extensions_hpp DEPENDS "${PROJECT_BINARY_DIR}/include/glCompact/gl/Extensions.hpp")
ADD_CUSTOM_COMMAND(
    PRE_BUILD VERBATIM
    DEPENDS                        "${PROJECT_SOURCE_DIR}/include/glCompact/gl/Extensions.hppToGenerate.pyt" "${GL_XLM}"
    COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/include/glCompact/gl/Extensions.hppToGenerate.pyt" "${GL_XLM}" "${PROJECT_BINARY_DIR}/include/glCompact/gl/Extensions.hpp"
    OUTPUT                                                                                                               "${PROJECT_BINARY_DIR}/include/glCompact/gl/Extensions.hpp"
)

ADD_CUSTOM_COMMAND(
    PRE_LINK VERBATIM
    DEPENDS                        "${PROJECT_SOURCE_DIR}/src/glCompact/gl/Functions.cppToGenerate.pyt" "${GL_XLM}"
    COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/src/glCompact/gl/Functions.cppToGenerate.pyt" "${GL_XLM}" "${PROJECT_BINARY_DIR}/src/glCompact/gl/Functions.cpp"
    OUTPUT                                                                                                          "${PROJECT_BINARY_DIR}/src/glCompact/gl/Functions.cpp"
)

ADD_CUSTOM_COMMAND(
    PRE_LINK VERBATIM
    DEPENDS                        "${PROJECT_SOURCE_DIR}/src/glCompact/gl/Extensions.cppToGenerate.pyt" "${GL_XLM}"
    COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/src/glCompact/gl/Extensions.cppToGenerate.pyt" "${GL_XLM}" "${PROJECT_BINARY_DIR}/src/glCompact/gl/Extensions.cpp"
    OUTPUT                                                                                                           "${PROJECT_BINARY_DIR}/src/glCompact/gl/Extensions.cpp"
)

FILE(GLOB SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/src/glCompact/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/glCompact/gl/*.cpp"
)
SET(SOURCE_FILES
    "${SOURCE_FILES}"
    "${PROJECT_BINARY_DIR}/src/glCompact/gl/Extensions.cpp"
    "${PROJECT_BINARY_DIR}/src/glCompact/gl/Functions.cpp"
)
file(GLOB INCLUDE_FILES
    "${PROJECT_SOURCE_DIR}/include/glCompact/*.hpp"
    #"${PROJECT_SOURCE_DIR}/include/glCompact/gl/*.hpp"
)
ADD_LIBRARY(${PROJECT_NAME} STATIC ${SOURCE_FILES})
ADD_DEPENDENCIES(
    ${PROJECT_NAME}
    Constants_hpp
    FunctionsTypedef_hpp
    Functions_hpp
    Extensions_hpp
)

#We need at minimum C++11
#TARGET_COMPILE_FEATURES(${PROJECT_NAME} PUBLIC cxx_std_11)
#set_target_properties(${PROJECT_NAME} PROPERTIES
#    INTERPROCEDURAL_OPTIMIZATION ON
#)

#This only should be set when developing glCompact. It can break LTO when the parent project uses a higher C++ standard
#SET(ENFORCINGCPLUSPLUS11 NO)

#TODO: Is there a way to only set this as a minimum requiement?
#IF(ENFORCINGCPLUSPLUS11)
#    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
#        CXX_STANDARD 11
#        CXX_STANDARD_REQUIRED YES
#        CXX_EXTENSIONS NO
#    )
#ENDIF()

IF(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    SET(WE_ARE_THE_TOP_LEVEL_PROJECT TRUE)
ELSE()
    SET(WE_ARE_THE_TOP_LEVEL_PROJECT FALSE)
ENDIF()

IF(NOT WE_ARE_THE_TOP_LEVEL_PROJECT)
    #Set cmake variables for the parent project
    SET(GLCOMPACT_INCLUDE_DIRS "${GLM_INCLUDE_DIRS}" "${PROJECT_SOURCE_DIR}/include" PARENT_SCOPE)
    SET(GLCOMPACT_LIBRARY glCompact PARENT_SCOPE)
ENDIF()

#********** ********** DOXYGEN ********** **********
IF(DOXYGEN_FOUND AND WE_ARE_THE_TOP_LEVEL_PROJECT)
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY)
    ADD_CUSTOM_TARGET(
        doc
        ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
ENDIF()

IF(WIN32)
    SET(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}")
    INSTALL(
        TARGETS glCompact
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        RUNTIME DESTINATION "bin"
    )
    #Auto generated header files
    INSTALL(
        DIRECTORY ${PROJECT_BINARY_DIR}/include/
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
        DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp"
    )
    #Auto generated header files
    INSTALL(
        DIRECTORY ${PROJECT_BINARY_DIR}/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp"
    )
ELSE()
    MESSAGE(FATAL_ERROR "No cmake installer path for this platform!")
ENDIF()

IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET(BUILDBIT 64)
ELSE()
    SET(BUILDBIT 32)
ENDIF()

MESSAGE("-- glCompact (${BUILDBIT}bit) (${CMAKE_BUILD_TYPE}) (LTO=${LTOAvailable}) (Enforcing C++11=${ENFORCINGCPLUSPLUS11}) build will be INSTALLED in: ${CMAKE_INSTALL_PREFIX}")
IF(NOT DOXYGEN_FOUND)
    MESSAGE("-- Could not find Doxygen, install doxygen to be able to use build target doc to generate documentation!")
ENDIF()
