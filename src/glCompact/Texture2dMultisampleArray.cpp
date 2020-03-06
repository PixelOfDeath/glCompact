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
#include "glCompact/Texture2dMultisampleArray.hpp"
#include "glCompact/gl/Constants.hpp"
//#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    Texture2dMultisampleArray::Texture2dMultisampleArray(
        SurfaceFormat surfaceFormat,
        uint32_t      x,
        uint32_t      y,
        uint32_t      layers,
        uint32_t      samples
    ) {
        UNLIKELY_IF (x > getMaxXY() || y > getMaxXY())
            throw runtime_error("Trying to create Texture2dMultisampleArray with size(x = " + to_string(x) + ", y = " + to_string(y) + "), but that is bayond getMaxXY(GL_MAX_TEXTURE_SIZE = " + to_string(threadContextGroup->values.GL_MAX_TEXTURE_SIZE) + ")");
        UNLIKELY_IF (layers > getMaxLayers())
            throw runtime_error("Trying to create Texture2dMultisampleArray with size(layers = " + to_string(layers) + "), but that is bayond getMaxLayers(GL_MAX_ARRAY_TEXTURE_LAYERS = " + to_string(threadContextGroup->values.GL_MAX_ARRAY_TEXTURE_LAYERS) + ")");
        //TODO: check sample limits
        create(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, surfaceFormat, x, y, layers, false, samples);
    }

    /*
        Returns maximum supported x and y size. Minimum supported value is 1024.
    */
    uint32_t Texture2dMultisampleArray::getMaxXY() {
        return threadContextGroup->values.GL_MAX_TEXTURE_SIZE;
    }

    /*
        Returns maximum supported layer count. Minimum supported value is 256.
    */
    uint32_t Texture2dMultisampleArray::getMaxLayers() {
        return threadContextGroup->values.GL_MAX_ARRAY_TEXTURE_LAYERS;
    }

    /*
        Returns the maximum supported samples for all texture rgba formats (except non-normalized integer formats)
    */
    uint32_t Texture2dMultisampleArray::getMaxSamplesRgbaNormalizedOrFloat() {
        return threadContextGroup->values.GL_MAX_COLOR_TEXTURE_SAMPLES;
    }

    /*
        Returns the maximum supported samples for all texture depth and/or stencil formats
    */
    uint32_t Texture2dMultisampleArray::getMaxSamplesDepthAndOrStencil() {
        return threadContextGroup->values.GL_MAX_DEPTH_TEXTURE_SAMPLES;
    }

    /*
        Returns the maximum supported samples for non-normalized rgba integer formats
    */
    uint32_t Texture2dMultisampleArray::getMaxSamplesRgbaInteger() {
        return threadContextGroup->values.GL_MAX_INTEGER_SAMPLES;
    }
}

/*Texture2dMultisampleArray::Texture2dMultisampleArray(ImageFormat imageFormat, Texture2dMultisample&      srcImages) {
    createView(srcImages, GL_TEXTURE_2D_MULTISAMPLE_ARRAY, imageFormat, 0, 0, 0, 1);
}
Texture2dMultisampleArray::Texture2dMultisampleArray(ImageFormat imageFormat, Texture2dMultisampleArray& srcImages, int layer, int layerCount) {
    createView(srcImages, GL_TEXTURE_2D_MULTISAMPLE_ARRAY, imageFormat, 0, 0, layer, layerCount);
}*/
