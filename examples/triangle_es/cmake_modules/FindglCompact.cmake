# Locate glCompact library
# This module defines
# GLCOMPACT_FOUND, if false, do not try to link to GLCOMPACT
# GLCOMPACT_LIBRARY, the name of the librarys to link against
# GLCOMPACT_INCLUDE_DIR, where to find GLCOMPACT headers
#
# Because glCompact depends on OpenGL, OPENGL_gl_LIBRARY will be included automatically
# Static lib:
# IMPORTANT: GLCOMPACT_LIBRARY HAS TO BE IN TARGET_LINK_LIBRARIES BEFORE ANY OPENGL LIB!
# OR THERE WILL BE LINK ERRORS: e.g. undefined reference to `glXGetProcAddress'

#Find_Package(OpenGL REQUIRED)
#Find_Package(SDL2 REQUIRED)
Find_Package(GLM REQUIRED)
if (MSVC)
    Find_Package(DbgHelp)
endif()

SET( GLCOMPACTDIR $ENV{GLCOMPACTDIR} )
IF(WIN32 AND NOT(CYGWIN))
    # Convert backslashes to slashes
    STRING(REGEX REPLACE "\\\\" "/" GLCOMPACTDIR "${GLCOMPACTDIR}")
ENDIF(WIN32 AND NOT(CYGWIN))

FIND_PATH(GLCOMPACT_INCLUDE_DIR glCompact/glCompact.hpp
    PATHS
        /usr/local/include/glCompact
        ${GLCOMPACTDIR}/include
    #PATH_SUFFIXES
    #   glCompact
)

#IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
#    SET(LIBNAMESUFFIX "64")
#ELSE()
#    SET(LIBNAMESUFFIX "32")
#ENDIF()
#SET(LIBNAME "glCompact${LIBNAMESUFFIX}")
SET(LIBNAME "glCompact")

FIND_LIBRARY(
    GLCOMPACT_LIBRARY
    NAMES
        ${LIBNAME}
        #${LIBNAME}.so
        ${LIBNAME}.a
        ${LIBNAME}.lib
    PATHS
        /usr/local/lib/
        ${GLCOMPACTDIR}/lib/
    PATH_SUFFIXES
        lib64 lib32 lib
)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    glCompact
    REQUIRED_VARS
        GLCOMPACT_LIBRARY
        GLCOMPACT_INCLUDE_DIR
    #VERSION_VAR
    #    glCompact_VERSION
    #FAIL_MESSAGE
    #    DEFAULT_MSG
) 

#if (MSVC AND DBGHELP_FOUND)
#    SET(GLCOMPACT_LIBRARY
#        ${GLCOMPACT_LIBRARY}
#        ${DBGHELP_LIBRARY}
#    )
#endif()

SET(GLCOMPACT_INCLUDE_DIR
    ${GLCOMPACT_INCLUDE_DIR}
    #${SDL2_INCLUDE_DIR}
    ${GLM_INCLUDE_DIRS}
)
#if (MSVC AND DBGHELP_FOUND)
#    SET(GLCOMPACT_INCLUDE_DIR
#        ${GLCOMPACT_INCLUDE_DIR}
#        ${DBGHELP_INCLUDE_DIR}
#    )
#endif()
