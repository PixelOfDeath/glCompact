#include "glCompact/ToolsInternal.hpp"
#include "glCompact/SurfaceInterface.hpp"
#include "glCompact/Context.hpp"
#include "glCompact/ThreadContext.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/SurfaceFormatDetail.hpp"

#include <glm/glm.hpp>

#include <stdexcept>

/*
    Limitations for size are:
        for RenderBuffers:
            GL_MAX_SAMPLES                for depth/stencil/rgba normalized or float
            GL_MAX_INTEGER_SAMPLES        only for rgba integer formats

        textures:
            GL_MAX_TEXTURE_SIZE           max size for all except 3d texture
            GL_MAX_3D_TEXTURE_SIZE        max size for 3d texture

            GL_MAX_COLOR_TEXTURE_SAMPLES
            GL_MAX_INTEGER_SAMPLES
            GL_MAX_DEPTH_TEXTURE_SAMPLES

            GL_MAX_ARRAY_TEXTURE_LAYERS   may layers for 1d/2d array textures

        FBO without attachments:
            GL_MAX_FRAMEBUFFER_SAMPLES (min. 4)
*/

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    SurfaceInterface& SurfaceInterface::operator=(
        SurfaceInterface&& surfaceInterface
    ) {
        x             = surfaceInterface.x;
        y             = surfaceInterface.y;
        z             = surfaceInterface.z;
        mipmapCount   = surfaceInterface.mipmapCount;
        samples       = surfaceInterface.samples;
        surfaceFormat = surfaceInterface.surfaceFormat;
        id            = surfaceInterface.id;
        target        = surfaceInterface.target;
        surfaceInterface.id = 0;
        return *this;
    }

    void SurfaceInterface::detachFromThreadContext() {
        if (!id) return;
        //TODO
        if (threadContext) {
            LOOPI(threadContext->texture_getHighestNonNull()) {
                if (threadContext->texture_id[i] == id) {
                    threadContext->texture_id[i] = 0;
                }
            }
            LOOPI(threadContext->image_getHighestNonNull()) {
                if (threadContext->image_id[i] == id) {
                    threadContext->image_id[i] = 0;
                    //TODO: set the rest values to NULL, too?
                }
            }
        }
    }

    void SurfaceInterface::free() {
        if (!id) return;
        detachFromThreadContext();
        //If this texture/renderBuffer is part of the currently bound FBO then it would be unbound by OpenGL.
        //To prevent this we always set the default FBO. Still being bound to any FBO will act like a hard link (like a texture view) to the underlaying surface memory.
        //This gives a consistend behavior for FBOs and also prevents them from being changed and possibly made incomplete!
        threadContext->cachedBindDrawFbo(0);
        threadContext->cachedBindReadFbo(0); //TODO read fbo relevant? I guess yes.
        if (target == GL_RENDERBUFFER) {
            threadContextGroup_->functions.glDeleteRenderbuffers(1, &id);
        } else {
            threadContextGroup_->functions.glDeleteTextures(1, &id);
        }
        id            = 0;
        target        = 0;
        x             = 0;
        y             = 0;
        z             = 0;
        mipmapCount   = 0;
        samples       = 0;
        surfaceFormat = static_cast<SurfaceFormat::FormatEnum>(0);
    }

    /**
        Depends on GL_ARB_copy_image (Core since 4.3) or GL_EXT_copy_image (Not part of Core); (Maybe we can include GL_NV_copy_image, too. But it does not support copys between compressed and uncompressed formats!)

        GL_ARB_copy_image and GL_EXT_copy_image work with textures and renderBuffers (TODO: is that the case for the NV extension, too?)

        This is a function to copy raw data between surface objects (textures and/or renderbuffers). It only can copy between formats where the sample count is
        the same and where the pixel byte size (in case of compressed textures the block byte size) is the same!
    */
    void SurfaceInterface::copyFromSurfaceMemory(
        const SurfaceInterface& srcSurface,
        uint32_t                srcMipmapLevel,
        glm::ivec3              srcOffset,
        uint32_t                dstMipmapLevel,
        glm::ivec3              dstOffset,
        glm::ivec3              size
    ) {
        if (!this->id) return;
        if (!srcSurface.id) return;

        //TODO: test for size limits
        //TODO: test for format compatibility
        //TODO: do GL_EXT_copy_image/GL_NV_copy_image have the same limits then GL_ARB_copy_image?

        if (threadContextGroup_->extensions.GL_ARB_copy_image) {
            threadContextGroup_->functions.glCopyImageSubData(srcSurface.id, srcSurface.target, srcMipmapLevel, srcOffset.x, srcOffset.y, srcOffset.z, this->id, this->target, dstMipmapLevel, dstOffset.x, dstOffset.y, dstOffset.z, size.x, size.y, size.z);
        } else if (threadContextGroup_->extensions.GL_EXT_copy_image) {
            threadContextGroup_->functions.glCopyImageSubDataEXT(srcSurface.id, srcSurface.target, srcMipmapLevel, srcOffset.x, srcOffset.y, srcOffset.z, this->id, this->target, dstMipmapLevel, dstOffset.x, dstOffset.y, dstOffset.z, size.x, size.y, size.z);
        //} else if (threadContextGroup_->extensions.GL_NV_copy_image) {
        //    threadContext->glCopyImageSubDataNV(srcSurface.id, srcSurface.target, srcMipmapLevel, srcOffset.x, srcOffset.y, srcOffset.z, this->id, this->target, dstMipmapLevel, dstOffset.x, dstOffset.y, dstOffset.z, size.x, size.y, size.z);
        } else {
            throw runtime_error("Non GL_ARB_copy_image/GL_EXT_copy_image path not implemented");
            /*FrameBuffer srcFb;
            srcFb.create();
            srcFb.attach(GLenum attachment, id, sourceMipmapLevel);
            FrameBuffer dstFb;*/

            //An FBO can't be used to copy from compressed formats, also not for raw memory copy.
            //But glGetTexImage could be used for a roundtrip of
            //texture -> buffer -> texture
            //FBO(renderBuffer) -> FBO(renderBuffer)
            //FBO(renderBuffer) -> buffer -> compressed texture
        }
    }

    /**
        (Core since 1.2)
            glCopyTexSubImage1D
            glCopyTexSubImage2D
            glCopyTexSubImage3D
            //we dont use this one! They only create a new texture with fbo sourced data.
            glCopyTexImage1D
            glCopyTexImage2D
            glCopyTexImage3D

        (Core since 3.0)
            glFramebufferTexture1D
            glFramebufferTexture2D
            glFramebufferTexture3D
            glFramebufferTextureLayer

        ARB_geometry_shader4 (Core since 3.2 as non-ARB)
            glProgramParameteriARB
            glFramebufferTextureARB
            glFramebufferTextureLayerARB
            glFramebufferTextureFaceARB

        (Core since 3.2)
            glProgramParameteri
            glFramebufferTexture
            glFramebufferTextureLayer
            glFramebufferTextureFace

        ARB_direct_state_access (Core since 4.5)
            glCopyTextureSubImage1D
            glCopyTextureSubImage2D
            glCopyTextureSubImage3D


        Core since 1.2
        |                   ARB_direct_state_access (Core since 4.5)
        glCopyTexSubImage1D |                           TEXTURE_1D
                            glCopyTextureSubImage1D

        glCopyTexSubImage2D                             TEXTURE_2D              (glCopyTexSubImage2D also accepts cube map face targets)
                            glCopyTextureSubImage2D     TEXTURE_1D_ARRAY
                                                        TEXTURE_RECTANGLE

        glCopyTexSubImage3D                             TEXTURE_3D
                            glCopyTextureSubImage3D     TEXTURE_2D_ARRAY        (glCopyTextureSubImage3D also supports TEXTURE_CUBE_MAP)
                                                        TEXTURE_CUBE_MAP_ARRAY



        Q: HOW to select between depth/stencil in formats that have both?
        A: In case of copying where the source or target only has one kind it of course is only the part both have.
           And in cases where both are depthStencil always copy both?

        Q: Possible to transfer from depth/stencil to color and the other way around? And how to select depth or stencil as source/target? Or limit this to raw copy?
        A: Only possible with copyFromSurfaceMemory

        GL_ARB_get_Texture_sub_image (Core since 4.5)

        texture to texture memcopy
        glReadnPixels                (Core 4.5, so its useless)

        ARB_copy_image               (Core since 4.3)
        GL_EXT_copy_image            (not present in mesa?)
        GL_NV_copy_image             (not present in mesa)
        GL_EXT_copy_texture          (very very old, not relevant?)

        GL_ARB_framebuffer_object    (Core since 3.0)



        Still not sure if I can emulate this with reasonable results

        OPTION 1:   fbo(texture0) -> texture1
            GL_ARB_framebuffer_object (Core in 3.0)
            most of this is core in gl 2.0 (Affected by glPixelStore?) (NOTE: this functions clamp to [0,1])
            glCopyTexSubImage1D
            glCopyTexSubImage2D
            glCopyTexSubImage3D
            -Can ONLY copy color formats
            -Can NOT copy from compressed textures
            - What is with snorm/int/uint formats?

        OPTION 2:   fbo(texture0) -> fbo(texture1)
            -Does not work with compressed formats
            -activating a draw fbo may fuck up performance to much
            glCopyPixels is deprecated in modern OpenGL (Well we would only use it in older GPUs, but maybe its depricated in a version where we still have no alternative!?)
            glBlitFramebuffer​ (Has to note that it behaves like a raserizer command and is influenced by states such as scissor test, etc...)

        OPTION 3:   texture0 -> buffer -> texture1
            -glGetTexImage
            -glGetCompressedTexImage
            -Problem is reading sub regions only works with ARB_get_Texture_sub_image (Core since 4.5). So copying even a single texel would force a full read of the src mipmap level. Even more terrible with array/3d textures!
                ARB_texture_view does not help much (core in 4.3)
            -Also we need create a fully sized buffer for this!


        OPTION 4:   fbo(texture0) -> buffer -> texture
            glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * data);

        Maybe useful for later:
            uintptr_t sizeOfLayer               = pixelSize(componentArrangement, componentType, alignment) * mipmapLevelSize.x * mipmapLevelSize.y;
            uintptr_t compressedCubeMapSideSize = formatEntry->compressedBlockSize * align(mipmapLevelSize.x, 4) * align(mipmapLevelSize.y, 4);

        TODO: how are depth/stencil textures are handled by ARB_copy_image? FBOs can't copy in between stencil/depth/colour targets!
        TODO: how is the difference in data converting between ARB_copy_image, PBOs and FBOs?

        TODO: we can make this non offset/size function work without ARB_get_Texture_sub_image
        but big texture copy to small texture? Still would need to read the complete big textures mipmap level :(

        TODO: if src and dest are the same texture and mipmap level, test for overlapping area and in that case cause error!

        this function can copy to compressed textures, but NOT FROM compressed textures!
        Also it will clamp to normalize [0..1] values?!?!

        blitting is limited to:
        float or normalized  only to  float or normalized
        signed integer       only to  signed integer
        unsigned integer     only to  unsigned integer

        color components     only to  color components
        depth component      only to  depth component
        stencil component    only to  stencil component



        fbo(     texture) ->          texture    via glCopyTex*SubImage*
        fbo(renderbuffer) ->          texture    via glCopyTex*SubImage*
        fbo(renderbuffer) -> fbo(renderbuffer)   via blitting
    */
    void SurfaceInterface::copyFromSurfaceComponents(
        const SurfaceInterface& srcSurface,
        uint32_t                srcMipmapLevel,
        glm::ivec3              srcOffset,
        uint32_t                dstMipmapLevel,
        glm::ivec3              dstOffset,
        glm::ivec3              size
    ) {
        //TODO: Test input parameters
        //TODO: tailor input parameters (Only in non debug mode or always? Maybe only warning?)
        UNLIKELY_IF (!this->id) return;
        UNLIKELY_IF (!srcSurface.id) return;
        UNLIKELY_IF (this->samples != srcSurface.samples)
            throw runtime_error("copyFromSurfaceComponents: Can not copy components from surface with different sampler count!");

        //glm::ivec3 srcLimits = glm::ivec3(srcSurface.x, srcSurface.y, srcSurface.z) - srcOffset;
        //glm::ivec3 dstLimits = glm::ivec3(    this->x,     this->y,     this->z) - dstOffset;
        //size = glm::min(size, glm::min(srcLimits, dstLimits));
        //if (size.x < 1 || size.y < 1 || size.z < 1) return; //debug warning here?

        UNLIKELY_IF (srcSurface.surfaceFormat->isCompressed)
            throw std::runtime_error("copyFromSurfaceComponents can not copy from compressed format!");

        GLint  srcId             = srcSurface.id;
      //bool   srcIsRenderBuffer = srcSurface.target == GL_RENDERBUFFER;
        GLenum srcAttachmentType = srcSurface.surfaceFormat->attachmentType;
        GLenum srcTarget         = srcSurface.target;

        GLenum dstTarget         = this->target;
        GLint  dstId             = this->id;

        //create and destroy both FBOs here?
        //Except for very seldom corner cases where the user copy from or to the same targets and layers repeatetly, state caching won't help here anyway!
        //GLuint srcFboId       = threadContext->frameBufferIdForSubImageRead;
        //threadContext->bindReadFbo(srcFboId);
        GLuint srcFboId;
        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            threadContextGroup_->functions.glCreateFramebuffers(1, &srcFboId); //TODO do I even need this here? I instandly bind it anyway.
        } else {
            threadContextGroup_->functions.glGenFramebuffers(1, &srcFboId);
        }
        threadContextGroup_->functions.glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFboId);

        GLuint targetFboId;

        const GLenum blitMask =
                srcSurface.surfaceFormat->isRgbaNormalizedIntegerOrFloat ? GL_COLOR_BUFFER_BIT   : 0
            |   srcSurface.surfaceFormat->isRgbaInteger                  ? GL_COLOR_BUFFER_BIT   : 0
            |   srcSurface.surfaceFormat->isDepth                        ? GL_DEPTH_BUFFER_BIT   : 0
            |   srcSurface.surfaceFormat->isStencil                      ? GL_STENCIL_BUFFER_BIT : 0;

        //Binding source as layers is useless, because there is no copy function that can select a source layer (copy anything else then layer 0)! So no glFramebufferTexture (Core since 3.2) here.
        //Array textures and cube map exist before 3.2 but glFramebufferTexture, to set layered texture as a whole, did not!
        //so to support all kind of drivers we need a loop that binds and copies for each layer

        for (int loopLayerOffset = 0; loopLayerOffset < size.z; ++loopLayerOffset) {
            //glFramebufferTexture1D/2D/3D/Layer are Core since 3.0
            //not using any DSA here? FBO has to be bound anyway.
            switch (srcTarget) {
                case GL_TEXTURE_1D: {
                    threadContextGroup_->functions.glFramebufferTexture1D(GL_READ_FRAMEBUFFER, srcAttachmentType, GL_TEXTURE_1D, srcId, srcMipmapLevel);
                    break;
                }
                //case GL_TEXTURE_RECTANGLE:
                //case GL_TEXTURE_1D_ARRAY: //no way to bind to fbo???
                case GL_TEXTURE_2D:
                case GL_TEXTURE_2D_MULTISAMPLE: {
                    threadContextGroup_->functions.glFramebufferTexture2D(GL_READ_FRAMEBUFFER, srcAttachmentType, srcTarget, srcId, srcMipmapLevel);
                    break;
                }
                case GL_TEXTURE_CUBE_MAP: {
                    GLenum srcCubeMapTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + srcOffset.z + loopLayerOffset;
                    threadContextGroup_->functions.glFramebufferTexture2D(GL_READ_FRAMEBUFFER, srcAttachmentType, srcCubeMapTarget, srcId, srcMipmapLevel);
                    break;
                }
                case GL_TEXTURE_3D: {
                    threadContextGroup_->functions.glFramebufferTexture3D(GL_READ_FRAMEBUFFER, srcAttachmentType, srcTarget, srcId, srcMipmapLevel, srcOffset.z + loopLayerOffset);
                    break;
                }
                case GL_TEXTURE_CUBE_MAP_ARRAY:
                case GL_TEXTURE_2D_ARRAY:
                case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: {
                    //this only attaches one single layer
                    //Similar to glFramebufferTexture3D. Used for 3d, 2d-array or 1d-array texture. Does not need a texture type parameter.
                    threadContextGroup_->functions.glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, srcAttachmentType, srcId, srcMipmapLevel, srcOffset.z + loopLayerOffset);
                    break;
                }
                case GL_RENDERBUFFER: { //both non-multisample and multisample
                    threadContextGroup_->functions.glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, srcAttachmentType, GL_RENDERBUFFER, srcId);
                }
            }


            //The non-dsa and dsa functions have significant differences in what target they are used for. Only use non-DSA here for now?
            switch (dstTarget) {
                case GL_TEXTURE_1D: {
                    if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
                        threadContextGroup_->functions.glCopyTextureSubImage1D(dstId, dstMipmapLevel, dstOffset.x, srcOffset.x, srcOffset.y, size.x);
                    } else {
                        bindTemporal();
                        threadContextGroup_->functions.glCopyTexSubImage1D(GL_TEXTURE_1D, dstMipmapLevel, dstOffset.x, srcOffset.x, srcOffset.y, size.x);
                    }
                    break;
                }
                //GL_TEXTURE_RECTANGLE:
                case GL_TEXTURE_1D_ARRAY:
                case GL_TEXTURE_2D: {
                    if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
                        threadContextGroup_->functions.glCopyTextureSubImage2D(dstId, dstMipmapLevel, dstOffset.x, dstOffset.y, srcOffset.x, srcOffset.y, size.x, size.y);
                    } else {
                        bindTemporal();
                        threadContextGroup_->functions.glCopyTexSubImage2D(GL_TEXTURE_2D, dstMipmapLevel, dstOffset.x, dstOffset.y, srcOffset.x, srcOffset.y, size.x, size.y);
                    }
                    break;
                }
                case GL_TEXTURE_CUBE_MAP: {
                    GLenum dstCubeMapTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + dstOffset.z + loopLayerOffset;
                    if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
                        //threadContext->glCopyTextureSubImage3D();
                    } else {
                        bindTemporal(); //TODO: different target needed here?? Have to test!
                        threadContextGroup_->functions.glCopyTexSubImage2D(dstCubeMapTarget, dstMipmapLevel, dstOffset.x, dstOffset.y, srcOffset.x, srcOffset.y, size.x, size.y);
                    }
                    break;
                }
                case GL_TEXTURE_2D_ARRAY:
                case GL_TEXTURE_3D:
                case GL_TEXTURE_CUBE_MAP_ARRAY: {
                    if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {

                    } else {
                        //NOTE: this function can only copy the first 2d layer from the fbo to any layers of a 3d or 2d array texture!
                        bindTemporal();
                        threadContextGroup_->functions.glCopyTexSubImage3D(GL_TEXTURE_3D, dstMipmapLevel, dstOffset.x, dstOffset.y, dstOffset.z + loopLayerOffset, srcOffset.x, srcOffset.y, size.x, size.y);
                    }
                    break;
                }
                case GL_TEXTURE_2D_MULTISAMPLE:
                case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: {
                    //TODO: setup raster states correctly that may influence blitting (scissor test, etc...)
                    //TODO: move once-stuff out of the loop
                    if (!targetFboId) {
                        threadContextGroup_->functions.glCreateFramebuffers(1, &targetFboId);
                        threadContextGroup_->functions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFboId);
                    }
                    //...
                    break;
                }
                case GL_RENDERBUFFER: { //this is non-multisample and multisample renderBuffer
                    if (!targetFboId) {
                        threadContextGroup_->functions.glCreateFramebuffers(1, &targetFboId);
                        threadContextGroup_->functions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFboId);
                    }
                    threadContextGroup_->functions.glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, srcAttachmentType, GL_RENDERBUFFER, srcId);
                    //not sure if I have to +1 on the size value?
                    threadContextGroup_->functions.glBlitFramebuffer(srcOffset.x, srcOffset.y, size.x, size.y, dstOffset.x, dstOffset.y, size.x, size.y, blitMask, GL_NEAREST);
                    break;
                }
            }
        }

        if (targetFboId) {
            int setCurrentDrawId = Config::Workarounds::AMD_DELETING_ACTIVE_FBO_NOT_SETTING_DEFAULT_FBO ? -1 : 0;
            threadContextGroup_->functions.glDeleteFramebuffers(1, &targetFboId);
            threadContext->current_frame_drawId = setCurrentDrawId;
        }

        threadContextGroup_->functions.glDeleteFramebuffers(1, &srcFboId);
        threadContext->current_frame_readId = -1; //TODO: Do some drivers also fuck around with setting the readFbo to 0 when deleting the current fbo? Or can I set this one to 0?
    }

    bool SurfaceInterface::isLayered() {
        switch (target) {
            case GL_TEXTURE_2D_ARRAY:
            case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
            case GL_TEXTURE_3D:
            case GL_TEXTURE_CUBE_MAP:
            case GL_TEXTURE_CUBE_MAP_ARRAY:
                return true;
        }
        return false;
    }

    /**
        This members must be set before calling this function:
            id
            target
    */
    void SurfaceInterface::bindTemporalFirstTime() const {
        threadContext->cachedBindTextureCompatibleOrFirstTime(target, id);
    }

    void SurfaceInterface::bindTemporal() const {
        threadContext->cachedBindTexture(target, id);
    }
}
