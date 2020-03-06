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
/*
    GL_PIXEL_PACK_BUFFER_ARB   = BUFFER <- TEXTURE
    GL_PIXEL_UNPACK_BUFFER_ARB = BUFFER -> TEXTURE

    GL_ARB_texture_cube_map                 (Core since 1.3)
    GL_ARB_texture_compression              (Core since 1.3)
    GL_ARB_texture_non_power_of_two         (Core since 2.0)
    GL_ARB_pixel_buffer_object              (Core since 2.1)
    GL_ARB_texture_float                    (Core since 3.0)
    GL_ARB_texture_compression_rgtc         (Core since 3.0)
    GL_ARB_Texture_multisample              (Core since 3.2)
    GL_ARB_texture_rgb10_a2ui               (Core since 3.3)
    ----------
    GL_ARB_texture_cube_map_array           (Core since 4.0, uses funcARB names!)
    GL_ARB_texture_storage                  (Core since 4.2)
    GL_ARB_compressed_texture_pixel_storage (Core since 4.2) ("Allow pixel storage parameters to affect packing and unpacking of compressed textures")
    GL_ARB_texture_storage_multisample      (Core since 4.3)
    GL_ARB_stencil_texturing                (Core since 4.3)
    GL_ARB_Texture_view                     (core since 4.3)
    GL_ARB_texture_stencil8                 (Core since 4.4)
    GL_ARB_get_Texture_sub_image            (Core since 4.5)

    GL_EXT_texture_compression_s3tc         (not Core, Ubiquitous Extension)
    GL_EXT_texture_sRGB                     (not Core, Ubiquitous Extension)
    GL_ARB_texture_compression_bptc         (core since 4.2)

    GL_ARB_SPARSE_TEXTURE                   (not Core)
    GL_AMD_SPARSE_TEXTURE                   (not Core)
    GL_AMD_SPARSE_TEXTURE_POOL              (not Core)
    GL_ARB_SPARSE_TEXTURE2                  (not Core)

    GL_MAX_TEXTURE_SIZE          (min. 1024)
    GL_MAX_ARRAY_TEXTURE_LAYERS  (min.  256)
    GL_MAX_CUBE_MAP_TEXTURE_SIZE (min. 1024)
    GL_MAX_RENDERBUFFER_SIZE     (min.   64)
S

    sample count must be
        GL_MAX_DEPTH_TEXTURE_SAMPLES //for depth and depth_stencil formats (not 100% if this also is used for stencil only formats)
        GL_MAX_FRAMEBUFFER_SAMPLES   //only relevant for attachmentsless FBOs

        GL_MAX_SAMPLES               //?????????????
        GL_MAX_COLOR_TEXTURE_SAMPLES //for color renderable formats, max samples for normalized or float R/RG/RGB/RGBA formats
        GL_MAX_DEPTH_TEXTURE_SAMPLES //depth- or stencil-renderable format
        GL_MAX_INTEGER_SAMPLES       //for non normalized signed/unsigned formats: e.g. GL_RGBA8I, GL_RED8UI

    TextureInterface
    1d
    |
    |   2d
    |   | 3d
    |   | | cube
    |   | | | array
    |   | | | | multisample
    |   | | | | | Texture
    |   | | | | | | RenderBuffer
    |   | | | | | | |
    X             X   Texture1d
    X X           X   Texture1dArray (except for mipmap and sampeling behaves like a 2d texture)
        X         X   Texture2d
        X     X   X   Texture2dArray
        X       X X   Texture2dMs
        X     X X X   Texture2dMsArray
          X       X   Texture3d
        X   X     X   Texture2dCubemap
        X   X X   X   Texture2dCubemapArray
        X           X RenderBuffer2d
        X       X   X RenderBuffer2dMs

    fixedSampleLocations is so badly documented that atm I won't even include it as a option to disable it. One less thing to worry about when creating or copying a framebuffer.
*/

/** \class glCompact::TextureInterface
    GL3.3 doc page 134:
    Final Expansion to RGBA
    This step is performed only for non-depth component groups. Each group is con-
    verted to a group of 4 elements as follows: if a group does not contain an A ele-
    ment, then A is added and set to 1 for integer components or 1.0 for floating-point
    components. If any of R, G, or B is missing from the group, each missing element
    is added and assigned a value of 0 for integer components or 0.0 for floating-point
    components.

    When setting only some components this are the default values: R 0 0 1
    G and B default is 0, alpha default is 1

    Only found render/sample req. list in gl4.5 spec, not in gl3.3 spec :(


    All compressed formats use 4x4 texel blocks.
    GL_ARB_texture_compression_rgtc (Core in 3.0)
        GL_COMPRESSED_RED_RGTC1
        GL_COMPRESSED_SIGNED_RED_RGTC1
        GL_COMPRESSED_RG_RGTC2
        GL_COMPRESSED_SIGNED_RG_RGTC2
    GL_EXT_texture_compression_s3tc (Not Core)
        GL_COMPRESSED_RGB_S3TC_DXT1_EXT
        GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
        GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
        GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
    GL_ARB_texture_compression_bptc
        GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT
        GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT
        GL_COMPRESSED_RGBA_BPTC_UNORM
        GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM
*/

#include "glCompact/TextureInterface.hpp"

#include "glCompact/ToolsInternal.hpp"
#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"
#include "glCompact/GlTools.hpp"
#include "glCompact/BufferInterface.hpp"
#include "glCompact/Frame.hpp"

#include "glCompact/Sampler.hpp"
#include "glCompact/SurfaceFormatDetail.hpp"
#include "glCompact/MemorySurfaceFormatDetail.hpp"

#include <stdexcept>

#include <glm/glm.hpp>

#include <algorithm> //for min/max in msvc
#include <string>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    TextureInterface::~TextureInterface() {
        free();
    }

    /*TextureInterface::TextureInterface(const TextureInterface& srcTextureInterface) {
        create(srcTextureInterface.target_, srcTextureInterface.imageFormat, srcTextureInterface.x, srcTextureInterface.y, srcTextureInterface.z, srcTextureInterface.mipmapCount > 1, srcTextureInterface.samples);
        LOOPI(srcTextureInterface.mipmapCount)
            copyFromImageComponents(srcTextureInterface, i, {0, 0, 0}, i, {0, 0, 0}, {srcTextureInterface.x, srcTextureInterface.y, srcTextureInterface.z});
    }

    TextureInterface& TextureInterface::operator=(const TextureInterface& srcImages) {
        if (this != &srcImages) {
            create(srcImages.target_, srcImages.imageFormat, srcImages.x, srcImages.y, srcImages.z, srcImages.mipmapCount > 1, srcImages.samples);
            LOOPI(srcImages.mipmapCount)
                copyFromImageComponents(srcImages, i, {0, 0, 0}, i, {0, 0, 0}, {srcImages.x, srcImages.y, srcImages.z});
        }
        return *this;
    }

    TextureInterface::TextureInterface(TextureInterface&& images) {
        this->id_          = images.id_;
        this->target_      = images.target_;
        this->x_           = images.x_;
        this->y_           = images.y_;
        this->z_           = images.z_;
        this->mipmapCount_ = images.mipmapCount_;
        this->samples_     = images.samples_;
        this->imageFormat_ = images.imageFormat_;
        images.id_         = 0;
    }

    TextureInterface& TextureInterface::operator=(TextureInterface&& images) {
        if (this != &images) {
            free();
            this->id_          = images.id_;
            this->target_      = images.target_;
            this->x_           = images.x_;
            this->y_           = images.y_;
            this->z_           = images.z_;
            this->mipmapCount_ = images.mipmapCount_;
            this->samples_     = images.samples_;
            this->imageFormat_ = images.imageFormat_;
            images.id_         = 0;
        }
        return *this;
    }*/



    /**
        Create a view from an existing texture.

        @param sourceImages
        @param internalFormat can be 0 to use sourceImages internalFormat
    */
    /*void TextureInterface::createView2d(TextureInterface *sourceImages, GLenum internalFormat, int layer, int minLevel, int numLevels) {
        free();
        if (numLevels == 0) return;

        if (minLevel < sourceImages->mipmapCount_) return;

        const FormatEntry* formatEntry;
        if (!internalFormat) {
            internalFormat = sourceImages->formatEntry->sizedFormat;
            formatEntry    = sourceImages->formatEntry;
        } else {
            formatEntry = findSizedFormatEntry(internalFormat);
            if (!formatEntry) {
                threadContext->feedbackHandler(FEEDBACK_TYPE_ERROR, "unknown internalFormat");
                return;
            }
            //test if the formats match in byte size
        }

        threadContext->glGenTextures(1, &id_);
        //this->target_ = target;
        //bindTemporalFirstTime(); //<- do I need this for textureView? Actually the opposite may be the case. It needs to be NOT bound before!

        threadContext->glTextureView(id, GL_TEXTURE_2D, sourceImages->id, internalFormat, minLevel, numLevels, layer, 1);
        //void glTextureView​(GLuint texture​, GLenum target​, GLuint origtexture​, GLenum internalformat​, GLuint minlevel​, GLuint numlevels​, GLuint minlayer​, GLuint numlayers​)

        //internalFormat

        glm::ivec3 mipmapLevelSize = sourceImages->getMipmapLevelSize(minLevel);

        this->formatEntry_          = formatEntry;
        this->target_               = GL_TEXTURE_2D;
        this->mipmapCount_          = 1;
        this->x_                    = mipmapLevelSize.x;
        this->y_                    = mipmapLevelSize.y;
        this->z_                    = 1;
        this->samples_              = samples;
    }*/

    //void TextureInterface::createView2dArray(TextureInterface *sourceImages, GLenum internalFormat, int minLevel, int numLevels, int minLayer, int numLayers)

    /**
        @param target
        @param internalFormat
        @param x
        @param y
        @param z
        @param mipmap
        @param samples

        Internal:
            This uses ARB DSA and/or texture_storage if available.

            We completely ignore GL_EXT_direct_state_access for texture creation, because it depends on first bind to get all parameters (texture target) before OpenGL actually can create the texture.
            Also Khronos forgot to define functions for creating GL_TEXTURE_2D_MULTISAMPLE/GL_TEXTURE_2D_MULTISAMPLE_ARRAY in GL_EXT_direct_state_access.

            There are three paths of texture creation here:
            - With    ARB DSA, with    texture_storage
            - Without ARB DSA, with    texture_storage
            - Without ARB DSA, without texture_storage
    */
    void TextureInterface::create(
        int32_t       target,
        SurfaceFormat surfaceFormat,
        uint32_t      x,
        uint32_t      y,
        uint32_t      z,
        bool          mipmap,
        uint8_t       samples
    ) {
        const bool fixedSampleLocations = true;
        using namespace std;
        free();
        int mipmapCount = 1;
        if (mipmap) switch (target) {
            case GL_TEXTURE_2D_MULTISAMPLE:
            case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: mipmapCount = 1; break;
            case GL_TEXTURE_1D:
            case GL_TEXTURE_1D_ARRAY:             mipmapCount = int(ceil(log2(    x             + 1))); break;
            case GL_TEXTURE_2D:
            case GL_TEXTURE_2D_ARRAY:
            case GL_TEXTURE_CUBE_MAP:
            case GL_TEXTURE_CUBE_MAP_ARRAY:       mipmapCount = int(ceil(log2(max(x,     y)     + 1))); break;
            case GL_TEXTURE_3D:                   mipmapCount = int(ceil(log2(max(x, max(y, z)) + 1))); break;
        }

        int mipmapLevelX = x;
        int mipmapLevelY = y;
        int mipmapLevelZ = z;

        bool isMultiSample =
               target == GL_TEXTURE_2D_MULTISAMPLE
            || target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
        if (isMultiSample) {
            //TODO: include other sizes, e.g. 6 on nv
            UNLIKELY_IF (!(samples == 1 || samples == 2 || samples == 4 || samples == 8)) {
                throw runtime_error("samples must be 1, 2, 4 or 8");
                return;
            }
        }
        //from here on we actually change data after free()

        bool usingTexStorage = false;

        GLenum sizedFormat = surfaceFormat->sizedFormat;

        //Theoretical GL_ARB_direct_state_access could be supported without GL_ARB_texture_storage or GL_ARB_texture_storage_multisample.
        //But its unlikely and in that case we just use the old style path
        if (threadContextGroup->extensions.GL_ARB_direct_state_access && threadContextGroup->extensions.GL_ARB_texture_storage && threadContextGroup->extensions.GL_ARB_texture_storage_multisample) {
            threadContextGroup->functions.glCreateTextures(target, 1, &id);
            switch (target) {
                case GL_TEXTURE_1D                  : threadContextGroup->functions.glTextureStorage1D                   (id, mipmapCount, sizedFormat, x);       break;
                case GL_TEXTURE_1D_ARRAY            : threadContextGroup->functions.glTextureStorage2D                   (id, mipmapCount, sizedFormat, x, y);    break;
                case GL_TEXTURE_2D                  : threadContextGroup->functions.glTextureStorage2D                   (id, mipmapCount, sizedFormat, x, y);    break;
                case GL_TEXTURE_2D_ARRAY            : threadContextGroup->functions.glTextureStorage3D                   (id, mipmapCount, sizedFormat, x, y, z); break;
                case GL_TEXTURE_CUBE_MAP            : threadContextGroup->functions.glTextureStorage2D                   (id, mipmapCount, sizedFormat, x, y);    break;
                case GL_TEXTURE_CUBE_MAP_ARRAY      : threadContextGroup->functions.glTextureStorage3D                   (id, mipmapCount, sizedFormat, x, y, z); break;
                case GL_TEXTURE_3D                  : threadContextGroup->functions.glTextureStorage3D                   (id, mipmapCount, sizedFormat, x, y, z); break;

                case GL_TEXTURE_2D_MULTISAMPLE      : threadContextGroup->functions.glTextureStorage2DMultisample        (id, samples,     sizedFormat, x, y,    fixedSampleLocations); break;
                case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: threadContextGroup->functions.glTextureStorage3DMultisample        (id, samples,     sizedFormat, x, y, z, fixedSampleLocations); break;
            }
            usingTexStorage = true;
        } else {
            //TODO: do compressed formats need different function calls here? For example glCompressedTexImage2D?
            //compressed formats work fine here in win7+catalyst and r600 on 5770-Juniper.
            //Seems this functions only exist for creation + upload at the same time. But we completely separates creation and upload, so we don't have any use for them right now.

            threadContextGroup->functions.glGenTextures(1, &id);
            this->target = target;
            bindTemporalFirstTime();

            if (!isMultiSample && threadContextGroup->extensions.GL_ARB_texture_storage) {
                switch (target) {
                    case GL_TEXTURE_1D                  : threadContextGroup->functions.glTexStorage1D           (target, mipmapCount, sizedFormat, x);       break;
                    case GL_TEXTURE_1D_ARRAY            : threadContextGroup->functions.glTexStorage2D           (target, mipmapCount, sizedFormat, x, y);    break;
                    case GL_TEXTURE_2D                  : threadContextGroup->functions.glTexStorage2D           (target, mipmapCount, sizedFormat, x, y);    break;
                    case GL_TEXTURE_2D_ARRAY            : threadContextGroup->functions.glTexStorage3D           (target, mipmapCount, sizedFormat, x, y, z); break;
                    case GL_TEXTURE_CUBE_MAP            : threadContextGroup->functions.glTexStorage2D           (target, mipmapCount, sizedFormat, x, y);    break;
                    case GL_TEXTURE_CUBE_MAP_ARRAY      : threadContextGroup->functions.glTexStorage3D           (target, mipmapCount, sizedFormat, x, y, z); break;
                    case GL_TEXTURE_3D                  : threadContextGroup->functions.glTexStorage3D           (target, mipmapCount, sizedFormat, x, y, z); break;
                }
                usingTexStorage = true;
            } else if (isMultiSample && threadContextGroup->extensions.GL_ARB_texture_storage_multisample) {
                switch (target) {
                    case GL_TEXTURE_2D_MULTISAMPLE      : threadContextGroup->functions.glTexStorage2DMultisample(target, samples, sizedFormat, x, y,    fixedSampleLocations); break;
                    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: threadContextGroup->functions.glTexStorage3DMultisample(target, samples, sizedFormat, x, y, z, fixedSampleLocations); break;
                }
                usingTexStorage = true;
            } else {
                //Border colour is still listed in 4.5 core profile. But it is MARKED as deprecated. In a forward compatible context the border width MUST be zero!

                //we only need the border value to be non-zero for some older compressed formats.
                //I guess this formats are all marked as deprecated?!
                //Border colour is also part of a sampler object
                //TEXTURE_BORDER_COLOR
                //GLint  border       = formatEntry->border;
                GLint  border = 0;
                //components and componentsTypes don't get used as a parameter, but they still have to be valid values!
                //TODO: Does every format need to be in list or can we just use GL_RED, GL_UNSIGNED_BYTE for everything?
                GLenum components      = GL_RED;
                GLenum componentsTypes = GL_UNSIGNED_BYTE;

                switch (target) {
                    case GL_TEXTURE_1D:
                        LOOPI(mipmapCount) {
                            threadContextGroup->functions.glTexImage1D(target, i, sizedFormat, mipmapLevelX, border, components, componentsTypes, 0);
                            mipmapLevelX = max(1, mipmapLevelX / 2);
                        }
                        break;
                    case GL_TEXTURE_1D_ARRAY:
                        LOOPI(mipmapCount) {
                            //TODO test layers parameter
                            threadContextGroup->functions.glTexImage2D(target, i, sizedFormat, mipmapLevelX, y, border, components, componentsTypes, 0);
                            mipmapLevelX = max(1, mipmapLevelX / 2);
                        }
                        break;
                    case GL_TEXTURE_2D:
                        LOOPI(mipmapCount) {
                            threadContextGroup->functions.glTexImage2D(target, i, sizedFormat, mipmapLevelX, mipmapLevelY, border, components, componentsTypes, 0);
                            mipmapLevelX = max(1, mipmapLevelX / 2);
                            mipmapLevelY = max(1, mipmapLevelY / 2);
                        }
                        break;
                    case GL_TEXTURE_2D_MULTISAMPLE:
                        threadContextGroup->functions.glTexImage2DMultisample(target, samples, sizedFormat, x, y, fixedSampleLocations);
                        break;
                    case GL_TEXTURE_2D_ARRAY:
                        LOOPI(mipmapCount) {
                            //TODO test layers parameter
                            threadContextGroup->functions.glTexImage3D(target, i, sizedFormat, mipmapLevelX, mipmapLevelY, z, border, components, componentsTypes, 0);
                            mipmapLevelX = max(1, mipmapLevelX / 2);
                            mipmapLevelY = max(1, mipmapLevelY / 2);
                        }
                        break;
                    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                        threadContextGroup->functions.glTexImage3DMultisample(target, samples, sizedFormat, x, y, z, fixedSampleLocations);
                        break;
                    //TODO test cube map and cube map array creation
                    //TODO: documentation is shit again and says nothing about the non array version...
                    case GL_TEXTURE_CUBE_MAP:
                        LOOPI(mipmapCount) {
                            LOOPJ(6)
                                threadContextGroup->functions.glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, i, sizedFormat, mipmapLevelX, mipmapLevelY, border, components, componentsTypes, 0);
                            mipmapLevelX = max(1, mipmapLevelX / 2);
                            mipmapLevelY = max(1, mipmapLevelY / 2);
                        }
                        break;
                    case GL_TEXTURE_CUBE_MAP_ARRAY:
                        LOOPI(mipmapCount) {
                            //LOOPJ(6)
                            //    glTexImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, i, sizedFormat, mipmapLevelX, mipmapLevelY, layers, border, components, componentsTypes, 0);
                            //TODO test layers parameter
                                //glTexImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, i, sizedFormat, mipmapLevelX, mipmapLevelY, layers, border, GL_RED, GL_UNSIGNED_BYTE, 0);
                                //glTexImage3D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, i, sizedFormat, mipmapLevelX, mipmapLevelY, layers, border, GL_RED, GL_UNSIGNED_BYTE, 0);
                                //glTexImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, i, sizedFormat, mipmapLevelX, mipmapLevelY, layers, border, GL_RED, GL_UNSIGNED_BYTE, 0);
                                //glTexImage3D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, i, sizedFormat, mipmapLevelX, mipmapLevelY, layers, border, GL_RED, GL_UNSIGNED_BYTE, 0);
                                //glTexImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, i, sizedFormat, mipmapLevelX, mipmapLevelY, layers, border, GL_RED, GL_UNSIGNED_BYTE, 0);
                                //glTexImage3D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, i, sizedFormat, mipmapLevelX, mipmapLevelY, layers, border, GL_RED, GL_UNSIGNED_BYTE, 0);
                            threadContextGroup->functions.glTexImage3D(target, i, sizedFormat, mipmapLevelX, mipmapLevelY, z, border, components, componentsTypes, 0);
                            mipmapLevelX = max(1, mipmapLevelX / 2);
                            mipmapLevelY = max(1, mipmapLevelY / 2);
                        }
                        break;
                    case GL_TEXTURE_3D:
                        LOOPI(mipmapCount) {
                            threadContextGroup->functions.glTexImage3D(target, i, sizedFormat, mipmapLevelX, mipmapLevelY, mipmapLevelZ, border, components, componentsTypes, 0);
                            mipmapLevelX = max(1, mipmapLevelX / 2);
                            mipmapLevelY = max(1, mipmapLevelY / 2);
                            mipmapLevelZ = max(1, mipmapLevelZ / 2);
                        }
                        break;
                }
            }
        }

        /*
            With texture storage the mipmap range is automaticly limited to its actually size. Old style texture creation needs this limits set manually.
            Because we always create a full texture from level 0 to smallest mipmap (if mipmaps are used), we only have to set GL_TEXTURE_MAX_LEVEL.
            This prevents any sampler to go bayond the texture mipmap range and therefor one less chance for it to cause an error because of the sampling state being considered incomplite.
        */
        //setTextureParameter(GL_TEXTURE_BASE_LEVEL, int32_t(0)); //default value, no need to set it

        //Hat some bug issues before with the catalyst/win7 driver. Where setting GL_TEXTURE_MAX_LEVEL to anything other then the default 1000 using texStorage without mipmaps breaks something
        //without any error feedback. Sampling would always retunr null data. Is no longer an issue, but there is no need to set this when using texture storage anyway.
        if (!usingTexStorage) {
            setTextureParameter(GL_TEXTURE_MAX_LEVEL, int32_t(mipmapCount - 1));
        }

        //Maybe only use GL default init values here? (NEAREST_MIPMAP_LINEAR, GL_LINEAR)
        if (!surfaceFormat->isCompressed & surfaceFormat->isRgbaInteger) {
            //Integer textures do not allow linear filtering but they can come with mipmaps.
            setTextureParameter(GL_TEXTURE_MIN_FILTER, mipmap ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
            setTextureParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            //Set trilinear filtering as default for textures with mipmaps.
            setTextureParameter(GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            setTextureParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        //GL_ARB_texture_storage says they changed default warping mode to GL_CLAMP_TO_EDGE
        //Old style textures use GL_REPEAT and ref 4.6 also only lists GL_REPEAT as the default value (except for rectangle textures using CLAMP_TO_EDGE, but we don't use them at all anyway)
        //setTextureParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
        //setTextureParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
        //setTextureParameter(GL_TEXTURE_WRAP_R, GL_REPEAT);

        this->mipmapCount   = mipmapCount;
        this->target        = target;
        this->x             = x;
        this->y             = y;
        this->z             = z;
        this->samples       = samples;
        this->surfaceFormat = surfaceFormat;
    }

    /*
        ARB_texture_view (Core since 4.3)

    */
    void TextureInterface::createView(
        TextureInterface& srcImages,
        int32_t           newTarget,
        SurfaceFormat     newSurfaceFormat,
        uint32_t          firstMipmap,
        bool              mipmap,
        uint32_t          firstLayer,
        uint32_t          layerCount
    ) {
        UNLIKELY_IF (!threadContextGroup->extensions.GL_ARB_texture_view)
            throw runtime_error("Can't create texture view! GL_ARB_texture_view not supported!");
        UNLIKELY_IF (layerCount <= 0)
            throw runtime_error("layerCount must be a positive number!");

        //TODO: test SurfaceFormat compatibility

        target        = newTarget;
        x             = srcImages.x; //TODO, needs formula for mipmap
        y             = srcImages.y; //TODO, needs formula for mipmap (and 1d array texture!)
        z             = min(layerCount, srcImages.z);
        mipmapCount   = mipmap ? firstMipmap - srcImages.mipmapCount : 1;
        samples       = 0;
        surfaceFormat = newSurfaceFormat;

        //GL_ARB_direct_state_access glCreateTextures does NOT work here //tested with win10-blob and mesa (on r290)
        threadContextGroup->functions.glGenTextures(1, &id);
        threadContextGroup->functions.glTextureView(id, newTarget, srcImages.id, newSurfaceFormat->sizedFormat, firstMipmap, mipmapCount, firstLayer, 1);
    }

    //TODO: shall this always return 1 (what it does right now!) for the "not existing" dimensions? So it can be used in multiplications without checking?
    glm::ivec3 TextureInterface::getMipmapLevelSize(
        uint32_t mipmapLevel
    ) {
        glm::ivec3 size(0);
        if (mipmapLevel >= mipmapCount) return size; //error or just return 0, 0, 0?
        uint32_t mipmapScale = pow(2, mipmapLevel); //integerPowerOf(2, mipmapLevel);
        size.x = max<uint32_t>(1, x / mipmapScale);
        size.y = max<uint32_t>(1, (target != GL_TEXTURE_1D_ARRAY) ? (y / mipmapScale) : y);
        size.z = max<uint32_t>(1, (target == GL_TEXTURE_3D)       ? (z / mipmapScale) : z);
        return size;
    }

    /**
        @param mem
        @param bufSize
        @param memorySurfaceFormat
        @param dstMipmapLevel
        @param dstOffset
        @param size
    */
    void TextureInterface::copyFromMemory(
        const void*         mem,
        uintptr_t           bufSize,
        MemorySurfaceFormat memorySurfaceFormat,
        uint32_t            dstMipmapLevel,
        glm::ivec3          dstOffset,
        glm::ivec3          size
    ) {
        copyFrom(0, mem, bufSize, memorySurfaceFormat, dstMipmapLevel, dstOffset, size);
    }

    void TextureInterface::copyToMemory(
        void*               mem,
        uintptr_t           bufSize,
        MemorySurfaceFormat memorySurfaceFormat,
        uint32_t            dstMipmapLevel,
        glm::ivec3          dstOffset,
        glm::ivec3          size
    ) {
        copyTo(0, mem, bufSize, memorySurfaceFormat, dstMipmapLevel, dstOffset, size);
    }

    void TextureInterface::copyFromBuffer(
        const BufferInterface& bufferInterface,
        uintptr_t              offset,
        uintptr_t              bufSize,
        MemorySurfaceFormat    memorySurfaceFormat,
        uint32_t               dstMipmapLevel,
        glm::ivec3             dstOffset,
        glm::ivec3             size
    ) {
        copyFrom(&bufferInterface, reinterpret_cast<void*>(offset), bufSize, memorySurfaceFormat, dstMipmapLevel, dstOffset, size);
    }

    void TextureInterface::copyToBuffer(
        BufferInterface&    bufferInterface,
        uintptr_t           offset,
        uintptr_t           bufSize,
        MemorySurfaceFormat memorySurfaceFormat,
        uint32_t            dstMipmapLevel,
        glm::ivec3          dstOffset,
        glm::ivec3          size
    ) {
        copyTo(&bufferInterface, reinterpret_cast<void*>(offset), bufSize, memorySurfaceFormat, dstMipmapLevel, dstOffset, size);
    }

    /**
        This function copies pixel data from client memory or a buffer object into a texture. It handles most cases of format conversions.
        Can generate compressed texture data from RGBA data. Or copy raw compressed data directly.

        For compressed textures x, y, z and xSize, ySize, zSize must be a factor of 4


        Internals:

            Using defined pixel data to fill any kind of texture (compressed or uncompressed)
                GL_ARB_direct_state_access
                    glTextureSubImage*D
                GL_EXT_direct_state_access
                    glTextureSubImage*DEXT
                old style with binding
                    glTexSubImage*D

            Copying already compressed data directly to a compressed texture:
                GL_ARB_direct_state_access
                    glCompressedTextureSubImage*D
                GL_EXT_direct_state_access
                    glCompressedTextureSubImage*DEXT
                old style with binding
                    glCompressedTexSubImage*D

            Core since eons:
                void glGetTexImage                 (GLenum target, int32_t level, GLenum format, GLenum type, void* pixels)
                void glGetCompressedTexImage       (GLenum target, int32_t level, void *pixels )

            GL_EXT_direct_state_access (Not core)
                void glGetTextureImageEXT          (uint32_t texture, GLenum target, int32_t level, GLenum format, GLenum type, void *pixels);
                void glGetCompressedTextureImageEXT(uint32_t texture, GLenum target, int32_t level, void* img);

            this two functions come with GL4.5 (not sure if they come in with a extension or just core4.5, they are not part of ARB DSA)
                void glGetnTexImage                (GLenum tex, int32_t level, GLenum format, GLenum type, uint32_t bufSize, void* pixels)
                void glGetnCompressedTexImage      (GLenum target, int32_t level, uint32_t bufSize, void* pixels)

            GL_ARB_direct_state_access (Core in 4.5)
                void glGetTextureImage             (uint32_t Texture, int32_t level, GLenum format, GLenum type, uint32_t bufSize, void* pixels)
                void glGetCompressedTextureImage   (uint32_t Texture, int32_t level, uint32_t bufSize, void* pixels)

            GL_ARB_get_Texture_sub_image (core in 4.5)
                Without this extension there is no way of selecting sub sections of an Texture to copy to memory or buffer.
                void glGetTextureSubImage          (uint32_t Texture, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, uint32_t width, uint32_t height, uint32_t depth, GLenum format, GLenum type, uint32_t bufSize, void* pixels);
                void glGetCompressedTextureSubImage(uint32_t Texture, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, uint32_t width, uint32_t height, uint32_t depth, uint32_t bufSize, void* pixels);


            This gl variables are describing the memory gl is unpacking to or packing from:
            For texture    -> mem/buffer   PACK_...
            For mem/buffer -> texture      UNPACK_...

            SWAP_BYTES     used to switch big and little endian
            LSB_FIRST
            ROW_LENGTH     This is needed to be set for SKIP_PIXELS to work correctly? (Texture size X)
            SKIP_ROWS      yOffset
            SKIP_PIXELS    xOffset (Needs ROW_LENGTH to be set correctly?)
            ALIGNMENT
            IMAGE_HEIGHT   zSize
            SKIP_IMAGES    zOffset

            COMPRESSED_BLOCK_WIDTH
            COMPRESSED_BLOCK_HEIGHT
            COMPRESSED_BLOCK_DEPTH
            COMPRESSED_BLOCK_SIZE

            ignored for compressed images.
            To enable PACK_SKIP_PIXELS and PACK_ROW_LENGTH, the values of PACK_COMPRESSED_BLOCK_SIZE and PACK_COMPRESSED_BLOCK_WIDTH must be non-zero.
            To enable PACK_SKIP_ROWS and PACK_IMAGE_HEIGHT, the value  of PACK_COMPRESSED_BLOCK_HEIGHT                               must be non-zero.


        TODO: MESA BUG workaround: when readframebuffer is set to something else then 0, mesa will throw an error if it is not complete (basically it tries to validate states that don't matter for the operations)
        TODO can gl handle negative sizes? For flipping the image/data?
        TODO: always use alignment of 1? (Default is 4), only affects the line size for copies from or to images
        Note that bufSize is GLsizei (uint32_t), and therefor limited to 4 GiB

        RGB/RGBA vs BGR/BGRA is NOT equivalent to REV!
        GL_ABGR_EXT
        GL_PACK_SWAP_BYTES only changes the byte order of each single component, it does not change the order of the components (So for litel/big endian CPU differences?)
    */
    void TextureInterface::copyFrom(
        const BufferInterface* bufferInterface,
        const void*            offsetPointer,
        uintptr_t              bufSize,
        MemorySurfaceFormat    memorySurfaceFormat,
        uint32_t               dstMipmapLevel,
        glm::ivec3             dstOffset,
        glm::ivec3             size
    ) {
        checkSurfaceFormatCompatibleToMemorySurfaceFormat(surfaceFormat, memorySurfaceFormat);
        const int32_t componentsAndArrangement = memorySurfaceFormat->componentsAndArrangement;

        uintptr_t dataOffset = reinterpret_cast<uintptr_t>(offsetPointer);

        //validate parameters, etc...
        UNLIKELY_IF (dstMipmapLevel > this->mipmapCount)
            throw runtime_error("Mipmap level outside of texture limit");

        glm::ivec3 mipmapLevelSize = getMipmapLevelSize(dstMipmapLevel);

        UNLIKELY_IF (!id)
            throw std::runtime_error("Trying to copy to textureInterface Object without created texture!");
        UNLIKELY_IF (dstOffset.x < 0 || dstOffset.y < 0 || dstOffset.z < 0)
            throw runtime_error("x, y and z must be positive");

        UNLIKELY_IF (size.x < 1 || size.y < 1 || size.z < 1) return; //TODO: can negative size be used for inverting images vertically/horizontally?
        UNLIKELY_IF (dstOffset.x + size.x > mipmapLevelSize.x || dstOffset.y + size.y > mipmapLevelSize.y || dstOffset.z + size.z > mipmapLevelSize.z)
            throw runtime_error("Trying to access bayond image size");

        //TODO: also check the memory pointer for the alignment? Throw error if it is of? One some platforms just performance relevant? On others a hard crash?
        const int blockSizeX = surfaceFormat->blockSizeX;
        const int blockSizeY = surfaceFormat->blockSizeY;
        if (surfaceFormat->isCompressed) {
            UNLIKELY_IF (dstOffset.x % blockSizeX || dstOffset.y % blockSizeY || size.x % blockSizeX || size.y % blockSizeY)
                throw runtime_error("For compressed textures dstOffset.xy(" + to_string(dstOffset.x) + ", " + to_string(dstOffset.y) + ") and size.xy(" +
                to_string(size.x) + ", " + to_string(size.y) + ") must aligned with the block size(" + to_string(blockSizeX) + ", " + to_string(blockSizeY) + ")");
        }
        uintptr_t requiredBufferSize;
        if (memorySurfaceFormat->isCompressed) {
            requiredBufferSize = (memorySurfaceFormat->bitsPerPixelOrBlock / 8) * (std::max(blockSizeX, size.x) / blockSizeX) * (std::max(blockSizeY, size.y) / blockSizeY) * std::max(1, size.z);
        } else {
            requiredBufferSize = (memorySurfaceFormat->bitsPerPixelOrBlock / 8) * std::max(1, size.x) * std::max(1, size.y) * std::max(1, size.z);
        }

        if (bufferInterface) {
            UNLIKELY_IF (bufferInterface->size_ == 0)
                throw runtime_error("Buffer has no memory allocated");
            UNLIKELY_IF (dataOffset >= bufferInterface->size_)
                throw runtime_error("Buffer offset outside of buffer size");
            UNLIKELY_IF (bufferInterface->size_ - dataOffset < requiredBufferSize)
                throw runtime_error("Buffer size to small");
        }
        UNLIKELY_IF (bufSize < requiredBufferSize)
            throw runtime_error("Buffer size (" + to_string(bufSize) + ") parameter given to this function is to small for the requested (" + to_string(requiredBufferSize) + ") transfer!");

        UNLIKELY_IF (dstOffset.x + size.x > mipmapLevelSize.x || dstOffset.y + size.y > mipmapLevelSize.y || dstOffset.z + size.z > mipmapLevelSize.z)
            throw runtime_error("Coordinates outside of texture limit");

        //now we actually do something
        threadContext->cachedBindPixelUnpackBuffer(bufferInterface ? bufferInterface->id : 0);

        const int32_t componentsTypes = memorySurfaceFormat->componentsTypes;

        if (!memorySurfaceFormat->isCompressed) {
            if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
                switch (target) {
                    case GL_TEXTURE_1D:
                        threadContextGroup->functions.glTextureSubImage1D(id, dstMipmapLevel, dstOffset.x, size.x, componentsAndArrangement, componentsTypes, offsetPointer);
                        break;
                    case GL_TEXTURE_2D:
                    case GL_TEXTURE_1D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE:
                        threadContextGroup->functions.glTextureSubImage2D(id, dstMipmapLevel, dstOffset.x, dstOffset.y, size.x, size.y, componentsAndArrangement, componentsTypes, offsetPointer);
                        break;
                    case GL_TEXTURE_3D:
                    case GL_TEXTURE_2D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                    case GL_TEXTURE_CUBE_MAP:
                    case GL_TEXTURE_CUBE_MAP_ARRAY:
                        threadContextGroup->functions.glTextureSubImage3D(id, dstMipmapLevel, dstOffset.x, dstOffset.y, dstOffset.z, size.x, size.y, size.z, componentsAndArrangement, componentsTypes, offsetPointer);
                        break;
                }
            } else if (threadContextGroup->extensions.GL_EXT_direct_state_access) {
                //does this work correctly with GL_TEXTURE_CUBE_MAP_ARRAY/GL_TEXTURE_CUBE_MAP?
                switch (target) {
                    case GL_TEXTURE_1D:
                        threadContextGroup->functions.glTextureSubImage1DEXT(id, target, dstMipmapLevel, dstOffset.x, size.x, componentsAndArrangement, componentsTypes, offsetPointer);
                        break;
                    case GL_TEXTURE_2D:
                    case GL_TEXTURE_1D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE:
                        threadContextGroup->functions.glTextureSubImage2DEXT(id, target, dstMipmapLevel, dstOffset.x, dstOffset.y, size.x, size.y, componentsAndArrangement, componentsTypes, offsetPointer);
                        break;
                    case GL_TEXTURE_3D:
                    case GL_TEXTURE_2D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                    case GL_TEXTURE_CUBE_MAP_ARRAY:
                        threadContextGroup->functions.glTextureSubImage3DEXT(id, target, dstMipmapLevel, dstOffset.x, dstOffset.y, dstOffset.z, size.x, size.y, size.z, componentsAndArrangement, componentsTypes, offsetPointer);
                        break;
                    case GL_TEXTURE_CUBE_MAP:
                        uintptr_t cubeSideBufferSize = (memorySurfaceFormat->bitsPerPixelOrBlock / 8) * size.x * size.y;
                        for (unsigned int i = z; i < z + size.z; ++i)
                            threadContextGroup->functions.glTextureSubImage2DEXT(id, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, dstMipmapLevel, dstOffset.x, dstOffset.y, size.x, size.y, componentsAndArrangement, componentsTypes, reinterpret_cast<const void*>(dataOffset + cubeSideBufferSize * i));
                        break;
                }
            } else {
                bindTemporal();
                switch (target) {
                    case GL_TEXTURE_1D:
                        threadContextGroup->functions.glTexSubImage1D(target, dstMipmapLevel, dstOffset.x, size.x, componentsAndArrangement, componentsTypes, offsetPointer);
                        break;
                    case GL_TEXTURE_2D:
                    case GL_TEXTURE_1D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE:
                        threadContextGroup->functions.glTexSubImage2D(target, dstMipmapLevel, dstOffset.x, dstOffset.y, size.x, size.y, componentsAndArrangement, componentsTypes, offsetPointer);
                        break;
                    case GL_TEXTURE_3D:
                    case GL_TEXTURE_2D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                    case GL_TEXTURE_CUBE_MAP_ARRAY:
                        threadContextGroup->functions.glTexSubImage3D(target, dstMipmapLevel, dstOffset.x, dstOffset.y, dstOffset.z, size.x, size.y, size.z, componentsAndArrangement, componentsTypes, offsetPointer);
                        break;
                    case GL_TEXTURE_CUBE_MAP: {
                        uintptr_t cubeSideBufferSize = (memorySurfaceFormat->bitsPerPixelOrBlock / 8) * size.x * size.y;
                        for (unsigned int i = z; i < z + size.z; ++i)
                            threadContextGroup->functions.glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, dstMipmapLevel, dstOffset.x, dstOffset.y, size.x, size.y, componentsAndArrangement, componentsTypes, reinterpret_cast<const void*>(dataOffset + cubeSideBufferSize * i));
                        break;
                    }
                }
            }
        } else {
            uint32_t bufSize32 = bufSize; //TODO: test for values that do not fit in 32bit?
            int32_t sizedFormat = surfaceFormat->sizedFormat;
            if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
                switch (target) {
                    case GL_TEXTURE_1D:
                        threadContextGroup->functions.glCompressedTextureSubImage1D(id, dstMipmapLevel, dstOffset.x, size.x, sizedFormat, bufSize32, offsetPointer);
                        break;
                    case GL_TEXTURE_2D:
                    case GL_TEXTURE_1D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE:
                        threadContextGroup->functions.glCompressedTextureSubImage2D(id, dstMipmapLevel, dstOffset.x, dstOffset.y, size.x, size.y, sizedFormat, bufSize32, offsetPointer);
                        break;
                    case GL_TEXTURE_3D:
                    case GL_TEXTURE_2D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                    case GL_TEXTURE_CUBE_MAP:
                    case GL_TEXTURE_CUBE_MAP_ARRAY:
                        threadContextGroup->functions.glCompressedTextureSubImage3D(id, dstMipmapLevel, dstOffset.x, dstOffset.y, dstOffset.z, size.x, size.y, size.z, sizedFormat, bufSize32, offsetPointer);
                        break;
                }
            } else if (threadContextGroup->extensions.GL_EXT_direct_state_access) {
                //TODO:how to handle GL_TEXTURE_CUBE_MAP/GL_TEXTURE_CUBE_MAP_ARRAY here
                switch (target) {
                    case GL_TEXTURE_1D:
                        threadContextGroup->functions.glCompressedTextureSubImage1DEXT(id, target, dstMipmapLevel, dstOffset.x, size.x, sizedFormat, bufSize32, offsetPointer);
                        break;
                    case GL_TEXTURE_2D:
                    case GL_TEXTURE_1D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE:
                        threadContextGroup->functions.glCompressedTextureSubImage2DEXT(id, target, dstMipmapLevel, dstOffset.x, dstOffset.y, size.x, size.y, sizedFormat, bufSize32, offsetPointer);
                        break;
                    case GL_TEXTURE_3D:
                    case GL_TEXTURE_2D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                    case GL_TEXTURE_CUBE_MAP:
                    case GL_TEXTURE_CUBE_MAP_ARRAY:
                        threadContextGroup->functions.glCompressedTextureSubImage3DEXT(id, target, dstMipmapLevel, dstOffset.x, dstOffset.y, dstOffset.z, size.x, size.y, size.z, sizedFormat, bufSize32, offsetPointer);
                        break;
                }
            } else {
                bindTemporal();
                switch (target) {
                    case GL_TEXTURE_1D:
                        threadContextGroup->functions.glCompressedTexSubImage1D(target, dstMipmapLevel, dstOffset.x, size.x, sizedFormat, bufSize32, offsetPointer);
                        break;
                    case GL_TEXTURE_2D:
                    case GL_TEXTURE_1D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE:
                        threadContextGroup->functions.glCompressedTexSubImage2D(target, dstMipmapLevel, dstOffset.x, dstOffset.y, size.x, size.y, sizedFormat, bufSize32, offsetPointer);
                        break;
                    case GL_TEXTURE_3D:
                    case GL_TEXTURE_2D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                        threadContextGroup->functions.glCompressedTexSubImage3D(target, dstMipmapLevel, dstOffset.x, dstOffset.y, dstOffset.z, size.x, size.y, size.z, sizedFormat, bufSize32, offsetPointer);
                        break;
                    case GL_TEXTURE_CUBE_MAP: {
                        uintptr_t cubeMapSideSize = surfaceFormat->bitsPerPixelOrBlock * 8 * align(mipmapLevelSize.x, blockSizeX) * align(mipmapLevelSize.y, blockSizeY);
                        for (unsigned i = z; i < z + size.z; ++i)
                            threadContextGroup->functions.glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, dstMipmapLevel, x, y, size.x, size.y, sizedFormat, uint32_t(cubeMapSideSize), reinterpret_cast<const void*>(dataOffset + cubeMapSideSize * i));
                        break;
                    }
                    case GL_TEXTURE_CUBE_MAP_ARRAY:
                        threadContextGroup->functions.glCompressedTexSubImage3D(target, dstMipmapLevel, dstOffset.x, dstOffset.y, dstOffset.z, size.x, size.y, size.z, sizedFormat, bufSize32, offsetPointer);
                        break;
                }
            }
        }
    }

    /**
        LIMITATIONS:
        For copying sub regions of compressed formats you need GL_ARB_get_texture_sub_image (Core in 4.5?) or GL_ARB_texture_view (Core in 4.2?).<br>
        Otherwise x, y, z must be 0 and xSize, ySize, zSize must be the size of the texture.<br>
        One special case is GL_TEXTURE_CUBE_MAP, where you always can freely select z and zSize.<br>
        <br>

        Reading to system ram is always blocking and causes sync, so we NEVER want to use that! For that reason we only make a buffer+offset function for reading!

        TODO: the main reason why reading from a multi sample texture makes no sense is that the pattern is not known to the application?!

        @param buffer 0 or a pointer to glCompact::Buffer object
        @param offsetPointer if buffer is set to 0, then this is a pointer to memory. Otherwise this is a offset into the buffer memory.
            NOTE: ALWAYS USE BUFFER OBJECTS. USING UNMANAGED MEMORY POINTERS CAUSES A SYNC POINT AND DESTROYS PERFROMANCE. ONLY USE FOR TESTING/DEBUGING/PROTOTYPING!
        @param bufSize
        @param memorySurfaceFormat
        @param dstMipmapLevel
        @param dstOffset
        @param size
    */
    void TextureInterface::copyTo(
        BufferInterface*    bufferInterface,
        void*               offsetPointer,
        uintptr_t           bufSize,
        MemorySurfaceFormat memorySurfaceFormat,
        uint32_t            dstMipmapLevel,
        glm::ivec3          dstOffset,
        glm::ivec3          size
    ) {
        using namespace std;
        checkSurfaceFormatCompatibleToMemorySurfaceFormat(surfaceFormat, memorySurfaceFormat);
        const GLenum componentsAndArrangement = memorySurfaceFormat->componentsAndArrangement;

        uintptr_t dataOffset = reinterpret_cast<uintptr_t>(offsetPointer);

        UNLIKELY_IF (dstMipmapLevel > this->mipmapCount)
            throw runtime_error("Mipmap level outside of texture limit");
        glm::ivec3 mipmapLevelSize = getMipmapLevelSize(dstMipmapLevel);

        //TODO negative numbers possible to invert image?
        if (size.x < 1 || size.y < 1 || size.z < 1) return;
        //UNLIKELY_IF (!id)
        //    throw runtime_error("TextureInterface object is empty");
        UNLIKELY_IF (    this->target == GL_TEXTURE_2D_MULTISAMPLE
                     ||  this->target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY)
            throw runtime_error("Can not read texels from multi sample texture");


        if (bufferInterface) {
            UNLIKELY_IF (!bufferInterface->id)
                throw runtime_error("Buffer has no allocated memory");
            UNLIKELY_IF (dataOffset >= bufferInterface->size_)
                throw runtime_error("Buffer offset outside of buffer size");
        } else {
            //TODO maybe disallow this?
            //threadContext->feedbackHandler(FEEDBACK_TYPE_WARNING, "Directly reading texture data to unmanaged memory causes a sync point! EXTREME PERFROMANCE PENALTY!");
        }

        const int blockSizeX = surfaceFormat->blockSizeX;
        const int blockSizeY = surfaceFormat->blockSizeY;
        if (surfaceFormat->isCompressed) {
            UNLIKELY_IF (dstOffset.x % blockSizeX || dstOffset.y % blockSizeY || size.x % blockSizeX || size.y % blockSizeY)
                throw runtime_error("For compressed textures dstOffset.xy(" + to_string(dstOffset.x) + ", " + to_string(dstOffset.y) + ") and size.xy(" +
                to_string(size.x) + ", " + to_string(size.y) + ") must aligned with the block size(" + to_string(blockSizeX) + ", " + to_string(blockSizeY) + ")");
        }
        uintptr_t requiredBufferSize;
        if (memorySurfaceFormat->isCompressed) {
            requiredBufferSize = (memorySurfaceFormat->bitsPerPixelOrBlock / 8) * (std::max(blockSizeX, size.x) / blockSizeX) * (std::max(blockSizeY, size.y) / blockSizeY) * std::max(1, size.z);
        } else {
            requiredBufferSize = (memorySurfaceFormat->bitsPerPixelOrBlock / 8) * std::max(1, size.x) * std::max(1, size.y) * std::max(1, size.z);
        }

        bool fullTextureRead;
        if (target == GL_TEXTURE_CUBE_MAP)
            fullTextureRead = dstOffset.x == 0 && dstOffset.y == 0 &&                     size.x == mipmapLevelSize.x && size.y == mipmapLevelSize.y;
        else
            fullTextureRead = dstOffset.x == 0 && dstOffset.y == 0 && dstOffset.z == 0 && size.x == mipmapLevelSize.x && size.y == mipmapLevelSize.y && size.z == mipmapLevelSize.z;

        bool fullTextureReadCubeMapLayersSelected = fullTextureRead && (target == GL_TEXTURE_CUBE_MAP && (dstOffset.z != 0 && size.z != 6));

        //now we actually do stuff...
        threadContext->cachedBindPixelPackBuffer(bufferInterface ? bufferInterface->id : 0);

        GLenum componentsTypes = memorySurfaceFormat->componentsTypes;
        uint32_t bufSize32 = bufSize; //TODO: test for values that do not fit in 32bit?

        if (!memorySurfaceFormat->isCompressed) {
            if (threadContextGroup->extensions.GL_ARB_get_texture_sub_image) {
                threadContextGroup->functions.glGetTextureSubImage(id, dstMipmapLevel, dstOffset.x, dstOffset.y, dstOffset.z, size.x, size.y, size.z, componentsAndArrangement, componentsTypes, bufSize32, offsetPointer);
            //} else if (threadContextGroup->extensions.GL_ARB_texture_view) {
            // TODO
            } else {
                //if this read is on the full size of the texture we use glGetTextureImage, otherwise we fall back to use a fbo to read sub sections
                if (fullTextureRead) {
                    if (threadContextGroup->extensions.GL_ARB_direct_state_access && !fullTextureReadCubeMapLayersSelected) {
                        threadContextGroup->functions.glGetTextureImage(id, dstMipmapLevel, componentsAndArrangement, componentsTypes, bufSize32, offsetPointer);
                    } else if (threadContextGroup->extensions.GL_EXT_direct_state_access) {
                        //TODO: does this function take GL_TEXTURE_CUBE_MAP as parameter or not? For now I guess it does not!
                        if (target == GL_TEXTURE_CUBE_MAP) {
                            /*uintptr_t cubeMapSideSize = pixelSize(componentArrangement, componentTypes, alignment) * mipmapLevelSize.x * mipmapLevelSize.y;
                            for (int i = z; i < z + size.z; ++i)
                                threadContextGroup->functions.glGetTextureImageEXT(id, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, dstMipmapLevel, componentArrangement, componentTypes, reinterpret_cast<void*>(dataOffset + cubeMapSideSize * i));*/
                        } else {
                            threadContextGroup->functions.glGetTextureImageEXT(id, target, dstMipmapLevel, componentsAndArrangement, componentsTypes, offsetPointer);
                        }
                    } else {
                        bindTemporal();
                        if (target == GL_TEXTURE_CUBE_MAP) {
                            /*uintptr_t cubeMapSideSize = pixelSize(componentArrangement, componentTypes, alignment) * mipmapLevelSize.x * mipmapLevelSize.y;
                            for (int i = z; i < z + size.z; ++i)
                                threadContextGroup->functions.glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, dstMipmapLevel, componentArrangement, componentTypes, reinterpret_cast<void*>(dataOffset + cubeMapSideSize * i));*/
                        } else {
                            threadContextGroup->functions.glGetTexImage(target, dstMipmapLevel, componentsAndArrangement, componentsTypes, offsetPointer);
                        }
                    }
                } else {
                    //fbo(texture0) -> buffer
                    //TODO: do FBOs may have size limits smaller then supported texture sizes?
                    //TODO: can glReadPixels only read colours? A: No: componentArrangement decides if it reads colour, depth, stencil or depth-stencil
                    //TODO: test how terrible the performance of this is :B

                    GLuint fboId = threadContext->frameBufferIdForSubImageRead;

                    //BUG: In MESA if a incomplete fbo is bound to the GL_READ_FRAMEBUFFER,
                    //commands cause "FBO incomplete: no attachments [-1]" even when they have nothing to do with it.
                    //workaround: we only bind the fbo for the duration of usage and then rebind the last one again.
                    threadContextGroup->functions.glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
                    /*if (threadContext->current_frameBuffer_readId != fboId) {
                        threadContextGroup->functions.glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
                        threadContext->current_frameBuffer_readId = fboId;
                    }*/

                    if (this->z > 1) {
                        /*uintptr_t sizeOfLayer = pixelSize(componentArrangement, componentTypes, alignment) * mipmapLevelSize.x * mipmapLevelSize.y;
                        LOOPI(size.z) {
                            threadContextGroup->functions.glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, surfaceFormat->getAttachmentType(), id, dstMipmapLevel, dstOffset.z + i); //Core in 3.0 or 3.2? ARB_geometry_shader4?
                            threadContextGroup->functions.glReadPixels(dstOffset.x, dstOffset.y, size.x, size.y, componentArrangement, componentTypes, reinterpret_cast<GLvoid*>(dataOffset + sizeOfLayer * i));
                        }*/
                    } else {
                        threadContextGroup->functions.glFramebufferTexture(GL_READ_FRAMEBUFFER, surfaceFormat->attachmentType, id, dstMipmapLevel);
                        threadContextGroup->functions.glReadPixels(dstOffset.x, dstOffset.y, size.x, size.y, componentsAndArrangement, componentsTypes, offsetPointer);
                    }
                    //for now we always detach, so we don't end up with a FBO that has several attachments (colour, depth, stencil) and may be invalid
                    threadContextGroup->functions.glFramebufferTexture(GL_READ_FRAMEBUFFER, surfaceFormat->attachmentType, 0, 0);

                    threadContextGroup->functions.glBindFramebuffer(GL_READ_FRAMEBUFFER, threadContext->current_frame_readId);
                }
            }
        } else {
            if (threadContextGroup->extensions.GL_ARB_get_texture_sub_image) {
                threadContextGroup->functions.glGetCompressedTextureSubImage(this->id, dstMipmapLevel, dstOffset.x, dstOffset.y, dstOffset.z, size.x, size.y, size.z, bufSize32, offsetPointer);
            //} else if (threadContextGroup->extensions.GL_ARB_texture_view) {
            // TODO
            } else if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
                threadContextGroup->functions.glGetCompressedTextureImage(id, dstMipmapLevel, bufSize32, offsetPointer);
            } else if (threadContextGroup->extensions.GL_EXT_direct_state_access) {
                //TODO: does this function take GL_TEXTURE_CUBE_MAP as parameter?
                threadContextGroup->functions.glGetCompressedTextureImageEXT(id, target, dstMipmapLevel, offsetPointer);
            } else {
                bindTemporal();
                if (target == GL_TEXTURE_CUBE_MAP) {
                    uintptr_t cubeMapSideSize = surfaceFormat->bitsPerPixelOrBlock * 8 * align(mipmapLevelSize.x, blockSizeX) * align(mipmapLevelSize.y, blockSizeY);
                    LOOPI(6) threadContextGroup->functions.glGetCompressedTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, dstMipmapLevel, reinterpret_cast<void*>(dataOffset + cubeMapSideSize * i));
                } else {
                    threadContextGroup->functions.glGetCompressedTexImage(target, dstMipmapLevel, offsetPointer);
                }
            }
        }
    }

    /*
        Confines what mipmap level is the lowest (largest) used when sampeling from this texture. (GLSL textureSize function also will return size of mipmap[GL_TEXTURE_BASE_LEVEL])
        This also is the mipmap level that will be used by generateMipmaps() to generate all the higher (smaller) mipmap levels.
        The default value for all textures is 0.

        One use case for this kind of sampler limitation is texture streaming. Where small versions of textures are loaded and used first.
        Until later when the fully sized textures get loaded.
    */
    void TextureInterface::setMipmapBaseLevel(
        uint32_t level
    ) {
        UNLIKELY_IF (!id)
            throw runtime_error("Can't set mipmap base level of empty texture object!");
        UNLIKELY_IF (level >= mipmapCount)
            throw runtime_error("Can't set mipmap base level bayond mipmap count (" + to_string(mipmapCount) + ") of texture!");

        if (mipmapBaseLevel != level) {
            mipmapBaseLevel = level;
            setTextureParameter(GL_TEXTURE_BASE_LEVEL, int32_t(level));
        }
    }

    /*
        This function uses the mipmap mipmapBaseLevel (default = 0) to generate and fill all higher (smaller) mipmaps.

        Function does NOT work with compressed formats. Trying to do so will cause an exception!

        The standard does not mention anything about memory barriers in connection with the commands that generate mipmaps. E.g. if image store is used and then glGenerateMipmap() called.
        The default behaviour of drivers seems to be that they take care of all needed memory barriers. (https://www.khronos.org/bugzilla/show_bug.cgi?id=1429)
    */
    void TextureInterface::generateMipmaps() {
        UNLIKELY_IF (!id)
            throw runtime_error("Can't generate mipmaps for empty texture object!");
        UNLIKELY_IF (mipmapCount <= 1)
            throw runtime_error("Can't generate mipmaps for texture that only has base mipmap level 0!");
        UNLIKELY_IF (surfaceFormat->isCompressed)
            throw runtime_error("Invalid usage of generateMipmaps(), can't be used on compressed texture format!");
        if (threadContextGroup->extensions.GL_ARB_direct_state_access)
            threadContextGroup->functions.glGenerateTextureMipmap(id);
        else if (threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glGenerateTextureMipmapEXT(id, target);
        else {
            bindTemporal();
            threadContextGroup->functions.glGenerateMipmap(target);
        }
    }

    void TextureInterface::invalidate() {
        if (!id) return;
        if (!threadContextGroup->extensions.GL_ARB_invalidate_subdata) return;
        LOOPI(mipmapCount) invalidate(i);
    }

    void TextureInterface::invalidate(
        uint32_t mipmapLevel
    ) {
        if (!id) return;
        if (!threadContextGroup->extensions.GL_ARB_invalidate_subdata) return;
        threadContextGroup->functions.glInvalidateTexImage(id, mipmapLevel);
    }

    void TextureInterface::invalidate(
        uint32_t   mipmapLevel,
        glm::ivec3 offset,
        glm::ivec3 size
    ) {
        if (!id) return;
        if (threadContextGroup->extensions.GL_ARB_invalidate_subdata) {
            threadContextGroup->functions.glInvalidateTexSubImage(id, mipmapLevel, offset.x, offset.y, offset.z, size.x, size.y, size.z);
        }
    }


    /*
        GL_ARB_clear_texture (Core since 4.4)
            void glClearTexImage   (uint32_t textureId, int32_t level, GLenum format, GLenum type, const void* data);
            void glClearTexSubImage(uint32_t textureId, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, uint32_t width, uint32_t height, uint32_t depth, GLenum format, GLenum type, const void * data);

        Does not work with compressed textures! (would just throw errors)

        if a buffer is bound to GL_PIXEL_UNPACK_BUFFER, then the data pointer is offset into that buffer!

        If texture is a multisample texture, all the samples in a texel are cleared to the value

        TODO:
            - use base type depending on texture format -> can components and componentsTypes be 0 if data is 0?
            - check if memorySurfaceFormat fits texture format

        For renderable formats fbo(texture)->clear could be used. But still no decent way for sample only textures, and on some implementations even an fbo clear is probably a performance trap.
    */

    void TextureInterface::clearCheck(
        uint32_t mipmapLevel
    ) {
        UNLIKELY_IF (!threadContextGroup->extensions.GL_ARB_clear_texture)
            throw runtime_error("Can't clear texture! Missing extension GL_ARB_clear_texture (Core since 4.4)");
        UNLIKELY_IF (!id)
            throw runtime_error("Can't clear empty texture object!");
        UNLIKELY_IF (mipmapLevel >= mipmapCount)
            throw runtime_error("Can't clear texture mipmap level that doesn't exist!");
        UNLIKELY_IF (surfaceFormat->isCompressed)
            throw runtime_error("Can't clear texture that is using a compressed format!");
    }

    void TextureInterface::clear(
        uint32_t mipmapLevel
    ) {
        clearCheck(mipmapLevel);

        threadContext->cachedBindPixelUnpackBuffer(0);
        threadContextGroup->functions.glClearTexImage(this->id, mipmapLevel, 0, 0, 0);
    }

    void TextureInterface::clear(
        uint32_t               mipmapLevel,
        MemorySurfaceFormat    memorySurfaceFormat,
        const BufferInterface* buffer,
        const void*            ptr
    ) {
        clearCheck(mipmapLevel);

        threadContext->cachedBindPixelUnpackBuffer(buffer ? buffer->id : 0);
        threadContextGroup->functions.glClearTexImage(this->id, mipmapLevel, memorySurfaceFormat->componentsAndArrangement, memorySurfaceFormat->componentsTypes, ptr);
    }

    void TextureInterface::clear(
        uint32_t   mipmapLevel,
        glm::ivec3 offset,
        glm::ivec3 size
    ) {
        clearCheck(mipmapLevel);

        threadContext->cachedBindPixelUnpackBuffer(0);
        threadContextGroup->functions.glClearTexSubImage(this->id, mipmapLevel, offset.x, offset.y, offset.z, size.x, size.y, size.z, 0, 0, 0);
    }

    void TextureInterface::clear(
        uint32_t               mipmapLevel,
        glm::ivec3             offset,
        glm::ivec3             size,
        MemorySurfaceFormat    memorySurfaceFormat,
        const BufferInterface* buffer,
        const void*            ptr
    ) {
        clearCheck(mipmapLevel);

        threadContext->cachedBindPixelUnpackBuffer(buffer ? buffer->id : 0);
        threadContextGroup->functions.glClearTexSubImage(this->id, mipmapLevel, offset.x, offset.y, offset.z, size.x, size.y, size.z, memorySurfaceFormat->componentsAndArrangement, memorySurfaceFormat->componentsTypes, ptr);
    }

    /*
        int mipmapLevelCount = int(ceil(log(max(xSize, ySize, zSize) + 1)));
        if (mipmapLevel >= mipmapLevelCount) return 0;

        int mipmapLevelX = xSize;
        int mipmapLevelY = ySize;
        int mipmapLevelZ = zSize;
        LOOPI (mipmapLevelCount) {
            mipmapLevelX = max(1, mipmapLevelX / 2);
            mipmapLevelY = max(1, mipmapLevelY / 2);
            mipmapLevelZ = max(1, mipmapLevelZ / 2);
        }
    */

    /**
        Texture parameters that can be changed, with their default value after texture creation:

        GL_TEXTURE_BASE_LEVEL =     0
        GL_TEXTURE_MAX_LEVEL  =  1000
        TEXTURE_MIN_LOD       = -1000
        TEXTURE_MAX_LOD       =  1000
        GL_TEXTURE_MIN_FILTER = GL_NEAREST_MIPMAP_LINEAR (except for rectangle textures where it is GL_LINEAR, but we don't use them)
        GL_TEXTURE_MAG_FILTER = GL_LINEAR
        GL_TEXTURE_WRAP_S     = GL_REPEAT (except for rectangle textures where it is GL_CLAMP_TO_EDGE, but we don't use them)
        GL_TEXTURE_WRAP_T     = GL_REPEAT (except for rectangle textures where it is GL_CLAMP_TO_EDGE, but we don't use them)
        GL_TEXTURE_WRAP_R     = GL_REPEAT (except for rectangle textures where it is GL_CLAMP_TO_EDGE, but we don't use them)

        If texture storage is used the object constant GL_TEXTURE_IMMUTABLE_LEVELS will be the limit to what GL_TEXTURE_MAX_LEVEL can be set.
    */
    void TextureInterface::setTextureParameter(
        int32_t pname,
        int32_t param
    ) {
        UNLIKELY_IF (!id)
            throw runtime_error("Can't set texture parameter of empty texture object!");
        if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
            threadContextGroup->functions.glTextureParameteri(id, pname, param);
        } else if (threadContextGroup->extensions.GL_EXT_direct_state_access) {
            threadContextGroup->functions.glTextureParameteriEXT(id, target, pname, param);
        } else {
            bindTemporal();
            threadContextGroup->functions.glTexParameteri(target, pname, param);
        }
    }

    void TextureInterface::setTextureParameter(
        int32_t pname,
        float   param
    ) {
        UNLIKELY_IF (!id)
            throw runtime_error("Can't set texture parameter of empty texture object!");
        if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
            threadContextGroup->functions.glTextureParameterf(id, pname, param);
        } else if (threadContextGroup->extensions.GL_EXT_direct_state_access) {
            threadContextGroup->functions.glTextureParameterfEXT(id, target, pname, param);
        } else {
            bindTemporal();
            threadContextGroup->functions.glTexParameterf(target, pname, param);
        }
    }

    void TextureInterface::checkSurfaceFormatCompatibleToMemorySurfaceFormat(
        SurfaceFormat       surfaceFormat,
        MemorySurfaceFormat memorySurfaceFormat
    ) {
        UNLIKELY_IF (memorySurfaceFormat->isCompressed && surfaceFormat->isCompressed && (memorySurfaceFormat->isCompressed != surfaceFormat->isCompressed)) {
            throw runtime_error("If MemorySurfaceFormat(" + string(memorySurfaceFormat->name) + ") is a compressed format it must be the exact same format as WurfaceFormat(" + string(surfaceFormat->name) + ")!");
        }

        if (surfaceFormat->isRgbaInteger                  && memorySurfaceFormat->isRgbaInteger)                  return;
        if (surfaceFormat->isRgbaNormalizedIntegerOrFloat && memorySurfaceFormat->isRgbaNormalizedIntegerOrFloat) return;
        if (surfaceFormat->isDepth                        && memorySurfaceFormat->isDepth)                        return;
        if (surfaceFormat->isStencil                      && memorySurfaceFormat->isStencil)                      return;

        //TODO: outputing the exact SurfaceFormat/MemorySurfaceFormat string would be nice
        throw runtime_error("SurfaceFormat(" + string(surfaceFormat->name) + ") not compatible to MemorySurfaceFormat(" + string(memorySurfaceFormat->name) + ")!");
    }
}
