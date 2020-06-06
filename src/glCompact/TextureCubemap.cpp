#include "glCompact/TextureCubemap.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    TextureCubemap::TextureCubemap(
        SurfaceFormat surfaceFormat,
        uint32_t      xy,
        bool          mipmaps
    ) {
        UNLIKELY_IF (xy > getMaxXY())
            throw runtime_error("Trying to create TextureCubemap with size(xy = " + to_string(xy) + "), but that is bayond getMaxXY(GL_MAX_CUBE_MAP_TEXTURE_SIZE = " + to_string(threadContextGroup_->values.GL_MAX_CUBE_MAP_TEXTURE_SIZE) + ")");
        create(GL_TEXTURE_CUBE_MAP, surfaceFormat, {xy, xy, 1}, mipmaps, 0);
    }

    /*
        Returns maximum supported xy size for a cubemap texture. Minimum supported value is 1024.
    */
    uint32_t TextureCubemap::getMaxXY() {
        return threadContextGroup_->values.GL_MAX_CUBE_MAP_TEXTURE_SIZE;
    }
}

/*TextureCubemap::TextureCubemap(ImageFormat imageFormat, Texture2dArray&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_3D, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}
TextureCubemap::TextureCubemap(ImageFormat imageFormat, TextureCubemap&      srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_3D, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}
TextureCubemap::TextureCubemap(ImageFormat imageFormat, TextureCubemapArray& srcImages, int firstMipmap, bool mipmaps, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_3D, imageFormat, firstMipmap, mipmaps, layer, layerCount);
}*/
