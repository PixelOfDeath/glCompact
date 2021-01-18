#include "glCompact/Frame.hpp"
#include "glCompact/Context_.hpp"
#include "glCompact/threadContext_.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/threadContextGroup_.hpp"
#include "glCompact/Tools_.hpp"
#include "glCompact/SurfaceInterface.hpp"
#include "glCompact/SurfaceFormatDetail.hpp"
#include "glCompact/MemorySurfaceFormatDetail.hpp"
#include "glCompact/minimumMaximum.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <algorithm>
#include <array>

using namespace std;
using namespace glCompact::gl;
using namespace glm;

/*
    This can be lower min. values without the arb extension!
        values.GL_MAX_COLOR_ATTACHMENTS (min. 4)
        values.GL_MAX_DRAW_BUFFERS (min. 4)

    For dual source blending and Multiple Draw Buffers:
        values.GL_MAX_DUAL_SOURCE_DRAW_BUFFERS (min. 1)


        glFramebufferRenderbuffer(GLenum target,  GLenum attachment,  GLenum renderbuffertarget,  GLuint renderbuffer);

    GL2.0
        GL_ARB_texture_non_power_of_two

    GL3.0
        GL_ARB_framebuffer_object

        void glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
        void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
        void glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer);
        void glFramebufferTextureLayer(GLenum target,  GLenum attachment,  GLuint texture,  GLint level,  GLint layer);

    GL3.2
        void glFramebufferTexture(GLenum target,GLenum attachment, GLuint texture, GLint level); (Is this part of an extension?)

    Core since 4.3
        ARB_framebuffer_no_attachments

    GL4.5
        void glNamedFramebufferTexture(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
        void glNamedFramebufferTextureLayer(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);

    For NON-layered fbo
        glNamedFramebufferTextureEXT(fboId, attachment, tex.getId(), level);
        glNamedFramebufferTextureLayerEXT(fboId, attachment, tex.getId(), level, layer);


    //TODO what can I set with this?
        void FramebufferParameteri( enum target, enum pname, int param );
        void NamedFramebufferParameteri( uint framebuffer, enum pname, int param );


    //for queris:
        void GetFramebufferAttachmentParameteriv( enum target, enum attachment, enum pname, int *params );
        void GetNamedFramebufferAttachmentParameteriv(uint framebuffer, enum attachment, enum pname, int *params );



        void ClearBuffer{if ui}v( enum buffer, int drawbuffer, const T *value );
        void ClearNamedFramebuffer{if ui}v( uint framebuffer, enum buffer, int drawbuffer, const T *value );
        void ClearBufferfi( enum buffer, int drawbuffer, float depth, int stencil );
        void ClearNamedFramebufferfi( uint framebuffer, enum buffer, int drawbuffer, float depth, int stencil );

    Fixed sample locations must be the same for all targets! (Only using fixed sample locations in glCompact) Note that multisample renderbuffers always have fixed sample locations.

    A fbo only can hold all non-layered or all layered render targets.

    Also limited to all sRGB or non-sRGB color targets?


    //void setSRGBconversion(bool value); //enables/disables sRGB conversation  glEnable(GL_FRAMEBUFFER_SRGB)  (default is false. In OpenGL ES it is always one)
*/

/*

    GL_MAX_COLOR_ATTACHMENTS
    GL_MAX_DRAW_BUFFERS

    GL_MAX_SAMPLES          (float)
    GL_MAX_INTEGER_SAMPLES  (int)

    GL_MAX_RENDERBUFFER_SIZE
    GL_MAX_3D_TEXTURE_SIZE
    GL_MAX_ARRAY_TEXTURE_LAYERS



    notes:
    - If a layered image is attached to one attachment, then all attachments must be layered. But each can have different counts of layers!
    - changing to different FBOs is way more expensive then using a geometry shader to switch render layers! (Per draw call, changing layer target inside the shader has bad performance (except on intel iGPU))

    In some docs there is the function glFramebufferTextureFace, it does NOT really exist! :P

    We ignore rectangle textures completely for now!


    - glInvalidateFramebuffer <- can invalidate(telling ogl the current data inside that buffers is irelevant and will be overwriten completly later) each fbo slot seperatly or all at once
    -glInvalidateSubFramebuffer


    FROM THE REFERENCE DOC
    - glRenderbufferStorage is equivalent to calling glRenderbufferStorageMultisample with the samples set to zero.

    -you only need glFramebufferTexture, FramebufferTextureLayer (Except for cubemaps where glFramebufferTexture2D is needed)?

    Theoreticly FBOs support seperate bindings of one depth and one stencil buffer, but nearly no driver supports that (they only supports targets that use combination formats!)
    So this class does NOT take seperate depth/stencil targets! Only one of both or a combination format!
 */


namespace glCompact {
    /**
        \ingroup API
        \class glCompact::Frame
        \brief Texture and render-buffer container to be used for off-screen rasterization

        \details This object is a collection of texture and/or render-buffers to be used as a target for rasterization operations by PipelineRasterization.

        It can contain:

        - None or one single depth OR stencil OR depthStencil target.

        - None or up to 8 RGBA targets.

        - Targets must be either all layered or all unlayered. Layer counts can be arbitrary mixed.

        - Targets must all have the same sample count.

        - Targets must all be either SRGB or non-SRGB

        The smallest target size defines the rasterization size of the Frame. Therefor the Frame also needs at last one target.

        With GL_ARB_framebuffer_no_attachments (Core since 4.3), it is possible to create a "virtual" Frame without any attachments.
        The size, layer count and sample count are set via parameters. It is intended to be used for rasterization with e.g. SSBO image store targets.

        Note that this object only exist in the creator context. It can not be accessed by any other OpenGL context!

        The underlaying memory of all attached textures and render-buffers is not deleted until all Frame objects targeting them are also deleted.

        GL_MAX_DUAL_SOURCE_DRAW_BUFFERS even in GL4.6 only has a lower limit of 1. Support of anything more depends on the specific hardware/driver.

        TODO:
        Can not map a draw target more then once! (intercept that one here and throw! Could be very complicated with texture views)
    */
    Frame::Frame(
        SurfaceSelector depthAndOrStencilSurface,
        SurfaceSelector rgba0,
        SurfaceSelector rgba1,
        SurfaceSelector rgba2,
        SurfaceSelector rgba3,
        SurfaceSelector rgba4,
        SurfaceSelector rgba5,
        SurfaceSelector rgba6,
        SurfaceSelector rgba7
    ) {
        const array<SurfaceSelector, config::MAX_RGBA_ATTACHMENTS> rgbaSurfaceList = {rgba0, rgba1, rgba2, rgba3, rgba4, rgba5, rgba6, rgba7};

        bool     foundSingleLayer      = false;
        bool     foundMultiLayer       = false;
        bool     foundDifferentSamples = false;
        bool     foundSRGB             = false;
        bool     foundNonSRGB          = false;
        uvec3    minSize               = uvec3(0xFFFFFFFF);
        uint32_t lastSamples           = 0xFFFFFFFF;
        int32_t  rgbaMapping[config::MAX_RGBA_ATTACHMENTS] = {}; //0 == GL_NONE
        int32_t  rgbaMappingCount      = 0;

        if (depthAndOrStencilSurface.surface) {
            UNLIKELY_IF (depthAndOrStencilSurface.surface->surfaceFormat->isCompressed)
                crash("depthAndOrStencilSurface must be a uncompressed format!");
            UNLIKELY_IF (!(depthAndOrStencilSurface.surface->surfaceFormat->isDepth || depthAndOrStencilSurface.surface->surfaceFormat->isStencil))
                crash("depthAndOrStencilSurface must have depth and/or stencil format!");

            foundSingleLayer = isSingleLayer(depthAndOrStencilSurface);
            foundMultiLayer  = isMultiLayer (depthAndOrStencilSurface);
            minSize = glm::min(minSize, depthAndOrStencilSurface.surface->getSize());
            lastSamples = depthAndOrStencilSurface.surface->samples;
        }
        LOOPI(config::MAX_RGBA_ATTACHMENTS) {
            auto surfaceSelector = rgbaSurfaceList[i];
            if (surfaceSelector.surface) {
                UNLIKELY_IF (surfaceSelector.surface->surfaceFormat->isCompressed)
                    crash("Rgba format must be a uncompressed format!");
                UNLIKELY_IF (  ! surfaceSelector.surface->surfaceFormat->isRenderable
                             ||!(surfaceSelector.surface->surfaceFormat->isRgbaNormalizedIntegerOrFloat || surfaceSelector.surface->surfaceFormat->isRgbaInteger))
                    crash(string("Not a RGBA renderable format: ") + surfaceSelector.surface->surfaceFormat->name);
                UNLIKELY_IF (i >= threadContextGroup_->values.GL_MAX_COLOR_ATTACHMENTS)
                    crash(string("Can't set RGBA attachment slot that is higher then GL_MAX_COLOR_ATTACHMENTS(") + to_string(threadContextGroup_->values.GL_MAX_COLOR_ATTACHMENTS) + ")");
                UNLIKELY_IF (rgbaTargetCount + 1 >= threadContextGroup_->values.GL_MAX_DRAW_BUFFERS)
                    crash(string("Can't use more RGBA targets then GL_MAX_DRAW_BUFFERS(") + to_string(threadContextGroup_->values.GL_MAX_DRAW_BUFFERS) + ")");
                foundSingleLayer = foundSingleLayer || isSingleLayer(surfaceSelector);
                foundMultiLayer  = foundMultiLayer  || isMultiLayer (surfaceSelector);
                foundSRGB        = foundSRGB        ||  surfaceSelector.surface->surfaceFormat->isSrgb;
                foundNonSRGB     = foundNonSRGB     || !surfaceSelector.surface->surfaceFormat->isSrgb;
                minSize = glm::min(minSize, surfaceSelector.surface->getSize());
                uint32_t sSamples = surfaceSelector.surface->samples;
                if (lastSamples != 0xFFFFFFFF && lastSamples != sSamples) foundDifferentSamples = true;
                lastSamples = sSamples;
                rgbaMapping[i] = GL_COLOR_ATTACHMENT0 + i;
                rgbaTargetCount++;
                rgbaMappingCount = i + 1;
            }
        }

        UNLIKELY_IF (foundSingleLayer && foundMultiLayer)
            crash("Can not mix single layer with multi layer surfaces!");
        UNLIKELY_IF (foundDifferentSamples)
            crash("Can not mix surfaces with different sample count!");
        UNLIKELY_IF (foundSRGB && foundNonSRGB)
            crash("Can not mix SRGB and non-SRGB formats!");
        //Check for zero surfaces! Incomplite without (except if GL_ARB_framebuffer_no_attachments (core since 4.3) is supportet)
        UNLIKELY_IF (!foundSingleLayer && !foundMultiLayer)
            crash("This constructor can not create a Frame without any attachments!");
        //TODO: Check for max surfaces!
        //LOTS OF Framebuffer Completeness RULES HERE: https://www.khronos.org/opengl/wiki/Framebuffer_Object#Framebuffer_Completeness

        size    = minSize;
        layered = foundMultiLayer;
        samples = lastSamples;
        srgb    = foundSRGB;

        viewportSize = {minSize};

        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            threadContextGroup_->functions.glCreateFramebuffers(1, &id);
        } else {
            threadContextGroup_->functions.glGenFramebuffers(1, &id);
            threadContext_->cachedBindDrawFbo(id);
        }

        if (depthAndOrStencilSurface.surface) setDepthAndOrStencilAttachment(depthAndOrStencilSurface);
        int rgbaSlot = 0;
        for (auto surfaceSelector : rgbaSurfaceList) {
            if (surfaceSelector.surface) setRgbaAttachment(surfaceSelector, rgbaSlot);
            rgbaSlot++;
        }

        if (rgbaMappingCount > 0) {
            if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
                threadContextGroup_->functions.glNamedFramebufferDrawBuffers(id, rgbaMappingCount, &rgbaMapping[0]);
            } else {
                threadContextGroup_->functions.glDrawBuffers(rgbaMappingCount, &rgbaMapping[0]);
            }
        }

        //NOTE: glCheckNamedFramebufferStatusEXT is useless, because it may not correctly return an error when the FBO is not complete!
        const GLenum fboStatus =
            threadContextGroup_->extensions.GL_ARB_direct_state_access ?
                threadContextGroup_->functions.glCheckNamedFramebufferStatus(id, GL_DRAW_FRAMEBUFFER)
            :   threadContextGroup_->functions.glCheckFramebufferStatus     (    GL_DRAW_FRAMEBUFFER);
        if (fboStatus == GL_FRAMEBUFFER_COMPLETE) return;
        free();
        switch (fboStatus) {
            case GL_FRAMEBUFFER_UNDEFINED:                     crash("Framebuffer undefined");
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         crash("Incomplete attachment");
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: crash("Missing attachment");
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        crash("Incomplete draw buffer");
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        crash("Incomplete read buffer");
            case GL_FRAMEBUFFER_UNSUPPORTED:                   crash("Framebuffer unsupported");
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        crash("Incomplete multisample");
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      crash("Incomplete layer targets");
            default:                                           crash("Unknown framebuffer status (" + to_string(int32_t(fboStatus)) + ")");
        }
    }

    /**
       Needs GL_ARB_framebuffer_no_attachments (Core since 4.3)
       @param sizeX
       @param sizeY
       @param layers  (0 = unlayered)
       @param samples (0 = not multi sampled)
    */
    Frame::Frame(
        uint32_t sizeX,
        uint32_t sizeY,
        uint32_t layers,
        uint32_t samples
    ) {
        //GL_FRAMEBUFFER_DEFAULT_SAMPLES
        //GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS (atm I guess this is a boolean values. TODO test)

        UNLIKELY_IF (!threadContextGroup_->extensions.GL_ARB_framebuffer_no_attachments)
            crash("Missing ARB_framebuffer_no_attachments");
        UNLIKELY_IF (sizeX   > uint32_t(threadContextGroup_->values.GL_MAX_FRAMEBUFFER_WIDTH))
            crash("sizeX ("   + to_string(sizeX)   + ") can't be larger then GL_MAX_FRAMEBUFFER_WIDTH ("   + to_string(threadContextGroup_->values.GL_MAX_FRAMEBUFFER_WIDTH)   + ")!");
        UNLIKELY_IF (sizeY   > uint32_t(threadContextGroup_->values.GL_MAX_FRAMEBUFFER_HEIGHT))
            crash("sizeY ("   + to_string(sizeY)   + ") can't be larger then GL_MAX_FRAMEBUFFER_HEIGHT ("  + to_string(threadContextGroup_->values.GL_MAX_FRAMEBUFFER_HEIGHT)  + ")!");
        UNLIKELY_IF (layers  > uint32_t(threadContextGroup_->values.GL_MAX_FRAMEBUFFER_LAYERS))
            crash("layers ("  + to_string(layers)  + ") can't be larger then GL_MAX_FRAMEBUFFER_LAYERS ("  + to_string(threadContextGroup_->values.GL_MAX_FRAMEBUFFER_LAYERS)  + ")!");
        UNLIKELY_IF (samples > uint32_t(threadContextGroup_->values.GL_MAX_FRAMEBUFFER_SAMPLES))
            crash("samples (" + to_string(samples) + ") can't be larger then GL_MAX_FRAMEBUFFER_SAMPLES (" + to_string(threadContextGroup_->values.GL_MAX_FRAMEBUFFER_SAMPLES) + ")!");

        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            threadContextGroup_->functions.glCreateFramebuffers(1, &id);
            threadContextGroup_->functions.glNamedFramebufferParameteri(id, GL_FRAMEBUFFER_DEFAULT_WIDTH,                  sizeX);
            threadContextGroup_->functions.glNamedFramebufferParameteri(id, GL_FRAMEBUFFER_DEFAULT_HEIGHT,                 sizeY);
            threadContextGroup_->functions.glNamedFramebufferParameteri(id, GL_FRAMEBUFFER_DEFAULT_LAYERS,                 layers);
            threadContextGroup_->functions.glNamedFramebufferParameteri(id, GL_FRAMEBUFFER_DEFAULT_SAMPLES               , samples);
            threadContextGroup_->functions.glNamedFramebufferParameteri(id, GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS, true);
        } else {
            threadContextGroup_->functions.glGenFramebuffers(1, &id);
            threadContext_->cachedBindDrawFbo(id);
            threadContextGroup_->functions.glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH,                  sizeX);
            threadContextGroup_->functions.glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT,                 sizeY);
            threadContextGroup_->functions.glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_LAYERS,                 layers);
            threadContextGroup_->functions.glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES               , samples);
            threadContextGroup_->functions.glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS, true);
        }

        size    = {sizeX, sizeY, layers};
        layered = layers != 0;
        this->samples = samples;

        viewportSize = {sizeX, sizeY};
    }

    /*Frame::Frame(const Frame& frame) {

    }

    Frame& Frame::operator=(const Frame& frame) {

    }*/

    Frame::Frame(
        Frame&& frame
    ) {
        id                          = frame.id;
        size                        = frame.size;
        samples                     = frame.samples;
        layered                     = frame.layered;
        srgb                        = frame.srgb;
        convertSrgb                 = frame.convertSrgb;
        scissorEnabled              = frame.scissorEnabled;
        viewportOffset              = frame.viewportOffset;
        viewportSize                = frame.viewportSize;
        scissorOffset               = frame.scissorOffset;
        scissorSize                 = frame.scissorSize;
        depthStencilSurfaceFormat   = frame.depthStencilSurfaceFormat;
        LOOPI(config::MAX_RGBA_ATTACHMENTS)
            rgbaSurfaceFormat[i]    = frame.rgbaSurfaceFormat[i];

        frame.id = 0;
    }

    Frame& Frame::operator=(
        Frame&& frame
    ) {
        UNLIKELY_IF (&frame == this) return *this;
        free();
        id                          = frame.id;
        size                        = frame.size;
        samples                     = frame.samples;
        layered                     = frame.layered;
        srgb                        = frame.srgb;
        convertSrgb                 = frame.convertSrgb;
        scissorEnabled              = frame.scissorEnabled;
        viewportOffset              = frame.viewportOffset;
        viewportSize                = frame.viewportSize;
        scissorOffset               = frame.scissorOffset;
        scissorSize                 = frame.scissorSize;
        depthStencilSurfaceFormat   = frame.depthStencilSurfaceFormat;
        LOOPI(config::MAX_RGBA_ATTACHMENTS)
            rgbaSurfaceFormat[i]    = frame.rgbaSurfaceFormat[i];

        frame.detachPtrFromThreadContextState();
        frame.id = 0;
        return *this;
    }

    Frame::~Frame() {
        free();
    }

    void Frame::free() {
        if (!id) return;
        detachFromThreadContextState();
        threadContextGroup_->functions.glDeleteFramebuffers(1, &id);
        setDefaultValues();
    }

    void Frame::setViewport(
        glm::uvec2 offset,
        glm::uvec2 size
    ) {
        //TODO test limits
        viewportOffset = offset;
        viewportSize   = size;
        threadContext_->stateChange.viewportScissor = true;

    }

    void Frame::setScissor(
        glm::uvec2 offset,
        glm::uvec2 size
    ) {
        //TODO test limits
        scissorOffset  = offset;
        scissorSize    = size;
        scissorEnabled = offset != glm::uvec2(0, 0) || size != glm::uvec2(this->size);
        threadContext_->stateChange.viewportScissor = true;
    }

    //If this Frame is using sRGB targets, then this will enable automatic translation between the sRGBA and the linear color space when writing to it
    //and also when blending takes destination samples from it
    void Frame::setConvertSrgb(bool enabled) {
        //UNLIKELY_IF(!srgb)
        //    throw std::runtime_error("Can only enable SRGB convert for a Frame that contains SRGB targets"); //just make this a debug warning?
        convertSrgb = enabled;
    }

    /**
        Use size of the Frame as viewport!
        This is used by default!
    */
    void Frame::setViewport() {
        viewportOffset = {0, 0};
        viewportSize   = size;
        threadContext_->stateChange.viewportScissor = true;
    }

    /**
        Disable scissor!
        This is used by default!
    */
    void Frame::setScissor() {
        scissorOffset  = {0, 0};
        scissorSize    = size;
        scissorEnabled = false;
        threadContext_->stateChange.viewportScissor = true;
    }

    void Frame::blitRgba(
        uint32_t slot
    ) {
        blitRgba(slot, {0, 0}, {0, 0}, size);
    }

    void Frame::blitRgba(
        uint32_t   slot,
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset
    ) {
        blitRgba(slot, srcOffset, dstOffset, size);
    }

    void Frame::blitRgba(
        uint32_t   slot,
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset,
        glm::ivec2 size
    ) {
        blitRgba(slot, srcOffset, dstOffset, size, size, false);
    }

    void Frame::blitRgba(
        uint32_t   slot,
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset,
        glm::ivec2 srcSize,
        glm::ivec2 dstSize,
        bool       filterLinear
    ) {
        blit(GL_COLOR_BUFFER_BIT, slot, threadContext_->pending_frame->id, srcOffset, dstOffset, srcSize, dstSize, filterLinear);
    }

    void Frame::blitDepth() {
        blitDepth({0, 0}, {0, 0}, size);
    }

    void Frame::blitDepth(
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset
    ) {
        blitDepth(srcOffset, dstOffset, size);
    }

    void Frame::blitDepth(
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset,
        glm::ivec2 size
    ) {
        blit(GL_DEPTH_BUFFER_BIT, 0, threadContext_->pending_frame->id, srcOffset, dstOffset, size, size, false);
    }

    void Frame::blitStencil(
    ) {
        blitStencil({0, 0}, {0, 0}, size);
    }

    void Frame::blitStencil(
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset
    ) {
        blitStencil(srcOffset, dstOffset, size);
    }

    void Frame::blitStencil(
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset,
        glm::ivec2 size
    ) {
        blit(GL_STENCIL_BUFFER_BIT, 0, threadContext_->pending_frame->id, srcOffset, dstOffset, size, size, false);
    }

    void Frame::blitDepthStencil() {
        blitDepthStencil({0, 0}, {0, 0}, size);
    }

    void Frame::blitDepthStencil(
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset
    ) {
        blitDepthStencil(srcOffset, dstOffset, size);
    }

    void Frame::blitDepthStencil(
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset,
        glm::ivec2 size
    ) {
        blit(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, 0, threadContext_->pending_frame->id, srcOffset, dstOffset, size, size, false);
    }

    /** BLock Image Transfer (blit)
        TODO: test for area not overlapping when src and dest Fbo are the same!
        NOTE: there is no EXT DSA blend function
        TODO: add the fbo0 targets like GL_DEPTH, GL_STENCIL, GL_COLOR? Old wrong info? In the 4.5spec is nothing about this!

        GL_SAMPLES of source and target must be identical. Or Target samples must be 0 and copy area must be not scaled!
        (with GL_EXT_framebuffer_multisample_blit_scaled also GL_SCALED_RESOLVE_FASTEST_EXT, GL_SCALED_RESOLVE_NICEST_EXT, But that seems to be a NV only extension)

            GL_COLOR_ATTACHMENT0 + threadContextGroup_->values.GL_MAX_COLOR_ATTACHMENTS

        @param mask        GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_STENCIL_BUFFER_BIT
        @param filter      GL_NEAREST, GL_LINEAR
        @param srcFrame    fboId to a Frame Buffer Object or 0 for the default framebuffer
        @param srcRgbaSlot if mask is GL_COLOR_BUFFER_BIT, this is the attachment slot number of the source framebuffer color buffer
    */
    void Frame::blit(
        GLenum     mask,
        uint32_t   srcRgbaSlot,
        GLuint     dstFboId,
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset,
        glm::ivec2 srcSize,
        glm::ivec2 dstSize,
        bool       filterLinear
    ) {
        GLuint srcFboId = this->id;
        GLenum filter = filterLinear ? GL_LINEAR : GL_NEAREST;

        //TODO: add negative values in size to invert image!
        GLint srcX0 = srcOffset.x;
        GLint srcY0 = srcOffset.y;
        GLint srcX1 = srcOffset.x + srcSize.x;
        GLint srcY1 = srcOffset.y + srcSize.y;
        GLint dstX0 = dstOffset.x;
        GLint dstY0 = dstOffset.y;
        GLint dstX1 = dstOffset.x + dstSize.x;
        GLint dstY1 = dstOffset.y + dstSize.y;

        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            if (mask == GL_COLOR_BUFFER_BIT)
                threadContextGroup_->functions.glNamedFramebufferReadBuffer(srcFboId, GL_COLOR_ATTACHMENT0 + srcRgbaSlot);
            threadContextGroup_->functions.glBlitNamedFramebuffer(srcFboId, dstFboId, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
        } else {
            threadContext_->cachedBindReadFbo(srcFboId);
            if (mask == GL_COLOR_BUFFER_BIT)
                threadContextGroup_->functions.glReadBuffer(GL_COLOR_ATTACHMENT0 + srcRgbaSlot);
            threadContext_->cachedBindDrawFbo(dstFboId);
            threadContextGroup_->functions.glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
        }
    }

    /*
        Soooo old...
            glClearColor(float r, float g, float b, float a)
            glClearDepth(float depth)
            glClearStencil(int stencil)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        Core since 3.0
            void glClearBufferiv (GLenum buffer, GLint drawBuffer, const GLint * value);
            void glClearBufferuiv(GLenum buffer, GLint drawBuffer, const GLuint * value);
            void glClearBufferfv (GLenum buffer, GLint drawBuffer, const GLfloat * value);
            void glClearBufferfi (GLenum buffer, GLint drawBuffer, GLfloat depth, GLint stencil);

        GL_ARB_direct_state_access (Core since 4.5)
            void glClearNamedFramebufferiv (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value);
            void glClearNamedFramebufferuiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value);
            void glClearNamedFramebufferfv (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value);
            void glClearNamedFramebufferfi (GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
    */

    /*
        -Clear with float values on interger formats, or integer values on float formats causes UNDEFINED BEHAVIOR!
        -Could intercept this and always use cast + clamp to get value types?
        -is this only for arb dsa or also the older functions?

        what other setting influence clearing? scissor, viewport(?),

        Can not use GL_COLOR for default frame buffer!?
    */
    void Frame::clearRgba() {
        LOOPI(config::MAX_RGBA_ATTACHMENTS)
            clearRgba(i);
    }

    void Frame::clearRgba(
        uint32_t  slot,
        glm::vec4 rgba
    ) {
        if (rgbaSurfaceFormat[slot]->isRgbaNormalizedIntegerOrFloat) {
            clearRgbaNormalizedOrFloat(slot, rgba);
        } else if (rgbaSurfaceFormat[slot]->isRgbaInteger) {
            if (rgbaSurfaceFormat[slot]->isSigned) {
                clearRgbaSigned(slot, rgba);
            } else {
                clearRgbaUnsigned(slot, rgba);
            }
        }
    }

    void Frame::clearRgba(
        uint32_t   slot,
        glm::uvec4 rgba
    ) {
        if (rgbaSurfaceFormat[slot]->isRgbaNormalizedIntegerOrFloat) {
            clearRgbaNormalizedOrFloat(slot, rgba);
        } else if (rgbaSurfaceFormat[slot]->isRgbaInteger) {
            if (rgbaSurfaceFormat[slot]->isSigned) {
                clearRgbaSigned(slot, rgba);
            } else {
                clearRgbaUnsigned(slot, rgba);
            }
        }
    }

    void Frame::clearRgba(
        uint32_t   slot,
        glm::ivec4 rgba
    ) {
        if (rgbaSurfaceFormat[slot]->isRgbaNormalizedIntegerOrFloat) {
            clearRgbaNormalizedOrFloat(slot, rgba);
        } else if (rgbaSurfaceFormat[slot]->isRgbaInteger) {
            if (rgbaSurfaceFormat[slot]->isSigned) {
                clearRgbaSigned(slot, rgba);
            } else {
                clearRgbaUnsigned(slot, rgba);
            }
        }
    }

    void Frame::clearRgbaNormalizedOrFloat(
        uint32_t  slot,
        glm::vec4 rgba
    ) {
        threadContext_->processPendingChangesDrawFrame(this);

      //if (threadContext->extensions.GL_ARB_direct_state_access) {
      //    threadContextGroup_->functions.glClearNamedFramebufferfv(id, GL_COLOR, slot, &rgba[0]);
      //} else {
      //    if (threadContext->version.equalOrGreater(3, 0)) {
                threadContextGroup_->functions.glClearBufferfv(GL_COLOR, slot, &rgba[0]);
      //    } else {
      //        threadContextGroup_->functions.glDrawBuffer(GL_COLOR_ATTACHMENT0 + slot);
      //        threadContextGroup_->functions.glClearColor(rgba.r, rgba.g, rgba.b, rgba.a);
      //        threadContextGroup_->functions.glClear(GL_COLOR_BUFFER_BIT);
      //    }
      //}
    }

    void Frame::clearRgbaUnsigned(
        uint32_t   slot,
        glm::uvec4 rgba
    ) {
        threadContext_->processPendingChangesDrawFrame(this);
      //if (threadContext->extensions.GL_ARB_direct_state_access) {
      //    threadContextGroup_->functions.glClearNamedFramebufferuiv(id, GL_COLOR, slot, &rgba[0]);
      //} else {
            threadContextGroup_->functions.glClearBufferuiv(GL_COLOR, slot, &rgba[0]);
      //}
    }

    void Frame::clearRgbaSigned(
        uint32_t   slot,
        glm::ivec4 rgba
    ) {
        threadContext_->processPendingChangesDrawFrame(this);
      //if (threadContext->extensions.GL_ARB_direct_state_access) {
      //    threadContextGroup_->functions.glClearNamedFramebufferiv(id, GL_COLOR, slot, &rgba[0]);
      //} else {
            threadContextGroup_->functions.glClearBufferiv(GL_COLOR, slot, &rgba[0]);
      //}
    }

    /**
        NOTE: default clear depth value in OpenGL is 1.0f!
    */
    void Frame::clearDepth(
        float depth
    ) {
        threadContext_->processPendingChangesDrawFrame(this);

      //if (threadContext->extensions.GL_ARB_direct_state_access) {
      //    threadContextGroup_->functions.glClearNamedFramebufferfv(id, GL_DEPTH, 0, &depth);
      //} else {
            threadContextGroup_->functions.glClearBufferfv(GL_DEPTH, 0, &depth);
      //}
    }

    void Frame::clearStencil(
        uint32_t stencil
    ) {
        threadContext_->processPendingChangesDrawFrame(this);

      //if (threadContext->extensions.GL_ARB_direct_state_access) {
      //    threadContextGroup_->functions.glClearNamedFramebufferuiv(id, GL_STENCIL, 0, &stencil);
      //} else {
            threadContextGroup_->functions.glClearBufferuiv(GL_STENCIL, 0, &stencil);
      //}
    }

    /**
        NOTE: default clear depth value in OpenGL is 1.0f!
    */
    void Frame::clearDepthStencil(
        float    depth,
        uint32_t stencil
    ) {
        threadContext_->processPendingChangesDrawFrame(this);

      //if (threadContext->extensions.GL_ARB_direct_state_access) {
      //    threadContextGroup_->functions.glClearNamedFramebufferfi(id, GL_DEPTH_STENCIL, 0, depth, stencil);
      //} else {
            threadContextGroup_->functions.glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil);
      //}
    }

    /**
        works with
        -core gl4.5
        -GL_ARB_invalidate_subdata (Core in 4.3)
        -GL_EXT_discard_framebuffer
        otherwise this function will do nothing

        TODO: on some platforms clear could be used if no invalidate is supported?!

        This function is most usefull to call on all the targets that dont need to be preserved before unbinding the fbo
        So the driver knows he does not have to unpack render data
        This also helps when under memory pressure, the driver can just free the memory of the texture/renderbuffer until future use

        @param attachment
            GL_DEPTH_ATTACHMENT
            GL_STENCIL_ATTACHMENT
            GL_DEPTH_STENCIL_ATTACHMENT
            GL_COLOR_ATTACHMENTi (i = 0..values.GL_MAX_COLOR_ATTACHMENTS-1)

    */
    void Frame::invalidate(
        GLenum attachment
    ) {
        if (!id) return;
        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            threadContextGroup_->functions.glInvalidateNamedFramebufferData(id, 1, &attachment);
        } else if (threadContextGroup_->extensions.GL_ARB_invalidate_subdata) {
            threadContext_->cachedBindDrawFbo(id);
            threadContextGroup_->functions.glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, &attachment);
        } else if (threadContextGroup_->extensions.GL_EXT_discard_framebuffer) {
            threadContext_->cachedBindDrawFbo(id);
            threadContextGroup_->functions.glDiscardFramebufferEXT(GL_DRAW_FRAMEBUFFER, 1, &attachment);
        }
    }

    void Frame::invalidateRgba() {
        LOOPI(threadContextGroup_->values.GL_MAX_COLOR_ATTACHMENTS) invalidateRgba(i);
    }

    void Frame::invalidateRgba(
        uint32_t slot
    ) {
        if (slot >= uint32_t(threadContextGroup_->values.GL_MAX_COLOR_ATTACHMENTS)) return;
        invalidate(GL_COLOR_ATTACHMENT0 + slot);
    }

    void Frame::invalidateDepth() {
        invalidate(GL_DEPTH_ATTACHMENT);
    }

    void Frame::invalidateStencil() {
        invalidate(GL_STENCIL_ATTACHMENT);
    }

    void Frame::invalidateDepthStencil() {
        invalidate(GL_DEPTH_STENCIL_ATTACHMENT);
    }

    void Frame::invalidate() {
        invalidateDepthStencil();
        invalidateRgba();
    }

    /*
        Core since wood exist:
            void glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type,                  GLvoid* data);
        Core since 3.0
            void glClampColor(GLenum target​, GLenum clamp​);
        Core since 4.5
            void glReadnPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void*   data);
    */
    void Frame::copyToMemory(
        uint32_t            rgbaSlot,
        void*               mem,
        uintptr_t           bufSize,
        MemorySurfaceFormat memorySurfaceFormat,
        glm::ivec2          offset,
        glm::ivec2          size
    ) {
        if (memorySurfaceFormat->isRgbaNormalizedIntegerOrFloat || memorySurfaceFormat->isRgbaInteger) {
            UNLIKELY_IF (rgbaSlot >= config::MAX_RGBA_ATTACHMENTS)
                throw std::runtime_error("Trying to select rgbaSlot(" + to_string(rgbaSlot) + ") beyond config::MAX_RGBA_ATTACHMENTS(0.." + to_string(config::MAX_RGBA_ATTACHMENTS - 1) + ")");
        }

        GLenum format = memorySurfaceFormat->componentsAndArrangement;
        GLenum type   = memorySurfaceFormat->componentsTypes;

        threadContext_->cachedBindReadFbo(id);
        threadContext_->cachedBindPixelPackBuffer(0);
        //only needed for RGBA copy; TODO: test if this works for the windows frame buffer on older GL implementations, or if I need to use GL_FRONT/GL_BACK
        //Not sure how much SDL2 intercepts in terms of default frame buffer, probably have to test with glfw
        threadContextGroup_->functions.glReadBuffer(GL_COLOR_ATTACHMENT0 + rgbaSlot);
        threadContextGroup_->functions.glReadPixels(offset.x, offset.y, size.x, size.y, format, type, mem);
    }

    bool Frame::isSingleLayer(
        SurfaceSelector sel
    ) {
        return sel.surface && (sel.layer != -1 || !sel.surface->isLayered());
    }

    bool Frame::isMultiLayer(
        SurfaceSelector sel
    ) {
        return sel.surface && (sel.layer == -1 &&  sel.surface->isLayered());
    }

    /**
        @param attachmentType
            GL_DEPTH_ATTACHMENT
            GL_STENCIL_ATTACHMENT
            GL_DEPTH_STENCIL_ATTACHMENT
            GL_COLOR_ATTACHMENTi (i = 0..values.GL_MAX_COLOR_ATTACHMENTS-1)
    */
    void Frame::setAttachment(
        SurfaceSelector sel,
        GLenum          attachmentType
    ) {
        bool   isTexture        = sel.surface->target != GL_RENDERBUFFER;
        bool   isLayerSelection = sel.layer != -1;
        int    layer            = sel.layer;
        GLuint surfaceId        = sel.surface->id;
        int    mipmapLevel      = sel.mipmapLevel;

        if (threadContextGroup_->extensions.GL_ARB_direct_state_access) {
            if (isTexture) {
                if (isLayerSelection)
                    threadContextGroup_->functions.glNamedFramebufferTextureLayer(id, attachmentType, surfaceId, mipmapLevel, layer);
                else
                    threadContextGroup_->functions.glNamedFramebufferTexture(id, attachmentType, surfaceId, mipmapLevel);
            } else
                threadContextGroup_->functions.glNamedFramebufferRenderbuffer(id, attachmentType, GL_RENDERBUFFER, surfaceId);
        } else {
            threadContext_->cachedBindDrawFbo(id);
            if (isTexture) {
                if (isLayerSelection)
                    threadContextGroup_->functions.glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, attachmentType, surfaceId, mipmapLevel, layer);
                else
                    threadContextGroup_->functions.glFramebufferTexture(GL_DRAW_FRAMEBUFFER, attachmentType, surfaceId, mipmapLevel);
            } else
                threadContextGroup_->functions.glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, surfaceId);
        }
    }

    /*
        Maybe will use this if I ever do <3.3 support

        if (isLayerSelection) {
            if (threadContext->version.equalOrGreater(3, 0)) {
                threadContext->glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, attachment, images.id, mipmapLevel, layer);
            } else {
                GLenum textureTarget;
                int    textureLayer;
                if (images.target == GL_TEXTURE_CUBE_MAP || images.target == GL_TEXTURE_CUBE_MAP_ARRAY) {
                    textureTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + (layer % 6);
                    textureLayer  = layer / 6;
                } else {
                    textureTarget = images.target;
                    textureLayer  = layer;
                }
                threadContext->glFramebufferTexture3D(GL_DRAW_FRAMEBUFFER, attachment, textureTarget, images.id, mipmapLevel, textureLayer);
            }
        } else {
            if (threadContext->version.equalOrGreater(3, 2)) {
                threadContext->glFramebufferTexture(GL_DRAW_FRAMEBUFFER, attachmentType, surfaceId, mipmapLevel);
            } else {
                //glFramebufferTexture1D/2D/3D are Core since 3.0
                GLenum target = images ? images->target : GL_TEXTURE_1D;
                switch (target) {
                    case GL_TEXTURE_1D:
                        threadContext->glFramebufferTexture1D(GL_DRAW_FRAMEBUFFER, attachmentType, GL_TEXTURE_1D, surfaceId, mipmapLevel);
                        break;
                    case GL_TEXTURE_2D:
                    case GL_TEXTURE_2D_MULTISAMPLE:
                    //case GL_TEXTURE_RECTANGLE:
                    case GL_TEXTURE_CUBE_MAP:
                        threadContext->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachmentType, target, surfaceId, mipmapLevel);
                        break;
                    case GL_TEXTURE_3D:
                        //  threadContext->glFramebufferTexture3D();
                        break;
                }
            }
        }
    */

    void Frame::setDepthAndOrStencilAttachment(
        SurfaceSelector sel
    ) {
        setAttachment(sel, sel.surface->surfaceFormat->attachmentType);
        depthStencilSurfaceFormat = sel.surface->getSurfaceFormat();
    }

    void Frame::setRgbaAttachment(
        SurfaceSelector sel,
        uint32_t        rgbaSlot
    ) {
        setAttachment(sel, GL_COLOR_ATTACHMENT0 + rgbaSlot);
        rgbaSurfaceFormat[rgbaSlot] = sel.surface->getSurfaceFormat();
    }

    void Frame::detachPtrFromThreadContextState() const {
        assert(threadContext_);
        if (threadContext_->current_frame        == this) threadContext_->current_frame        = nullptr;
        if (threadContext_->pending_frame        == this) threadContext_->pending_frame        = nullptr;
    }

    void Frame::detachFromThreadContextState() const {
        detachPtrFromThreadContextState();
        const int setCurrentValue = config::Workarounds::AMD_DELETING_ACTIVE_FBO_NOT_SETTING_DEFAULT_FBO ? -1 : 0;
        if (threadContext_->current_frame_drawId ==   id) threadContext_->current_frame_drawId = setCurrentValue;
        if (threadContext_->current_frame_readId ==   id) threadContext_->current_frame_readId = setCurrentValue;
        if (threadContext_->pending_frame_drawId ==   id) threadContext_->pending_frame_drawId = setCurrentValue;
    }

    /*void Frame::setReadTarget(GLenum target) {
        //if (!id && !defaultFrameBuffer) return;
        if (!id) return;
        if (threadContext->extensions.GL_ARB_direct_state_access) {
            threadContext->glNamedFramebufferReadBuffer(id, target);
        } else {
            threadContext->cachedBindReadFbo();
            threadContext->glReadBuffer(target);
        }
    }*/

    void Frame::setDefaultValues() {
        new (this) Frame();
    }
}
