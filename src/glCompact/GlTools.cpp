#include "glCompact/GlTools.hpp"
#include "glCompact/gl/Constants.hpp"

#include "glCompact/ToolsInternal.hpp"

#include <vector>
#include <locale>


//#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#include <unordered_set>
#include <set>
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    glm::mat4 getOrthoInsideSquare(
        int   resX,
        int   resY,
        float zNear,
        float zFar
    ) {
        assert(resX > 0 && resY > 0);
        assert(zNear > 0.0f);
        assert(zFar > zNear);

        float x = static_cast<float>(resX);
        float y = static_cast<float>(resY);

        if (x > y) {
            float sideTruncated = y / x;
            return glm::ortho(-1.0f,          1.0f,          -sideTruncated, sideTruncated, zNear, zFar);
        } else if (x < y) {
            float sideTruncated = x / y;
            return glm::ortho(-sideTruncated, sideTruncated, -1.0f,          1.0f,          zNear, zFar);
        } else {
            return glm::ortho(-1.0f,          1.0f,          -1.0f,          1.0f,          zNear, zFar);
        }
    }

    /*glm::mat4 getFrustumInsideSquare(float fov, int resX, int resY, float zNear, float zFar)
    {
        assert(fov > 0.0f && fov < 180.0f);
        assert(resX > 0 && resY > 0);
        //assert(zNear > 0.0f);
        //assert(zFar > zNear);

        float x = static_cast<float>(resX);
        float y = static_cast<float>(resY);
        float zNearSide = (fov / 90.0f) * zNear;

        if (x > y) {
            float zNearSideTruncated = zNearSide * (y / x);
            return glm::frustum(-zNearSide,          zNearSide,          -zNearSideTruncated, zNearSideTruncated, zNear, zFar);
        } else if (x < y) {
            float zNearSideTruncated = zNearSide * (x / y);
            return glm::frustum(-zNearSideTruncated, zNearSideTruncated, -zNearSide,          zNearSide,          zNear, zFar);
        } else {
            return glm::frustum(-zNearSide,          zNearSide,          -zNearSide,          zNearSide,          zNear, zFar);
        }
    }*/

    /**
        Reversed Infinite Projection Matrix (1=near, 0=far)

        Part of this from: http://timothylottes.blogspot.de/2014/07/infinite-projection-matrix-notes.html

        This matrix is for use with GL_ARB_clip_control (core in 4.5) or GL_NV_depth_buffer_float

        With GL_ARB_clip_control:
        Use a float depht buffer texture
            glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
            glDepthFunc(GL_GREATER); //default would be GL_LESS
            glClearDepth(0.0);       //default would be 1.0f

        With GL_NV_depth_buffer_float:
        use a texture type from this extension (GL_DEPTH_COMPONENT32F_NV or GL_DEPTH32F_STENCIL8_NV)
        glDepthRangedNV(-1.0, 1.0);
        glDepthFunc(GL_GREATER); //default would be GL_LESS
        clearDepth(0.0f);        //default would be 1.0f

    TODO: Get this into GLM
    */
    glm::mat4 infinitePerspectiveReversed(
        float fovy,
        float aspect,
        float zNear
    ) {
        const float f = 1.0f / tan(fovy / 2.0f); // 1.0 / tan(X) == cotangent(X)
        glm::mat4 projectionMatrix =
        {
            f/aspect, 0.0f,    0.0f,  0.0f,
                0.0f,    f,    0.0f,  0.0f,
                0.0f, 0.0f,    0.0f, -1.0f,
                0.0f, 0.0f,   zNear,  0.0f
        };
        return projectionMatrix;
    }

    glm::dmat4 infinitePerspectiveReversed(
        double fovy,
        double aspect,
        double zNear
    ) {
        const double f = 1.0f / tan(fovy / 2.0f); // 1.0 / tan(X) == cotangent(X)
        glm::dmat4 projectionMatrix =
        {
            f/aspect, 0.0,     0.0,  0.0,
                 0.0,   f,     0.0,  0.0,
                 0.0, 0.0,     0.0, -1.0,
                 0.0, 0.0,   zNear,  0.0
        };
        return projectionMatrix;
    }

    /*int getGLenumInternalFormatSize()
    {
        GL_ALPHA,
        GL_ALPHA4,
        GL_ALPHA8,
        GL_ALPHA12,
        GL_ALPHA16,
        GL_COMPRESSED_ALPHA,
        GL_COMPRESSED_LUMINANCE,
        GL_COMPRESSED_LUMINANCE_ALPHA,
        GL_COMPRESSED_INTENSITY,
        GL_COMPRESSED_RGB,
        GL_COMPRESSED_RGBA,
        GL_DEPTH_COMPONENT,
        GL_DEPTH_COMPONENT16,
        GL_DEPTH_COMPONENT24,
        GL_DEPTH_COMPONENT32,
        GL_LUMINANCE,
        GL_LUMINANCE4,
        GL_LUMINANCE8,
        GL_LUMINANCE12,
        GL_LUMINANCE16,
        GL_LUMINANCE_ALPHA,
        GL_LUMINANCE4_ALPHA4,
        GL_LUMINANCE6_ALPHA2,
        GL_LUMINANCE8_ALPHA8,
        GL_LUMINANCE12_ALPHA4,
        GL_LUMINANCE12_ALPHA12,
        GL_LUMINANCE16_ALPHA16,
        GL_INTENSITY,
        GL_INTENSITY4,
        GL_INTENSITY8,
        GL_INTENSITY12,
        GL_INTENSITY16,
        GL_R3_G3_B2,
        GL_RGB,
        GL_RGB4,
        GL_RGB5,
        GL_RGB8,
        GL_RGB10,
        GL_RGB12,
        GL_RGB16,
        GL_RGBA,
        GL_RGBA2,
        GL_RGBA4,
        GL_RGB5_A1,
        GL_RGBA8,
        GL_RGB10_A2,
        GL_RGBA12,
        GL_RGBA16,
        GL_SLUMINANCE,
        GL_SLUMINANCE8,
        GL_SLUMINANCE_ALPHA,
        GL_SLUMINANCE8_ALPHA8,
        GL_SRGB,
        GL_SRGB8,
        GL_SRGB_ALPHA, or
        GL_SRGB8_ALPHA8.
    }*/
}
