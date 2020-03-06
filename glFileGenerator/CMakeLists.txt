cmake_minimum_required(VERSION 3.7)
Project(glFileGenerator C CXX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(SOURCE_FILES
    src/glFileGenerator.cpp
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake_modules)
Find_Package(tinyxml2 REQUIRED)

INCLUDE_DIRECTORIES(
    ${TINYXML2_INCLUDE_DIR}
)

#SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_BUILD_TYPE Release)

add_executable(${PROJECT_NAME}
    #WIN32 # Disable the DOS prompt in Windows
    #MACOSX_BUNDLE
    ${SOURCE_FILES}
)

message("-- Compiler is ${CMAKE_CXX_COMPILER_ID}")
if(${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
    set(COMPILER_MSVC 1)
#elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
#    set(COMPILER_CLANG 1)
#elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
#    set(COMPILER_INTEL 1)
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    set(COMPILER_GCC 1)
else()
    message("-- Compiler is not supported!")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

IF(COMPILER_GCC)
    target_compile_options(${PROJECT_NAME} PUBLIC "-Wall")
ELSE()
    #/GT = Support Fiber-Safe Thread-Local Storage (Optimisation)
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<STREQUAL:$<CONFIGURATION>,Release>:/GL>")
ENDIF()

target_link_libraries(${PROJECT_NAME}
    ${TINYXML2_LIBRARY}
)
