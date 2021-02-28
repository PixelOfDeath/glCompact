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
#include "glCompact/BufferInterface.hpp"
#include "glCompact/Tools_.hpp"
#include "glCompact/Context_.hpp"
#include "glCompact/threadContext_.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/threadContextGroup_.hpp"
#include "glCompact/GlTools.hpp"
#include "glCompact/Frame.hpp"
#include "glCompact/Sampler.hpp"
#include "glCompact/SurfaceFormatDetail.hpp"
#include "glCompact/MemorySurfaceFormatDetail.hpp"
#include "glCompact/minimumMaximum.hpp"

#include <glm/glm.hpp>

#include <stdexcept>
#include <algorithm> //for min/max in msvc
#include <string>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    TextureInterface::TextureInterface(
        const TextureInterface& textureInterface
    ) {
        create(textureInterface.target, textureInterface.surfaceFormat, textureInterface.size, textureInterface.mipmapCount, 0);
        if (threadContextGroup_->extensions.GL_ARB_copy_image) {
            LOOPI(textureInterface.mipmapCount)
                copyFromSurfaceMemory    (textureInterface, i, {0, 0, 0}, i, {0, 0, 0}, getMipmapLevelSize(i));
        } else {
            LOOPI(textureInterface.mipmapCount)
                copyFromSurfaceComponents(textureInterface, i, {0, 0, 0}, i, {0, 0, 0}, getMipmapLevelSize(i));
        }
        setMipmapBaseLevel(textureInterface.mipmapBaseLevel);
    }

    TextureInterface::TextureInterface(
        TextureInterface&& textureInterface
    ) :
        SurfaceInterface(move(textureInterface))
    {
        mipmapBaseLevel = textureInterface.mipmapBaseLevel;
    }

    TextureInterface& TextureInterface::operator=(
        const TextureInterface& textureInterface
    ) {
        UNLIKELY_IF (&textureInterface == this) return *this;
        free();
        return *new(this)TextureInterface(textureInterface);
    }

    TextureInterface& TextureInterface::operator=(
        TextureInterface&& textureInterface
    ) {
        free();
        return *new(this)TextureInterface(move(textureInterface));
    }

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
        glm::uvec3    newSize,
        bool          mipmap,
        uint8_t       samples
    ) {
        const bool fixedSampleLocations = true;
        free();
        int mipmapCount = 1;
        if (mipmap) switch (target) {
            case GL_TEXTURE_2D_MULTISAMPLE:
            case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: mipmapCount = 1; break;
            case GL_TEXTURE_1D:
            case GL_TEXTURE_1D_ARRAY:             mipmapCount = int(ceil(::log2(        newSize.x                                 + 1))); break;
            case GL_TEXTURE_2D:
            case GL_TEXTURE_2D_ARRAY:
            case GL_TEXTURE_CUBE_MAP:
            case GL_TEXTURE_CUBE_MAP_ARRAY:       mipmapCount = int(ceil(::log2(maximum(newSize.x,         newSize.y)             + 1))); break;
            case GL_TEXTURE_3D:                   mipmapCount = int(ceil(::log2(maximum(newSize.x, maximum(newSize.y, newSize.z)) + 1))); break;
        }

        int mipmapLevelX = newSize.x;
        int mipmapLevelY = newSize.y;
        int mipmapLevelZ = newSize.z;

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
        if (threadContextGroup_->extensions.GL_ARB_direct_state_access && threadContextGroup_->extensions.GL_ARB_texture_storage && threadContextGroup_->extensions.GL_ARB_texture_storage_multisample) {
            threadContextGroup_->functions.glCreateTextures(target, 1, &id);
            switch (target) {
                case GL_TEXTURE_1D                  : threadContextGroup_->functions.glTextureStorage1D                   (id, mipmapCount, sizedFormat, newSize.x);                       break;
                case GL_TEXTURE_1D_ARRAY            : threadContextGroup_->functions.glTextureStorage2D                   (id, mipmapCount, sizedFormat, newSize.x, newSize.y);            break;
                case GL_TEXTURE_2D                  : threadContextGroup_->functions.glTextureStorage2D                   (id, mipmapCount, sizedFormat, newSize.x, newSize.y);            break;
                case GL_TEXTURE_2D_ARRAY            : threadContextGroup_->functions.glTextureStorage3D                   (id, mipmapCount, sizedFormat, newSize.x, newSize.y, newSize.z); break;
                case GL_TEXTURE_CUBE_MAP            : threadContextGroup_->functions.glTextureStorage2D                   (id, mipmapCount, sizedFormat, newSize.x, newSize.y);            break;
                case GL_TEXTURE_CUBE_MAP_ARRAY      : threadContextGroup_->functions.glTextureStorage3D                   (id, mipmapCount, sizedFormat, newSize.x, newSize.y, newSize.z); break;
                case GL_TEXTURE_3D                  : threadContextGroup_->functions.glTextureStorage3D                   (id, mipmapCount, sizedFormat, newSize.x, newSize.y, newSize.z); break;

                case GL_TEXTURE_2D_MULTISAMPLE      : threadContextGroup_->functions.glTextureStorage2DMultisample        (id, samples,     sizedFormat, newSize.x, newSize.y,            fixedSampleLocations); break;
                case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: threadContextGroup_->functions.glTextureStorage3DMultisample        (id, samples,     sizedFormat, newSize.x, newSize.y, newSize.z, fixedSampleLocations); break;
            }
            usingTexStorage = true;
        } else {
            //TODO: do compressed formats need different function calls here? For example glCompressedTexImage2D?
            //compressed formats work fine here in win7+catalyst and r600 on 5770-Juniper.
            //Seems this functions only exist for creation + upload at the same time. But we completely separates creation and upload, so we don't have any use for them right now.

            threadContextGroup_->functions.glGenTextures(1, &id);
            this->target = target;
            bindTemporalFirstTime();

            if (!isMultiSample && threadContextGroup_->extensions.GL_ARB_texture_storage) {
                switch (target) {
                    case GL_TEXTURE_1D                  : threadContextGroup_->functions.glTexStorage1D           (target, mipmapCount, sizedFormat, newSize.x);                       break;
                    case GL_TEXTURE_1D_ARRAY            : threadContextGroup_->functions.glTexStorage2D           (target, mipmapCount, sizedFormat, newSize.x, newSize.y);            break;
                    case GL_TEXTURE_2D                  : threadContextGroup_->functions.glTexStorage2D           (target, mipmapCount, sizedFormat, newSize.x, newSize.y);            break;
                    case GL_TEXTURE_2D_ARRAY            : threadContextGroup_->functions.glTexStorage3D           (target, mipmapCount, sizedFormat, newSize.x, newSize.y, newSize.z); break;
                    case GL_TEXTURE_CUBE_MAP            : threadContextGroup_->functions.glTexStorage2D           (target, mipmapCount, sizedFormat, newSize.x, newSize.y);            break;
                    case GL_TEXTURE_CUBE_MAP_ARRAY      : threadContextGroup_->functions.glTexStorage3D           (target, mipmapCount, sizedFormat, newSize.x, newSize.y, newSize.z); break;
                    case GL_TEXTURE_3D                  : threadContextGroup_->functions.glTexStorage3D           (target, mipmapCount, sizedFormat, newSize.x, newSize.y, newSize.z); break;
                }
                usingTexStorage = true;
            } else if (isMultiSample && threadContextGroup_->extensions.GL_ARB_texture_storage_multisample) {
                switch (target) {
                    case GL_TEXTURE_2D_MULTISAMPLE      : threadContextGroup_->functions.glTexStorage2DMultisample(target, samples, sizedFormat, newSize.x, newSize.y,            fixedSampleLocations); break;
                    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: threadContextGroup_->functions.glTexStorage3DMultisample(target, samples, sizedFormat, newSize.x, newSize.y, newSize.z, fixedSampleLocations); break;
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
                            threadContextGroup_->functions.glTexImage1D(target, i, sizedFormat, mipmapLevelX, border, components, componentsTypes, 0);
                            mipmapLevelX = maximum(1, mipmapLevelX / 2);
                        }
                        break;
                    case GL_TEXTURE_1D_ARRAY:
                        LOOPI(mipmapCount) {
                            //TODO test layers parameter
                            threadContextGroup_->functions.glTexImage2D(target, i, sizedFormat, mipmapLevelX, newSize.y, border, components, componentsTypes, 0);
                            mipmapLevelX = maximum(1, mipmapLevelX / 2);
                        }
                        break;
                    case GL_TEXTURE_2D:
                        LOOPI(mipmapCount) {
                            threadContextGroup_->functions.glTexImage2D(target, i, sizedFormat, mipmapLevelX, mipmapLevelY, border, components, componentsTypes, 0);
                            mipmapLevelX = maximum(1, mipmapLevelX / 2);
                            mipmapLevelY = maximum(1, mipmapLevelY / 2);
                        }
                        break;
                    case GL_TEXTURE_2D_MULTISAMPLE:
                        threadContextGroup_->functions.glTexImage2DMultisample(target, samples, sizedFormat, newSize.x, newSize.y, fixedSampleLocations);
                        break;
                    case GL_TEXTURE_2D_ARRAY:
                        LOOPI(mipmapCount) {
                            //TODO test layers parameter
                            threadContextGroup_->functions.glTexImage3D(target, i, sizedFormat, mipmapLevelX, mipmapLevelY, newSize.z, border, components, componentsTypes, 0);
                            mipmapLevelX = maximum(1, mipmapLevelX / 2);
                            mipmapLevelY = maximum(1, mipmapLevelY / 2);
                        }
                        break;
                    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                        threadContextGroup_->functions.glTexImage3DMultisample(target, samples, sizedFormat, newSize.x, newSize.y, newSize.z, fixedSampleLocations);
                        break;
                    //TODO test cube map and cube map array creation
                    //TODO: documentation is shit again and says nothing about the non array version...
                    case GL_TEXTURE_CUBE_MAP:
                        LOOPI(mipmapCount) {
                            LOOPJ(6)
                                threadContextGroup_->functions.glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, i, sizedFormat, mipmapLevelX, mipmapLevelY, border, components, componentsTypes, 0);
                            mipmapLevelX = maximum(1, mipmapLevelX / 2);
                            mipmapLevelY = maximum(1, mipmapLevelY / 2);
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
                            threadContextGroup_->functions.glTexImage3D(target, i, sizedFormat, mipmapLevelX, mipmapLevelY, newSize.z, border, components, componentsTypes, 0);
                            mipmapLevelX = maximum(1, mipmapLevelX / 2);
                            mipmapLevelY = maximum(1, mipmapLevelY / 2);
                        }
                        break;
                    case GL_TEXTURE_3D:
                        LOOPI(mipmapCount) {
                            threadContextGroup_->functions.glTexImage3D(target, i, sizedFormat, mipmapLevelX, mipmapLevelY, mipmapLevelZ, border, components, componentsTypes, 0);
                            mipmapLevelX = maximum(1, mipmapLevelX / 2);
                            mipmapLevelY = maximum(1, mipmapLevelY / 2);
                            mipmapLevelZ = maximum(1, mipmapLevelZ / 2);
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
        if (samples) {
        } else {
            if (!surfaceFormat->isCompressed && surfaceFormat->isRgbaInteger) {
                //Integer textures do not allow linear filtering but they can come with mipmaps.
                setTextureParameter(GL_TEXTURE_MIN_FILTER, mipmap ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
                setTextureParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            } else {
                //Set trilinear filtering as default for textures with mipmaps.
                setTextureParameter(GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
                setTextureParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
        }

        //GL_ARB_texture_storage says they changed default warping mode to GL_CLAMP_TO_EDGE
        //Old style textures use GL_REPEAT and ref 4.6 also only lists GL_REPEAT as the default value (except for rectangle textures using CLAMP_TO_EDGE, but we don't use them at all anyway)
        //setTextureParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
        //setTextureParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
        //setTextureParameter(GL_TEXTURE_WRAP_R, GL_REPEAT);

        this->mipmapCount   = mipmapCount;
        this->target        = target;
        this->size          = newSize;
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
        UNLIKELY_IF (!threadContextGroup_->extensions.GL_ARB_texture_view)
            throw runtime_error("Can't create texture view! GL_ARB_texture_view not supported!");
        UNLIKELY_IF (layerCount <= 0)
            throw runtime_error("layerCount must be a positive number!");

        //TODO: test SurfaceFormat compatibility

        target        = newTarget;
        //TODO, needs formula for mipmap
        //TODO, needs formula for mipmap (and 1d array texture!)
        size          = glm::uvec3(srcImages.size.x, srcImages.size.y, minimum(layerCount, srcImages.size.z));
        mipmapCount   = mipmap ? firstMipmap - srcImages.mipmapCount : 1;
        samples       = 0;
        surfaceFormat = newSurfaceFormat;

        //GL_ARB_direct_state_access glCreateTextures does NOT work here //tested with win10-blob and mesa (on r290)
        threadContextGroup_->functions.glGenTextures(1, &id);
        threadContextGroup_->functions.glTextureView(id, newTarget, srcImages.id, newSurfaceFormat->sizedFormat, firstMipmap, mipmapCount, firstLayer, 1);
    }

    //TODO: shall this always return 1 (what it does right now!) for the "not existing" dimensions? So it can be used in multiplications without checking?
    glm::uvec3 TextureInterface::getMipmapLevelSize(
        uint32_t mipmapLevel
    ) const {
        if (mipmapLevel >= mipmapCount) return glm::uvec3(0); //error or just return 0, 0, 0?
        return glm::max(glm::uvec3(1), glm::uvec3(
                                               size.x >> mipmapLevel,
            (target != GL_TEXTURE_1D_ARRAY) ? (size.y >> mipmapLevel) : size.y,
            (target == GL_TEXTURE_3D)       ? (size.z >> mipmapLevel) : size.z
        ));
    }

    /**
        @param mem
        @param maxCopySizeGuard
        @param memorySurfaceFormat
        @param dstMipmapLevel
        @param texOffset
        @param texSize
    */
    void TextureInterface::copyConvertFromMemory(
        const void*         mem,
        uint32_t            maxCopySizeGuard,
        MemorySurfaceFormat memorySurfaceFormat,
        uint32_t            mipmapLevel,
        glm::ivec3          texOffset,
        glm::ivec3          texSize
    ) {
        copyConvertFrom(0, mem, maxCopySizeGuard, memorySurfaceFormat, mipmapLevel, texOffset, texSize);
    }

    void TextureInterface::copyConvertToMemory(
        void*               mem,
        uint32_t            maxCopySizeGuard,
        MemorySurfaceFormat memorySurfaceFormat,
        uint32_t            mipmapLevel,
        glm::ivec3          texOffset,
        glm::ivec3          texSize
    ) const {
        copyConvertTo(0, mem, maxCopySizeGuard, memorySurfaceFormat, mipmapLevel, texOffset, texSize);
    }

    void TextureInterface::copyConvertFromBuffer(
        const BufferInterface& bufferInterface,
        uintptr_t              offset,
        uint32_t               maxCopySizeGuard,
        MemorySurfaceFormat    memorySurfaceFormat,
        uint32_t               mipmapLevel,
        glm::ivec3             texOffset,
        glm::ivec3             texSize
    ) {
        copyConvertFrom(&bufferInterface, reinterpret_cast<void*>(offset), maxCopySizeGuard, memorySurfaceFormat, mipmapLevel, texOffset, texSize);
    }

    void TextureInterface::copyConvertToBuffer(
        BufferInterface&    bufferInterface,
        uintptr_t           offset,
        uint32_t            maxCopySizeGuard,
        MemorySurfaceFormat memorySurfaceFormat,
        uint32_t            mipmapLevel,
        glm::ivec3          texOffset,
        glm::ivec3          texSize
    ) const {
        copyConvertTo(&bufferInterface, reinterpret_cast<void*>(offset), maxCopySizeGuard, memorySurfaceFormat, mipmapLevel, texOffset, texSize);
    }

    /**
        This function copies pixel data from client memory or a buffer object into a texture. It handles most cases of format conversions.
        Can generate compressed texture data from RGBA data. Or copy raw compressed data directly.

        For compressed textures x, y, z and xSize, ySize, zSize must be a factor of 4


        Internals:

            Using defined pixel data to fill any kind of texture (compressed or uncompressed)
                GL_ARB_direct_state_access
                    glTextureSubImage*D
                old style with binding
                    glTexSubImage*D

            Copying already compressed data directly to a compressed texture:
                GL_ARB_direct_state_access
                    glCompressedTextureSubImage*D
                old style with binding
                    glCompressedTexSubImage*D

            Core since eons:
                void glGetTexImage                 (GLenum target, int32_t level, GLenum format, GLenum type, void* pixels)
                void glGetCompressedTexImage       (GLenum target, int32_t level, void *pixels )

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
    void TextureInterface::copyConvertFrom(
        const BufferInterface* bufferInterface,
        const void*            offsetPointer,
        uint32_t               maxCopySizeGuard,
        MemorySurfaceFormat    memorySurfaceFormat,
        uint32_t               mipmapLevel,
        glm::ivec3             texOffset,
        glm::ivec3             texSize
    ) {
        surfaceFormat.throwIfNotCopyConvertibleToThisMemorySurfaceFormat(memorySurfaceFormat);
        const uintptr_t dataOffset = reinterpret_cast<uintptr_t>(offsetPointer);

        //validate parameters, etc...
        UNLIKELY_IF (mipmapLevel > this->mipmapCount)
            throw runtime_error("Mipmap level outside of texture limit");

        const glm::ivec3 mipmapLevelSize = getMipmapLevelSize(mipmapLevel);

        UNLIKELY_IF (!id)
            throw std::runtime_error("Trying to copy to textureInterface Object without created texture!");
        UNLIKELY_IF (texOffset.x < 0 || texOffset.y < 0 || texOffset.z < 0)
            throw runtime_error("x, y and z must be positive");

        UNLIKELY_IF (texSize.x < 1 || texSize.y < 1 || texSize.z < 1) return; //TODO: can negative texSize be used for inverting images vertically/horizontally?
        UNLIKELY_IF (texOffset.x + texSize.x > mipmapLevelSize.x || texOffset.y + texSize.y > mipmapLevelSize.y || texOffset.z + texSize.z > mipmapLevelSize.z)
            throw runtime_error("Trying to access bayond texture size");

        //TODO: also check the memory pointer for the alignment? Throw error if it is of? One some platforms just performance relevant? On others a hard crash?
        const int blockSizeX = surfaceFormat->blockSizeX;
        const int blockSizeY = surfaceFormat->blockSizeY;
        if (surfaceFormat->isCompressed) {
            UNLIKELY_IF (texOffset.x % blockSizeX || texOffset.y % blockSizeY || texSize.x % blockSizeX || texSize.y % blockSizeY)
                throw runtime_error("For compressed textures texOffset.xy(" + to_string(texOffset.x) + ", " + to_string(texOffset.y) + ") and texSize.xy(" +
                to_string(texSize.x) + ", " + to_string(texSize.y) + ") must aligned with the block size(" + to_string(blockSizeX) + ", " + to_string(blockSizeY) + ")");
        }
        const uintptr_t requiredBufferSize = memorySurfaceFormat->isCompressed
            ? memorySurfaceFormat->bytePerPixelOrBlock * (maximum(blockSizeX, texSize.x) / blockSizeX) * (maximum(blockSizeY, texSize.y) / blockSizeY) * maximum(1, texSize.z)
            : memorySurfaceFormat->bytePerPixelOrBlock * maximum(1, texSize.x) * maximum(1, texSize.y) * maximum(1, texSize.z);

        if (bufferInterface) {
            UNLIKELY_IF (bufferInterface->size == 0)
                throw runtime_error("Buffer has no memory allocated");
            UNLIKELY_IF (dataOffset >= bufferInterface->size)
                throw runtime_error("Buffer offset outside of buffer size");
            UNLIKELY_IF (bufferInterface->size - dataOffset < requiredBufferSize)
                throw runtime_error("Buffer size to small");
        }
        UNLIKELY_IF (maxCopySizeGuard < requiredBufferSize)
            throw runtime_error("maxCopySizeGuard size (" + to_string(maxCopySizeGuard) + ") parameter given to this function is to small for the requested (" + to_string(requiredBufferSize) + ") transfer size!");

        UNLIKELY_IF (texOffset.x + texSize.x > mipmapLevelSize.x || texOffset.y + texSize.y > mipmapLevelSize.y || texOffset.z + texSize.z > mipmapLevelSize.z)
            throw runtime_error("Coordinates outside of texture limit");

        //now we actually do something
        threadContext_->cachedBindPixelUnpackBuffer(bufferInterface ? bufferInterface->id : 0);

        const int32_t componentsAndArrangement = memorySurfaceFormat->componentsAndArrangement;
        const int32_t componentsTypes          = memorySurfaceFormat->componentsTypes;

        if (!memorySurfaceFormat->isCompressed) {
            if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
                switch (target) {
                    case GL_TEXTURE_1D:
                        threadContextGroup_->functions.glTextureSubImage1D(id, mipmapLevel, texOffset.x, texSize.x, componentsAndArrangement, componentsTypes, offsetPointer);
                        break;
                    case GL_TEXTURE_2D:
                    case GL_TEXTURE_1D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE:
                        threadContextGroup_->functions.glTextureSubImage2D(id, mipmapLevel, texOffset.x, texOffset.y, texSize.x, texSize.y, componentsAndArrangement, componentsTypes, offsetPointer);
                        break;
                    case GL_TEXTURE_3D:
                    case GL_TEXTURE_2D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                    case GL_TEXTURE_CUBE_MAP:
                    case GL_TEXTURE_CUBE_MAP_ARRAY:
                        threadContextGroup_->functions.glTextureSubImage3D(id, mipmapLevel, texOffset.x, texOffset.y, texOffset.z, texSize.x, texSize.y, texSize.z, componentsAndArrangement, componentsTypes, offsetPointer);
                        break;
                }
            } else {
                bindTemporal();
                switch (target) {
                    case GL_TEXTURE_1D:
                        threadContextGroup_->functions.glTexSubImage1D(target, mipmapLevel, texOffset.x, texSize.x, componentsAndArrangement, componentsTypes, offsetPointer);
                        break;
                    case GL_TEXTURE_2D:
                    case GL_TEXTURE_1D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE:
                        threadContextGroup_->functions.glTexSubImage2D(target, mipmapLevel, texOffset.x, texOffset.y, texSize.x, texSize.y, componentsAndArrangement, componentsTypes, offsetPointer);
                        break;
                    case GL_TEXTURE_3D:
                    case GL_TEXTURE_2D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                    case GL_TEXTURE_CUBE_MAP_ARRAY:
                        threadContextGroup_->functions.glTexSubImage3D(target, mipmapLevel, texOffset.x, texOffset.y, texOffset.z, texSize.x, texSize.y, texSize.z, componentsAndArrangement, componentsTypes, offsetPointer);
                        break;
                    case GL_TEXTURE_CUBE_MAP: {
                        uintptr_t cubeSideBufferSize = memorySurfaceFormat->bytePerPixelOrBlock * texSize.x * texSize.y;
                        for (unsigned int i = size.z; i < size.z + texSize.z; ++i)
                            threadContextGroup_->functions.glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mipmapLevel, texOffset.x, texOffset.y, texSize.x, texSize.y, componentsAndArrangement, componentsTypes, reinterpret_cast<const void*>(dataOffset + cubeSideBufferSize * i));
                        break;
                    }
                }
            }
        } else {
            int32_t sizedFormat = surfaceFormat->sizedFormat;
            if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
                switch (target) {
                    case GL_TEXTURE_1D:
                        threadContextGroup_->functions.glCompressedTextureSubImage1D(id, mipmapLevel, texOffset.x, texSize.x, sizedFormat, maxCopySizeGuard, offsetPointer);
                        break;
                    case GL_TEXTURE_2D:
                    case GL_TEXTURE_1D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE:
                        threadContextGroup_->functions.glCompressedTextureSubImage2D(id, mipmapLevel, texOffset.x, texOffset.y, texSize.x, texSize.y, sizedFormat, maxCopySizeGuard, offsetPointer);
                        break;
                    case GL_TEXTURE_3D:
                    case GL_TEXTURE_2D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                    case GL_TEXTURE_CUBE_MAP:
                    case GL_TEXTURE_CUBE_MAP_ARRAY:
                        threadContextGroup_->functions.glCompressedTextureSubImage3D(id, mipmapLevel, texOffset.x, texOffset.y, texOffset.z, texSize.x, texSize.y, texSize.z, sizedFormat, maxCopySizeGuard, offsetPointer);
                        break;
                }
            } else {
                bindTemporal();
                switch (target) {
                    case GL_TEXTURE_1D:
                        threadContextGroup_->functions.glCompressedTexSubImage1D(target, mipmapLevel, texOffset.x, texSize.x, sizedFormat, maxCopySizeGuard, offsetPointer);
                        break;
                    case GL_TEXTURE_2D:
                    case GL_TEXTURE_1D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE:
                        threadContextGroup_->functions.glCompressedTexSubImage2D(target, mipmapLevel, texOffset.x, texOffset.y, texSize.x, texSize.y, sizedFormat, maxCopySizeGuard, offsetPointer);
                        break;
                    case GL_TEXTURE_3D:
                    case GL_TEXTURE_2D_ARRAY:
                    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                        threadContextGroup_->functions.glCompressedTexSubImage3D(target, mipmapLevel, texOffset.x, texOffset.y, texOffset.z, texSize.x, texSize.y, texSize.z, sizedFormat, maxCopySizeGuard, offsetPointer);
                        break;
                    case GL_TEXTURE_CUBE_MAP: {
                        uintptr_t cubeMapSideSize = surfaceFormat->bitsPerPixelOrBlock * 8 * alignTo(mipmapLevelSize.x, blockSizeX) * alignTo(mipmapLevelSize.y, blockSizeY);
                        for (unsigned i = size.z; i < size.z + texSize.z; ++i)
                            threadContextGroup_->functions.glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mipmapLevel, size.x, size.y, texSize.x, texSize.y, sizedFormat, uint32_t(cubeMapSideSize), reinterpret_cast<const void*>(dataOffset + cubeMapSideSize * i));
                        break;
                    }
                    case GL_TEXTURE_CUBE_MAP_ARRAY:
                        threadContextGroup_->functions.glCompressedTexSubImage3D(target, mipmapLevel, texOffset.x, texOffset.y, texOffset.z, texSize.x, texSize.y, texSize.z, sizedFormat, maxCopySizeGuard, offsetPointer);
                        break;
                }
            }
        }
    }

    /*
        LIMITATIONS:
        For copying sub regions of textures you need GL_ARB_get_texture_sub_image (Core since 4.5)
        Otherwise x, y, z must be 0 and xSize, ySize, zSize must be the size of the texture.
        One special case is GL_TEXTURE_CUBE_MAP, where you always can freely select z and zSize.

        - With GL_ARB_texture_view (Core since 4.3) this could be implemented for other layered textures!
        - For non compressed formats an FBO to buffer copy could be used? Unsure about the possible performance impact. Also no garanties what formats/size can be bound to FBO.

        TODO: the main reason why reading from a multi sample texture makes no sense is that the pattern is not known to the application?!
    */
    /**
        @param buffer 0 or a pointer to glCompact::Buffer object
        @param offsetPointer if buffer is set to 0, then this is a pointer to memory. Otherwise this is a offset into the buffer memory.
            NOTE: ALWAYS USE BUFFER OBJECTS. USING UNMANAGED MEMORY POINTERS CAUSES A SYNC POINT AND DESTROYS PERFROMANCE. ONLY USE FOR TESTING/DEBUGING/PROTOTYPING!
        @param maxCopySizeGuard maximum size of writable memory after offset! This function will throw if the data to copy is larger!
        @param memorySurfaceFormat
        @param mipmapLevel
        @param texOffset
        @param texSize
    */
    void TextureInterface::copyConvertTo(
        BufferInterface*    bufferInterface,
        void*               offsetPointer,
        uint32_t            maxCopySizeGuard,
        MemorySurfaceFormat memorySurfaceFormat,
        uint32_t            mipmapLevel,
        glm::ivec3          texOffset,
        glm::ivec3          texSize
    ) const {
        surfaceFormat.throwIfNotCopyConvertibleToThisMemorySurfaceFormat(memorySurfaceFormat);
        const uintptr_t dataOffset = reinterpret_cast<uintptr_t>(offsetPointer);

        UNLIKELY_IF (mipmapLevel > this->mipmapCount)
            throw runtime_error("Mipmap level outside of texture limit");
        const glm::ivec3 mipmapLevelSize = getMipmapLevelSize(mipmapLevel);

        //TODO negative numbers possible to invert image?
        if (texSize.x < 1 || texSize.y < 1 || texSize.z < 1) return;
        UNLIKELY_IF (!id)
            throw runtime_error("TextureInterface object contains no texture/renderBuffer");
        UNLIKELY_IF (    this->target == GL_TEXTURE_2D_MULTISAMPLE
                     ||  this->target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY)
            throw runtime_error("Can not read texels from multi sample texture");

        if (bufferInterface) {
            UNLIKELY_IF (!bufferInterface->id)
                throw runtime_error("Buffer has no allocated memory");
            UNLIKELY_IF (dataOffset >= bufferInterface->size)
                throw runtime_error("Buffer offset outside of buffer size");
        }

        const int blockSizeX = surfaceFormat->blockSizeX;
        const int blockSizeY = surfaceFormat->blockSizeY;
        if (surfaceFormat->isCompressed) {
            UNLIKELY_IF (texOffset.x % blockSizeX || texOffset.y % blockSizeY || texSize.x % blockSizeX || texSize.y % blockSizeY)
                throw runtime_error("For compressed textures texOffset.xy(" + to_string(texOffset.x) + ", " + to_string(texOffset.y) + ") and texSize.xy(" +
                to_string(texSize.x) + ", " + to_string(texSize.y) + ") must aligned with the block size(" + to_string(blockSizeX) + ", " + to_string(blockSizeY) + ")");
        }
        const uint32_t requiredBufferSize = memorySurfaceFormat->isCompressed
            ? memorySurfaceFormat->bytePerPixelOrBlock * (maximum(blockSizeX, texSize.x) / blockSizeX) * (maximum(blockSizeY, texSize.y) / blockSizeY) * maximum(1, texSize.z)
            : memorySurfaceFormat->bytePerPixelOrBlock *  maximum(1, texSize.x) * maximum(1, texSize.y) * maximum(1, texSize.z);

        UNLIKELY_IF (maxCopySizeGuard < requiredBufferSize)
            throw runtime_error("maxCopySizeGuard size (" + to_string(maxCopySizeGuard) + ") parameter given to this function is to small for the requested (" + to_string(requiredBufferSize) + ") transfer size!");

        const bool entireXY = texOffset.x == 0 && texOffset.y == 0 && texSize.x == mipmapLevelSize.x && texSize.y == mipmapLevelSize.y;
        const bool entireZ  = texOffset.z == 0                     && texSize.z == mipmapLevelSize.z;
        const bool entireXYZ = entireXY && entireZ;

        const int32_t componentsAndArrangement = memorySurfaceFormat->componentsAndArrangement;
        const int32_t componentsTypes          = memorySurfaceFormat->componentsTypes;

        threadContext_->cachedBindPixelPackBuffer(bufferInterface ? bufferInterface->id : 0);
        if (!memorySurfaceFormat->isCompressed) {
            if (entireXYZ) {
                if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
                    threadContextGroup_->functions.glGetTextureImage(id, mipmapLevel, componentsAndArrangement, componentsTypes, maxCopySizeGuard, offsetPointer);
                } else {
                    bindTemporal();
                    threadContextGroup_->functions.glGetTexImage(target, mipmapLevel, componentsAndArrangement, componentsTypes, offsetPointer);
                }
            } else {
                if (threadContextGroup_->extensions.GL_ARB_get_texture_sub_image) {
                    threadContextGroup_->functions.glGetTextureSubImage(id, mipmapLevel, texOffset.x, texOffset.y, texOffset.z, texSize.x, texSize.y, texSize.z, componentsAndArrangement, componentsTypes, maxCopySizeGuard, offsetPointer);
                } else if (entireXY && target == GL_TEXTURE_CUBE_MAP) {
                    uint32_t cubeMapSideSize = memorySurfaceFormat->bytePerPixelOrBlock * mipmapLevelSize.x * mipmapLevelSize.y;
                    bindTemporal();
                    for (uint32_t i = size.z; i < size.z + texSize.z; ++i)
                        threadContextGroup_->functions.glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mipmapLevel, componentsAndArrangement, componentsTypes, reinterpret_cast<void*>(dataOffset + cubeMapSideSize * i));
                } else if (entireXY && threadContextGroup_->extensions.GL_ARB_texture_view) {
                    uint32_t viewTexId = 0;
                    int32_t viewTarget = target == GL_TEXTURE_CUBE_MAP_ARRAY ? GL_TEXTURE_2D_ARRAY : target; //GL_TEXTURE_CUBE_MAP_ARRAY can not have an arbitary layer selection range, GL_TEXTURE_2D_ARRAY can!
                    threadContextGroup_->functions.glGenTextures(1, &viewTexId);
                    threadContextGroup_->functions.glTextureView(viewTexId, viewTarget, id, surfaceFormat->sizedFormat, mipmapLevel, 1, texOffset.z, texSize.z);
                    if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
                        threadContextGroup_->functions.glGetTextureImage(viewTexId, mipmapLevel, componentsAndArrangement, componentsTypes, maxCopySizeGuard, offsetPointer);
                    } else {
                        threadContext_->cachedBindTexture(0, viewTarget, viewTexId);
                        threadContextGroup_->functions.glGetTexImage(viewTarget, mipmapLevel, componentsAndArrangement, componentsTypes, offsetPointer);
                        threadContext_->cachedBindTexture(0, viewTarget, 0);
                    }
                    threadContextGroup_->functions.glDeleteTextures(1, &viewTexId);
                } else {
                    throw runtime_error("Missing GL_ARB_get_texture_sub_image, can not copy sub image to memory/buffer!");

                    /*
                    //fbo(texture0) -> buffer
                    //TODO: do FBOs may have size limits smaller then supported texture sizes?
                    //TODO: can glReadPixels only read colours? A: No: componentsAndArrangement decides if it reads colour, depth, stencil or depth-stencil
                    //TODO: test how terrible the performance of this is :B

                    uint32_t fboId;
                    threadContextGroup_->functions.glGenFramebuffers(1, &fboId);
                    threadContextGroup_->functions.glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);

                    if (this->z > 1) {
                        uintptr_t sizeOfLayer = (memorySurfaceFormat->bitsPerPixelOrBlock / 8) * mipmapLevelSize.x * mipmapLevelSize.y;
                        LOOPI(texSize.z) {
                            threadContextGroup_->functions.glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, surfaceFormat->getAttachmentType(), id, mipmapLevel, texOffset.z + i); //Core in 3.0 or 3.2? ARB_geometry_shader4?
                            threadContextGroup_->functions.glReadPixels(texOffset.x, texOffset.y, texSize.x, texSize.y, componentsAndArrangement, componentsTypes, reinterpret_cast<GLvoid*>(dataOffset + sizeOfLayer * i));
                        }
                    } else {
                        threadContextGroup_->functions.glFramebufferTexture(GL_READ_FRAMEBUFFER, surfaceFormat->attachmentType, id, mipmapLevel);
                        threadContextGroup_->functions.glReadPixels(texOffset.x, texOffset.y, texSize.x, texSize.y, componentsAndArrangement, componentsTypes, offsetPointer);
                    }

                    //Some drivers have issues removing the FBO from the context binding when it gets deleted, so we do it ourself
                    threadContextGroup_->functions.glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
                    threadContext->current_frame_readId = 0;
                    threadContextGroup_->functions.glDeleteFramebuffers(1, &fboId);
                    */
                }
            }
        } else {
            if (entireXYZ) {
                if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
                    threadContextGroup_->functions.glGetCompressedTextureImage(id, mipmapLevel, maxCopySizeGuard, offsetPointer);
                } else {
                    bindTemporal();
                    threadContextGroup_->functions.glGetCompressedTexImage(target, mipmapLevel, offsetPointer);
                }
            } else {
                if (threadContextGroup_->extensions.GL_ARB_get_texture_sub_image) {
                     threadContextGroup_->functions.glGetCompressedTextureSubImage(this->id, mipmapLevel, texOffset.x, texOffset.y, texOffset.z, texSize.x, texSize.y, texSize.z, maxCopySizeGuard, offsetPointer);
                } else if (entireXY && target == GL_TEXTURE_CUBE_MAP) {
                    bindTemporal();
                    uint32_t cubeMapSideSize = (surfaceFormat->bitsPerPixelOrBlock / 8) * alignTo(mipmapLevelSize.x, blockSizeX) * alignTo(mipmapLevelSize.y, blockSizeY);
                    LOOPI(6) threadContextGroup_->functions.glGetCompressedTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mipmapLevel, reinterpret_cast<void*>(dataOffset + cubeMapSideSize * i));
                } else {
                    throw runtime_error("Missing GL_ARB_get_texture_sub_image, can not copy sub image to memory/buffer!");
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
        if (threadContextGroup_->extensions.GL_ARB_direct_state_access)
            threadContextGroup_->functions.glGenerateTextureMipmap(id);
        else {
            bindTemporal();
            threadContextGroup_->functions.glGenerateMipmap(target);
        }
    }

    void TextureInterface::invalidate() {
        if (!id) return;
        if (!threadContextGroup_->extensions.GL_ARB_invalidate_subdata) return;
        LOOPI(mipmapCount) invalidate(i);
    }

    void TextureInterface::invalidate(
        uint32_t mipmapLevel
    ) {
        if (!id) return;
        if (!threadContextGroup_->extensions.GL_ARB_invalidate_subdata) return;
        threadContextGroup_->functions.glInvalidateTexImage(id, mipmapLevel);
    }

    void TextureInterface::invalidate(
        uint32_t   mipmapLevel,
        glm::ivec3 offset,
        glm::ivec3 size
    ) {
        if (!id) return;
        if (threadContextGroup_->extensions.GL_ARB_invalidate_subdata) {
            threadContextGroup_->functions.glInvalidateTexSubImage(id, mipmapLevel, offset.x, offset.y, offset.z, size.x, size.y, size.z);
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
        UNLIKELY_IF (!threadContextGroup_->extensions.GL_ARB_clear_texture)
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

        threadContext_->cachedBindPixelUnpackBuffer(0);
        threadContextGroup_->functions.glClearTexImage(this->id, mipmapLevel, 0, 0, 0);
    }

    void TextureInterface::clear(
        uint32_t               mipmapLevel,
        MemorySurfaceFormat    memorySurfaceFormat,
        const BufferInterface* buffer,
        const void*            ptr
    ) {
        clearCheck(mipmapLevel);

        threadContext_->cachedBindPixelUnpackBuffer(buffer ? buffer->id : 0);
        threadContextGroup_->functions.glClearTexImage(this->id, mipmapLevel, memorySurfaceFormat->componentsAndArrangement, memorySurfaceFormat->componentsTypes, ptr);
    }

    void TextureInterface::clear(
        uint32_t   mipmapLevel,
        glm::ivec3 offset,
        glm::ivec3 size
    ) {
        clearCheck(mipmapLevel);

        threadContext_->cachedBindPixelUnpackBuffer(0);
        threadContextGroup_->functions.glClearTexSubImage(this->id, mipmapLevel, offset.x, offset.y, offset.z, size.x, size.y, size.z, 0, 0, 0);
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

        threadContext_->cachedBindPixelUnpackBuffer(buffer ? buffer->id : 0);
        threadContextGroup_->functions.glClearTexSubImage(this->id, mipmapLevel, offset.x, offset.y, offset.z, size.x, size.y, size.z, memorySurfaceFormat->componentsAndArrangement, memorySurfaceFormat->componentsTypes, ptr);
    }

    /*
        int mipmapLevelCount = int(ceil(log(maximum(xSize, ySize, zSize) + 1)));
        if (mipmapLevel >= mipmapLevelCount) return 0;

        int mipmapLevelX = xSize;
        int mipmapLevelY = ySize;
        int mipmapLevelZ = zSize;
        LOOPI (mipmapLevelCount) {
            mipmapLevelX = maximum(1, mipmapLevelX / 2);
            mipmapLevelY = maximum(1, mipmapLevelY / 2);
            mipmapLevelZ = maximum(1, mipmapLevelZ / 2);
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
        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            threadContextGroup_->functions.glTextureParameteri(id, pname, param);
        } else {
            bindTemporal();
            threadContextGroup_->functions.glTexParameteri(target, pname, param);
        }
    }

    void TextureInterface::setTextureParameter(
        int32_t pname,
        float   param
    ) {
        UNLIKELY_IF (!id)
            throw runtime_error("Can't set texture parameter of empty texture object!");
        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            threadContextGroup_->functions.glTextureParameterf(id, pname, param);
        } else {
            bindTemporal();
            threadContextGroup_->functions.glTexParameterf(target, pname, param);
        }
    }
}
