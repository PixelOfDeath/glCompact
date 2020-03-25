#include "glCompact/Texture2d.hpp"
#include "glCompact/gl/Constants.hpp"
//#include "glCompact/ThreadContext.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    Texture2d::Texture2d(
        SurfaceFormat surfaceFormat,
        uint32_t      x,
        uint32_t      y,
        bool          mipmaps
    ) {
        UNLIKELY_IF (x > getMaxXY() || y > getMaxXY())
            throw runtime_error("Trying to create Texture2d with size(x = " + to_string(x) + ", y = " + to_string(y) + "), but that is bayond getMaxXY(GL_MAX_TEXTURE_SIZE = " + to_string(threadContextGroup_->values.GL_MAX_TEXTURE_SIZE) + ")");
        create(GL_TEXTURE_2D, surfaceFormat, x, y, 1, mipmaps, 0);
    }

    /*
        Returns maximum supported x and y size. Minimum supported value is 1024.
    */
    uint32_t Texture2d::getMaxXY() {
        return threadContextGroup_->values.GL_MAX_TEXTURE_SIZE;
    }
}

/*Texture2d::Texture2d(ImageFormat imageFormat, Texture2d&           srcImages, int firstMipmap, bool mipmaps) {
    createView(srcImages, GL_TEXTURE_2D, imageFormat, firstMipmap, mipmaps, 0, 1);
}
Texture2d::Texture2d(ImageFormat imageFormat, Texture2dArray&      srcImages, int firstMipmap, bool mipmaps, int layer) {
    createView(srcImages, GL_TEXTURE_2D, imageFormat, firstMipmap, mipmaps, layer, 1);
}
Texture2d::Texture2d(ImageFormat imageFormat, TextureCubemap&      srcImages, int firstMipmap, bool mipmaps, int layer) {
    createView(srcImages, GL_TEXTURE_2D, imageFormat, firstMipmap, mipmaps, layer, 1);
}
Texture2d::Texture2d(ImageFormat imageFormat, TextureCubemapArray& srcImages, int firstMipmap, bool mipmaps, int layer) {
    createView(srcImages, GL_TEXTURE_2D, imageFormat, firstMipmap, mipmaps, layer, 1);
}*/
