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
#include "glCompact/Frame.hpp"
#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"
#include "glCompact/PipelineRasterizationStateChangeInternal.hpp"
#include "glCompact/ToolsInternal.hpp"
#include "glCompact/SurfaceInterface.hpp"
#include "glCompact/SurfaceFormatDetail.hpp"
#include "glCompact/MemorySurfaceFormatDetail.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <algorithm>
#include <array>

using namespace glCompact::gl;
using namespace std;

/*
    TODO: Still have to find this exact spec in the reference!
        "If the textureId parameter is set to 0, then, the texture image will be detached from the FBO. If a texture object is deleted while it is still attached to a FBO,
        then, the texture image will be automatically detached from the currently bound FBO.
        However, if it is attached to multiple FBOs and deleted, then it will be detached from only the bound FBO, but will not be detached from any other un-bound FBOs."

        -> Probably always force unbind of FBO when deleting texture/renderBuffer, to keep the FBO as a hard link object with all its attached targets!

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
        \brief Container object to be used as target for off-screen rasterization

        \details This object is a collection of texture and/or render buffers to be used as a target for rasterization operations by PipelineRasterization.

        It can contain a single depth or stencil or depthStencil target. And up to 8 RGBA targets.

        - Targets must be either all layered or all unlayered. Layer counts can be arbitrary mixed.

        - Targets must all have the same sample count.

        - Targets must all be either SRGB or non-SRGB

        The smallest target size defines the rasterization size of the Frame. Therefor the Frame also needs at last one target.

        With GL_ARB_framebuffer_no_attachments (Core since 4.3), it is possible to create a "virtual" Frame without any attachments.
        The size, layer count and sample count are set via parameters. It is intended to be used for e.g. rasterization with SSBO image store.


        Note that this object can not be accessed by any other OpenGL context and only exist in the creator context!
    */
    /*
        TODO check max attachment count (throw is better then gl error!)

        TODO: I think I have to change default mapping with more then one rgba target? Default mapping only maps slot0 to 0???
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
    ) noexcept(false) {
        array<SurfaceSelector, 8> rgbaSurfaceList = {rgba0, rgba1, rgba2, rgba3, rgba4, rgba5, rgba6, rgba7};

        bool foundSingleLayer      = false;
        bool foundMultiLayer       = false;
        bool foundDifferentSamples = false;
        bool foundSRGB             = false;
        bool foundNonSRGB          = false;
        uint32_t minX = 0xFFFFFFFF;
        uint32_t minY = 0xFFFFFFFF;
        uint32_t minZ = 0xFFFFFFFF;
        uint32_t lastSamples = 0xFFFFFFFF;

        //bool isLayered;
        if (depthAndOrStencilSurface.surface) {
            UNLIKELY_IF (depthAndOrStencilSurface.surface->surfaceFormat->isCompressed)
                throw std::runtime_error("depthAndOrStencilSurface must be a uncompressed format!");
            UNLIKELY_IF (!(depthAndOrStencilSurface.surface->surfaceFormat->isDepth || depthAndOrStencilSurface.surface->surfaceFormat->isStencil))
                throw std::runtime_error("depthAndOrStencilSurface must have depth and/or stencil format!");

            foundSingleLayer = isSingleLayer(depthAndOrStencilSurface);
            foundMultiLayer  = isMultiLayer (depthAndOrStencilSurface);
            minX = min(minX, depthAndOrStencilSurface.surface->x);
            minY = min(minY, depthAndOrStencilSurface.surface->y);
            if (isMultiLayer(depthAndOrStencilSurface))
                minZ = min(minZ, depthAndOrStencilSurface.surface->z);
            lastSamples = depthAndOrStencilSurface.surface->samples;
        }
        for (auto surfaceSelector : rgbaSurfaceList) {
            if (surfaceSelector.surface) {
                UNLIKELY_IF (surfaceSelector.surface->surfaceFormat->isCompressed)
                    throw std::runtime_error("Rgba format must be a uncompressed format!");
                UNLIKELY_IF (    !surfaceSelector.surface->surfaceFormat->isRenderable
                             ||  !(surfaceSelector.surface->surfaceFormat->isRgbaNormalizedIntegerOrFloat || surfaceSelector.surface->surfaceFormat->isRgbaInteger))
                    throw std::runtime_error(string("Not a rgba renderable format: ") + surfaceSelector.surface->surfaceFormat->name);
                foundSingleLayer = foundSingleLayer || isSingleLayer(surfaceSelector);
                foundMultiLayer  = foundMultiLayer  || isMultiLayer (surfaceSelector);
                foundSRGB        = foundSRGB        ||  surfaceSelector.surface->surfaceFormat->isSrgb;
                foundNonSRGB     = foundNonSRGB     || !surfaceSelector.surface->surfaceFormat->isSrgb;
                minX = min(minX, surfaceSelector.surface->x);
                minY = min(minY, surfaceSelector.surface->y);
                if (isMultiLayer(surfaceSelector))
                    minZ = min(minZ, surfaceSelector.surface->z);
                uint32_t sSamples = surfaceSelector.surface->samples;
                if (lastSamples != 0xFFFFFFFF && lastSamples != sSamples) foundDifferentSamples = true;
                lastSamples = sSamples;
            }
        }
        UNLIKELY_IF (foundSingleLayer && foundMultiLayer)
            throw std::runtime_error("Can not mix single layer with multi layer surfaces!");
        UNLIKELY_IF (foundDifferentSamples)
            throw std::runtime_error("Can not mix surfaces with different sample count!");
        UNLIKELY_IF (foundSRGB && foundNonSRGB)
            throw std::runtime_error("Can not mix SRGB and non-SRGB formats!");
        //Check for zero surfaces! Incomplite without (except if GL_ARB_framebuffer_no_attachments (core since 4.3) is supportet)
        UNLIKELY_IF (!foundSingleLayer && !foundMultiLayer)
            throw std::runtime_error("This constructor can not create a Frame without any attachments!");
        //TODO: Check for max surfaces!
        //LOTS OF Framebuffer Completeness RULES HERE: https://www.khronos.org/opengl/wiki/Framebuffer_Object#Framebuffer_Completeness

        x       = minX;
        y       = minY;
        z       = foundMultiLayer ? 1 : minZ;
        layered = foundMultiLayer;
        samples = lastSamples;
        srgb    = foundSRGB;

        viewportSize = {x, y};

        if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
            threadContextGroup->functions.glCreateFramebuffers(1, &id);
        } else {
            threadContextGroup->functions.glGenFramebuffers(1, &id);
            threadContext->cachedBindDrawFbo(id);
        }

        if (depthAndOrStencilSurface.surface) setDepthAndOrStencilAttachment(depthAndOrStencilSurface);
        int rgbaSlot = 0;
        for (auto surfaceSelector : rgbaSurfaceList) {
            if (surfaceSelector.surface) setRgbaAttachment(surfaceSelector, rgbaSlot);
            rgbaSlot++;
        }


        //NOTE: glCheckNamedFramebufferStatusEXT is useless, because it may not correctly return an error when the FBO is not complete!
        GLenum fboStatus = 0;
        if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
            fboStatus = threadContextGroup->functions.glCheckNamedFramebufferStatus(id, GL_DRAW_FRAMEBUFFER);
        } else {
            fboStatus = threadContextGroup->functions.glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
        }
        if (fboStatus == GL_FRAMEBUFFER_COMPLETE) return;
        free();
        switch (fboStatus) {
            case GL_FRAMEBUFFER_UNDEFINED:                     throw std::runtime_error("Framebuffer undefined");
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         throw std::runtime_error("Incomplete attachment");
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: throw std::runtime_error("Missing attachment");
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        throw std::runtime_error("Incomplete draw buffer");
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        throw std::runtime_error("Incomplete read buffer");
            case GL_FRAMEBUFFER_UNSUPPORTED:                   throw std::runtime_error("Framebuffer unsupported");
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        throw std::runtime_error("Incomplete multisample");
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      throw std::runtime_error("Incomplete layer targets");
            default:                                           throw std::runtime_error("Unknown framebuffer status (" + to_string(int32_t(fboStatus)) + ")");
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

        UNLIKELY_IF (!threadContextGroup->extensions.GL_ARB_framebuffer_no_attachments)
            throw runtime_error("Missing ARB_framebuffer_no_attachments");
        UNLIKELY_IF (sizeX   > uint32_t(threadContextGroup->values.GL_MAX_FRAMEBUFFER_WIDTH))
            throw runtime_error("sizeX ("   + to_string(sizeX)   + ") can't be larger then GL_MAX_FRAMEBUFFER_WIDTH ("   + to_string(threadContextGroup->values.GL_MAX_FRAMEBUFFER_WIDTH)   + ")!");
        UNLIKELY_IF (sizeY   > uint32_t(threadContextGroup->values.GL_MAX_FRAMEBUFFER_HEIGHT))
            throw runtime_error("sizeY ("   + to_string(sizeY)   + ") can't be larger then GL_MAX_FRAMEBUFFER_HEIGHT ("  + to_string(threadContextGroup->values.GL_MAX_FRAMEBUFFER_HEIGHT)  + ")!");
        UNLIKELY_IF (layers  > uint32_t(threadContextGroup->values.GL_MAX_FRAMEBUFFER_LAYERS))
            throw runtime_error("layers ("  + to_string(layers)  + ") can't be larger then GL_MAX_FRAMEBUFFER_LAYERS ("  + to_string(threadContextGroup->values.GL_MAX_FRAMEBUFFER_LAYERS)  + ")!");
        UNLIKELY_IF (samples > uint32_t(threadContextGroup->values.GL_MAX_FRAMEBUFFER_SAMPLES))
            throw runtime_error("samples (" + to_string(samples) + ") can't be larger then GL_MAX_FRAMEBUFFER_SAMPLES (" + to_string(threadContextGroup->values.GL_MAX_FRAMEBUFFER_SAMPLES) + ")!");

        if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
            threadContextGroup->functions.glCreateFramebuffers(1, &id);
            threadContextGroup->functions.glNamedFramebufferParameteri(id, GL_FRAMEBUFFER_DEFAULT_WIDTH,                  sizeX);
            threadContextGroup->functions.glNamedFramebufferParameteri(id, GL_FRAMEBUFFER_DEFAULT_HEIGHT,                 sizeY);
            threadContextGroup->functions.glNamedFramebufferParameteri(id, GL_FRAMEBUFFER_DEFAULT_LAYERS,                 layers);
            threadContextGroup->functions.glNamedFramebufferParameteri(id, GL_FRAMEBUFFER_DEFAULT_SAMPLES               , samples);
            threadContextGroup->functions.glNamedFramebufferParameteri(id, GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS, true);
        } else {
            threadContextGroup->functions.glGenFramebuffers(1, &id);
            threadContext->cachedBindDrawFbo(id);
            threadContextGroup->functions.glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH,                  sizeX);
            threadContextGroup->functions.glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT,                 sizeY);
            threadContextGroup->functions.glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_LAYERS,                 layers);
            threadContextGroup->functions.glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES               , samples);
            threadContextGroup->functions.glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS, true);
        }

        x       = sizeX;
        y       = sizeY;
        z       = layers;
        layered = layers != 0;
        this->samples = samples;

        viewportSize = {x, y};
    }

    /*Frame::Frame(const Frame& frame) {

    }

    Frame& Frame::operator=(const Frame& frame) {

    }*/

    Frame::Frame(
        Frame&& frame
    ) {
        assert(this != &frame);
        id      = frame.id;
        x       = frame.x;
        y       = frame.y;
        z       = frame.z;
        samples = frame.samples;
        layered = frame.layered;
        LOOPI(Config::MAX_RGBA_ATTACHMENTS)
            rgbaAttachmentDataType[i] = frame.rgbaAttachmentDataType[i];
        scissorEnabled = frame.scissorEnabled;
        viewportOffset = frame.viewportOffset;
        viewportSize   = frame.viewportSize;
        scissorOffset  = frame.scissorOffset;
        scissorSize    = frame.scissorSize;

        setDefaultValues();
    }

    Frame& Frame::operator=(
        Frame&& frame
    ) {
        assert(this != &frame);
        if (!id) threadContextGroup->functions.glDeleteFramebuffers(1, &id);
        detachFromThreadContextStateCurrent();
        id      = frame.id;
        x       = frame.x;
        y       = frame.y;
        z       = frame.z;
        samples = frame.samples;
        layered = frame.layered;
        LOOPI(Config::MAX_RGBA_ATTACHMENTS)
            rgbaAttachmentDataType[i] = frame.rgbaAttachmentDataType[i];
        scissorEnabled = frame.scissorEnabled;
        viewportOffset = frame.viewportOffset;
        viewportSize   = frame.viewportSize;
        scissorOffset  = frame.scissorOffset;
        scissorSize    = frame.scissorSize;

        setDefaultValues();
        return *this;
    }

    Frame::~Frame() {
        free();
    }

    void Frame::free() {
        if (!id) return;
        detachFromThreadContextState();
        threadContextGroup->functions.glDeleteFramebuffers(1, &id);
        setDefaultValues();
    }

    void Frame::setViewport(
        glm::uvec2 offset,
        glm::uvec2 size
    ) {
        //TODO test limits
        viewportOffset = offset;
        viewportSize   = size;
        threadContext->pipelineRasterizationStateChangePending += PipelineRasterizationStateChange::viewportScissor;
    }

    void Frame::setScissor(
        glm::uvec2 offset,
        glm::uvec2 size
    ) {
        //TODO test limits
        scissorOffset  = offset;
        scissorSize    = size;
        scissorEnabled = offset != glm::uvec2(0, 0) || size != glm::uvec2(x, y);
        threadContext->pipelineRasterizationStateChangePending += PipelineRasterizationStateChange::viewportScissor;
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
        viewportSize   = {x, y};
        threadContext->pipelineRasterizationStateChangePending += PipelineRasterizationStateChange::viewportScissor;
    }

    /**
        Disable scissor!
        This is used by default!
    */
    void Frame::setScissor() {
        scissorOffset  = {0, 0};
        scissorSize    = {x, y};
        scissorEnabled = false;
        threadContext->pipelineRasterizationStateChangePending += PipelineRasterizationStateChange::viewportScissor;
    }

    void Frame::blitRgba(
        uint32_t slot
    ) {
        blitRgba(slot, {0, 0}, {0, 0}, {x, y});
    }

    void Frame::blitRgba(
        uint32_t   slot,
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset
    ) {
        blitRgba(slot, srcOffset, dstOffset, {x, y});
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
        blit(GL_COLOR_BUFFER_BIT, slot, threadContext->pending_frame->id, srcOffset, dstOffset, srcSize, dstSize, filterLinear);
    }

    void Frame::blitDepth() {
        blitDepth({0, 0}, {0, 0}, {x, y});
    }

    void Frame::blitDepth(
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset
    ) {
        blitDepth(srcOffset, dstOffset, {x, y});
    }

    void Frame::blitDepth(
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset,
        glm::ivec2 size
    ) {
        blit(GL_DEPTH_BUFFER_BIT, 0, threadContext->pending_frame->id, srcOffset, dstOffset, size, size, false);
    }

    void Frame::blitStencil(
    ) {
        blitStencil({0, 0}, {0, 0}, {x, y});
    }

    void Frame::blitStencil(
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset
    ) {
        blitStencil(srcOffset, dstOffset, {x, y});
    }

    void Frame::blitStencil(
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset,
        glm::ivec2 size
    ) {
        blit(GL_STENCIL_BUFFER_BIT, 0, threadContext->pending_frame->id, srcOffset, dstOffset, size, size, false);
    }

    void Frame::blitDepthStencil() {
        blitDepthStencil({0, 0}, {0, 0}, {x, y});
    }

    void Frame::blitDepthStencil(
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset
    ) {
        blitDepthStencil(srcOffset, dstOffset, {x, y});
    }

    void Frame::blitDepthStencil(
        glm::uvec2 srcOffset,
        glm::uvec2 dstOffset,
        glm::ivec2 size
    ) {
        blit(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, 0, threadContext->pending_frame->id, srcOffset, dstOffset, size, size, false);
    }

    /** BLock Image Transfer (blit)
        TODO: test for area not overlapping when src and dest Fbo are the same!
        NOTE: there is no EXT DSA blend function
        TODO: add the fbo0 targets like GL_DEPTH, GL_STENCIL, GL_COLOR? Old wrong info? In the 4.5spec is nothing about this!

        GL_SAMPLES of source and target must be identical. Or Target samples must be 0 and copy area must be not scaled!
        (with GL_EXT_framebuffer_multisample_blit_scaled also GL_SCALED_RESOLVE_FASTEST_EXT, GL_SCALED_RESOLVE_NICEST_EXT, But that seems to be a NV only extension)

            GL_COLOR_ATTACHMENT0 + threadContextGroup->values.GL_MAX_COLOR_ATTACHMENTS

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

        if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
            if (mask == GL_COLOR_BUFFER_BIT)
                threadContextGroup->functions.glNamedFramebufferReadBuffer(srcFboId, GL_COLOR_ATTACHMENT0 + srcRgbaSlot);
            threadContextGroup->functions.glBlitNamedFramebuffer(srcFboId, dstFboId, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
        } else {
            threadContext->cachedBindReadFbo(srcFboId);
            if (mask == GL_COLOR_BUFFER_BIT)
                threadContextGroup->functions.glReadBuffer(GL_COLOR_ATTACHMENT0 + srcRgbaSlot);
            threadContext->cachedBindDrawFbo(dstFboId);
            threadContextGroup->functions.glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
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
        LOOPI(Config::MAX_RGBA_ATTACHMENTS)
            clearRgba(i);
    }

    void Frame::clearRgba(
        uint32_t  slot,
        glm::vec4 rgba
    ) {
        switch (rgbaAttachmentDataType[slot]) {
            case AttachmentDataType::unused:
                break;
            case AttachmentDataType::normalizedOrFloat:
                clearRgbaNormalizedOrFloat(slot, rgba);
                break;
            case AttachmentDataType::unsignedInteger:
                clearRgbaUnsigned(slot, rgba);
                break;
            case AttachmentDataType::signedInteger:
                clearRgbaSigned(slot, rgba);
                break;
        }
    }

    void Frame::clearRgba(
        uint32_t   slot,
        glm::uvec4 rgba
    ) {
        switch (rgbaAttachmentDataType[slot]) {
            case AttachmentDataType::unused:
                break;
            case AttachmentDataType::normalizedOrFloat:
                clearRgbaNormalizedOrFloat(slot, rgba);
                break;
            case AttachmentDataType::unsignedInteger:
                clearRgbaUnsigned(slot, rgba);
                break;
            case AttachmentDataType::signedInteger:
                clearRgbaSigned(slot, rgba);
                break;
        }
    }

    void Frame::clearRgba(
        uint32_t   slot,
        glm::ivec4 rgba
    ) {
        switch (rgbaAttachmentDataType[slot]) {
            case AttachmentDataType::unused:
                break;
            case AttachmentDataType::normalizedOrFloat:
                clearRgbaNormalizedOrFloat(slot, rgba);
                break;
            case AttachmentDataType::unsignedInteger:
                clearRgbaUnsigned(slot, rgba);
                break;
            case AttachmentDataType::signedInteger:
                clearRgbaSigned(slot, rgba);
                break;
        }
    }

    void Frame::clearRgbaNormalizedOrFloat(
        uint32_t  slot,
        glm::vec4 rgba
    ) {
        threadContext->processPendingChangesDrawFrame(this);

      //if (threadContext->extensions.GL_ARB_direct_state_access) {
      //    threadContextGroup->functions.glClearNamedFramebufferfv(id, GL_COLOR, slot, &rgba[0]);
      //} else {
      //    if (threadContext->version.equalOrGreater(3, 0)) {
                threadContextGroup->functions.glClearBufferfv(GL_COLOR, slot, &rgba[0]);
      //    } else {
      //        threadContextGroup->functions.glDrawBuffer(GL_COLOR_ATTACHMENT0 + slot);
      //        threadContextGroup->functions.glClearColor(rgba.r, rgba.g, rgba.b, rgba.a);
      //        threadContextGroup->functions.glClear(GL_COLOR_BUFFER_BIT);
      //    }
      //}
    }

    void Frame::clearRgbaUnsigned(
        uint32_t   slot,
        glm::uvec4 rgba
    ) {
        threadContext->processPendingChangesDrawFrame(this);
      //if (threadContext->extensions.GL_ARB_direct_state_access) {
      //    threadContextGroup->functions.glClearNamedFramebufferuiv(id, GL_COLOR, slot, &rgba[0]);
      //} else {
            threadContextGroup->functions.glClearBufferuiv(GL_COLOR, slot, &rgba[0]);
      //}
    }

    void Frame::clearRgbaSigned(
        uint32_t   slot,
        glm::ivec4 rgba
    ) {
        threadContext->processPendingChangesDrawFrame(this);
      //if (threadContext->extensions.GL_ARB_direct_state_access) {
      //    threadContextGroup->functions.glClearNamedFramebufferiv(id, GL_COLOR, slot, &rgba[0]);
      //} else {
            threadContextGroup->functions.glClearBufferiv(GL_COLOR, slot, &rgba[0]);
      //}
    }

    /**
        NOTE: default clear depth value in OpenGL is 1.0f!
    */
    void Frame::clearDepth(
        float depth
    ) {
        threadContext->processPendingChangesDrawFrame(this);

      //if (threadContext->extensions.GL_ARB_direct_state_access) {
      //    threadContextGroup->functions.glClearNamedFramebufferfv(id, GL_DEPTH, 0, &depth);
      //} else {
            threadContextGroup->functions.glClearBufferfv(GL_DEPTH, 0, &depth);
      //}
    }

    void Frame::clearStencil(
        uint32_t stencil
    ) {
        threadContext->processPendingChangesDrawFrame(this);

      //if (threadContext->extensions.GL_ARB_direct_state_access) {
      //    threadContextGroup->functions.glClearNamedFramebufferuiv(id, GL_STENCIL, 0, &stencil);
      //} else {
            threadContextGroup->functions.glClearBufferuiv(GL_STENCIL, 0, &stencil);
      //}
    }

    /**
        NOTE: default clear depth value in OpenGL is 1.0f!
    */
    void Frame::clearDepthStencil(
        float    depth,
        uint32_t stencil
    ) {
        threadContext->processPendingChangesDrawFrame(this);

      //if (threadContext->extensions.GL_ARB_direct_state_access) {
      //    threadContextGroup->functions.glClearNamedFramebufferfi(id, GL_DEPTH_STENCIL, 0, depth, stencil);
      //} else {
            threadContextGroup->functions.glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil);
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
        if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
            threadContextGroup->functions.glInvalidateNamedFramebufferData(id, 1, &attachment);
        } else if (threadContextGroup->extensions.GL_ARB_invalidate_subdata) {
            threadContext->cachedBindDrawFbo(id);
            threadContextGroup->functions.glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, &attachment);
        } else if (threadContextGroup->extensions.GL_EXT_discard_framebuffer) {
            threadContext->cachedBindDrawFbo(id);
            threadContextGroup->functions.glDiscardFramebufferEXT(GL_DRAW_FRAMEBUFFER, 1, &attachment);
        }
    }

    void Frame::invalidateRgba() {
        LOOPI(threadContextGroup->values.GL_MAX_COLOR_ATTACHMENTS) invalidateRgba(i);
    }

    void Frame::invalidateRgba(
        uint32_t slot
    ) {
        if (slot >= uint32_t(threadContextGroup->values.GL_MAX_COLOR_ATTACHMENTS)) return;
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
            UNLIKELY_IF (rgbaSlot >= Config::MAX_RGBA_ATTACHMENTS)
                throw std::runtime_error("Trying to select rgbaSlot(" + to_string(rgbaSlot) + ") beyond Config::MAX_RGBA_ATTACHMENTS(0.." + to_string(Config::MAX_RGBA_ATTACHMENTS - 1) + ")");
        }

        if (memorySurfaceFormat->isRgbaNormalizedIntegerOrFloat) {
            UNLIKELY_IF (rgbaAttachmentDataType[rgbaSlot] != AttachmentDataType::normalizedOrFloat)
                throw std::runtime_error("rgbaSlot format and memorySurfaceFormat do not fit!");
        } else if (memorySurfaceFormat->isRgbaInteger && !memorySurfaceFormat->isSigned) {
            UNLIKELY_IF (rgbaAttachmentDataType[rgbaSlot] != AttachmentDataType::unsignedInteger)
                throw std::runtime_error("rgbaSlot format and memorySurfaceFormat do not fit!");
        } else if (memorySurfaceFormat->isRgbaInteger && memorySurfaceFormat->isSigned) {
            UNLIKELY_IF (rgbaAttachmentDataType[rgbaSlot] != AttachmentDataType::signedInteger)
                throw std::runtime_error("rgbaSlot format and memorySurfaceFormat do not fit!");
        } else {
            if (memorySurfaceFormat->isDepth && memorySurfaceFormat->isStencil) {

            } else if (memorySurfaceFormat->isDepth) {

            } else {

            }
        }

        GLenum format = memorySurfaceFormat->componentsAndArrangement;
        GLenum type   = memorySurfaceFormat->componentsTypes;

        threadContext->cachedBindReadFbo(id);
        threadContext->cachedBindPixelPackBuffer(0);
        //only needed for RGBA copy; TODO: test if this works for the windows frame buffer on older GL implementations, or if I need to use GL_FRONT/GL_BACK
        //Not sure how much SDL2 intercepts in terms of default frame buffer, probably have to test with glfw
        threadContextGroup->functions.glReadBuffer(GL_COLOR_ATTACHMENT0 + rgbaSlot);
        threadContextGroup->functions.glReadPixels(offset.x, offset.y, size.x, size.y, format, type, mem);
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

        if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
            if (isTexture) {
                if (isLayerSelection)
                    threadContextGroup->functions.glNamedFramebufferTextureLayer(id, attachmentType, surfaceId, mipmapLevel, layer);
                else
                    threadContextGroup->functions.glNamedFramebufferTexture(id, attachmentType, surfaceId, mipmapLevel);
            } else
                threadContextGroup->functions.glNamedFramebufferRenderbuffer(id, attachmentType, GL_RENDERBUFFER, surfaceId);
        } else if (threadContextGroup->extensions.GL_EXT_direct_state_access) {
            if (isTexture) {
                if (isLayerSelection)
                    threadContextGroup->functions.glNamedFramebufferTextureLayerEXT(id, attachmentType, surfaceId, mipmapLevel, layer);
                else
                    threadContextGroup->functions.glNamedFramebufferTextureEXT(id, attachmentType, surfaceId, mipmapLevel);
            } else
                threadContextGroup->functions.glNamedFramebufferRenderbufferEXT(id, attachmentType, GL_RENDERBUFFER, surfaceId);
        } else {
            threadContext->cachedBindDrawFbo(id);
            if (isTexture) {
                if (isLayerSelection)
                    threadContextGroup->functions.glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, attachmentType, surfaceId, mipmapLevel, layer);
                else
                    threadContextGroup->functions.glFramebufferTexture(GL_DRAW_FRAMEBUFFER, attachmentType, surfaceId, mipmapLevel);
            } else
                threadContextGroup->functions.glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, surfaceId);
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
    }

    void Frame::setRgbaAttachment(
        SurfaceSelector sel,
        uint32_t        rgbaSlot
    ) {
        setAttachment(sel, GL_COLOR_ATTACHMENT0 + rgbaSlot);
        auto surfaceFormat = sel.surface->getSurfaceFormat();
        if (surfaceFormat->isRgbaNormalizedIntegerOrFloat) {
            rgbaAttachmentDataType[rgbaSlot] = AttachmentDataType::normalizedOrFloat;
        } else if (surfaceFormat->isSigned) {
            rgbaAttachmentDataType[rgbaSlot] = AttachmentDataType::signedInteger;
        } else {
            rgbaAttachmentDataType[rgbaSlot] = AttachmentDataType::unsignedInteger;
        }
    }

    void Frame::detachFromThreadContextStateCurrent() const {
        assert(!threadContext);

        int setCurrentValue = 0;
        if (Config::Workarounds::AMD_DELETING_ACTIVE_FBO_NOT_SETTING_DEFAULT_FBO) setCurrentValue = -1;

        if (threadContext->current_frame        == this) threadContext->current_frame        = 0;
        if (threadContext->current_frame_drawId ==   id) threadContext->current_frame_drawId = setCurrentValue;
        if (threadContext->current_frame_readId ==   id) threadContext->current_frame_readId = setCurrentValue;
    }

    void Frame::detachFromThreadContextState() const {
        assert(!threadContext);

        detachFromThreadContextStateCurrent();

        //int setCurrentValue = 0;
        //if (Workarounds::AMD_DELETING_ACTIVE_FBO_NOT_SETTING_DEFAULT_FBO) setCurrentValue = -1;

        if (threadContext->pending_frame        == this) threadContext->pending_frame        = 0;
        if (threadContext->pending_frame_drawId ==   id) threadContext->pending_frame_drawId = 0;
        //if (threadContext->pending_frame_readId ==   id) threadContext->pending_frame_readId = 0;
    }

    /*void Frame::detach(GLenum attachment)
    {
        if (!id) return;
        if (threadContext->extensions.GL_ARB_direct_state_access) {
            threadContext->glNamedFramebufferTexture(id, attachment, 0, 0);
        } else if (threadContext->extensions.GL_EXT_direct_state_access) {
            threadContext->glNamedFramebufferTextureEXT(id, attachment, 0, 0);
        } else {
            threadContext->cachedBindDrawFbo(id);
            threadContext->glFramebufferTexture(GL_DRAW_FRAMEBUFFER, attachment, 0, 0);
        }
    }*/


    /**
        @param target GL_NONE or GL_COLOR_ATTACHMENT0..GL_COLOR_ATTACHMENT<values.GL_MAX_DRAW_BUFFERS - 1>
    */
    /*void Frame::setDrawTarget(GLenum target) const
    {
        if (!id && !defaultFrameBuffer) return;
        if (threadContext->extensions.GL_ARB_direct_state_access) {
            threadContext->glNamedFramebufferDrawBuffer(id, target);
        } else if (threadContext->extensions.GL_EXT_direct_state_access) {
            threadContext->glFramebufferDrawBufferEXT(id, target);
        } else {
            threadContext->cachedBindDrawFbo(id);
            threadContext->glDrawBuffer(target);
        }
    }*/

    /*
        Maps the rgba shader outputs to specific rgba targets of this Frame.


        TODO: Default mapping is like the Frame creation? (Could be a problem with drivers that allow more binding points then active targets!)
              Maybe only map as many as possible? Still will work fine in most cases as resonable default!

        Can not map a draw target more then once! (TODO: intercept that one internaly here and throw)

        Can not map to draw target if FBO has nothing bound to the slot. (automaticly includes GL_MAX_COLOR_ATTACHMENTS, I guess)

        Can not set slots bayond GL_MAX_DRAW_BUFFERS



        First color render target is 0.

        To disable a specific shader color output use -1.

        GL_MAX_COLOR_ATTACHMENTS
        GL_MAX_DRAW_BUFFERS


        Each draw buffers must either specify color attachment points that have images attached or must be GL_NONE.
        (GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER when false).
        Note that this test is not performed if OpenGL 4.1 or ARB_ES2_compatibility is available.


        Config::MAX_RGBA_ATTACHMENTS
    */
    void Frame::setRgbaDrawMapping(
        int32_t slot0,
        int32_t slot1,
        int32_t slot2,
        int32_t slot3,
        int32_t slot4,
        int32_t slot5,
        int32_t slot6,
        int32_t slot7
    ) {
        if (!id) return; //or throw
        const int32_t mappingListInput[8] = {slot0, slot1, slot2, slot3, slot4, slot5, slot6, slot7};
              int32_t mappingList[8]      = {GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE};

        int highestMapping = 0;
        for (int i = 0; i<8; i++) {
            if (mappingListInput[i] > -1) {
                UNLIKELY_IF (mappingListInput[i] >= threadContextGroup->values.GL_MAX_COLOR_ATTACHMENTS)
                    throw std::runtime_error("Can not map more then GL_MAX_COLOR_ATTACHMENTS(" + to_string(threadContextGroup->values.GL_MAX_COLOR_ATTACHMENTS) + ")");
                highestMapping = i + 1;
                mappingList[i] = mappingListInput[i] + GL_COLOR_ATTACHMENT0;
            }
        }

        UNLIKELY_IF (highestMapping >= threadContextGroup->values.GL_MAX_DRAW_BUFFERS)
            throw std::runtime_error("Can not set draw mapping slot that is higher then GL_MAX_DRAW_BUFFERS(" + to_string(threadContextGroup->values.GL_MAX_DRAW_BUFFERS) + ")");

        if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
            threadContextGroup->functions.glNamedFramebufferDrawBuffers(id, highestMapping, &mappingList[0]);
        } else if (threadContextGroup->extensions.GL_EXT_direct_state_access) {
            threadContextGroup->functions.glFramebufferDrawBuffersEXT(id, highestMapping, &mappingList[0]);
        } else {
            threadContext->cachedBindDrawFbo(id);
            threadContextGroup->functions.glDrawBuffers(highestMapping, &mappingList[0]);
        }

        //else if (defaultFrameBuffer) {
        //    //TODO
        //    if (threadContext->extensions.GL_ARB_direct_state_access) {
        //        threadContext->glNamedFramebufferDrawBuffer(id, targets[0]);
        //    } else if (threadContext->extensions.GL_EXT_direct_state_access) {
        //        threadContext->glFramebufferDrawBufferEXT(id, targets[0]);
        //    } else {
        //        threadContext->cachedBindDrawFbo(id);
        //        threadContext->glDrawBuffer(targets[0]);
        //    }
        //}
    }

    /*void Frame::setReadTarget(GLenum target) {
        //if (!id && !defaultFrameBuffer) return;
        if (!id) return;
        if (threadContext->extensions.GL_ARB_direct_state_access) {
            threadContext->glNamedFramebufferReadBuffer(id, target);
        } else if (threadContext->extensions.GL_EXT_direct_state_access) {
            threadContext->glFramebufferReadBufferEXT(id, target);
        } else {
            threadContext->cachedBindReadFbo();
            threadContext->glReadBuffer(target);
        }
    }*/

    void Frame::setDefaultValues() {
        new (this) Frame();
    }
}
