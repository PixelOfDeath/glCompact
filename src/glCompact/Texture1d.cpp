#include "glCompact/Texture1d.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    Texture1d::Texture1d(
        SurfaceFormat surfaceFormat,
        uint32_t      x,
        bool          mipmaps
    ) {
        UNLIKELY_IF (x > getMaxX())
            throw runtime_error("Trying to create Texture1d with size(x = " + to_string(x) + "), but that is bayond getMaxX(GL_MAX_TEXTURE_SIZE = " + to_string(threadContextGroup_->values.GL_MAX_TEXTURE_SIZE) + ")");
        create(GL_TEXTURE_1D, surfaceFormat, {x, 1, 1}, mipmaps, 0);
    }

    /*
        returns the maximum supported X size for a 1d texture. The minimum supported value is 1024.
    */
    uint32_t Texture1d::getMaxX() {
        return threadContextGroup_->values.GL_MAX_TEXTURE_SIZE;
    }
}

/*Texture1d::Texture1d(ImageFormat imageFormat, Texture1d& srcImages, int firstMipmap, bool mipmaps) {
    createView(srcImages, GL_TEXTURE_1D, imageFormat, firstMipmap, mipmaps, 0, 1);
}
Texture1d::Texture1d(ImageFormat imageFormat, Texture1dArray& srcImages, int firstMipmap, bool mipmaps, int layer) {
    createView(srcImages, GL_TEXTURE_1D, imageFormat, firstMipmap, mipmaps, layer, 1);
}*/
