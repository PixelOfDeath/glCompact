#include "glCompact/Context_.hpp"
#include "glCompact/threadContext_.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/threadContextGroup_.hpp"
#include "glCompact/Tools_.hpp"
#include "glCompact/Debug.hpp"
#include "glCompact/Frame.hpp"
#include "glCompact/PipelineInterface.hpp"
#include "glCompact/multiMalloc.h"
#include "glCompact/isDiffThenAssign.hpp"

#include <exception>
#include <stdexcept>
#include <algorithm> //msvc for max / min
#include <atomic>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    static atomic<uint32_t> nextContextId;
    /*
        Needs the current thread to have an OpenGL context with minimum 3.3!

        3.1
            restartIndex support (I really really want to have this! This is a minimum dependency!)
        3.2
            Sync objects
        3.3
            GLSL layout location (e.g. for attribute location and fragment output)
            If I ever want to support something lower I could use naming postfix: NAME_locationX

        DONE! GL_ARB_vertex_attrib_binding (core since 4.3)

        DONE! GL_ARB_multi_bind (core since 4.4)

        DONE!
            GL_ARB_shading_language_420pack (core in 4.2)
            DONE: replaced need for this extension via naming postfix: NAME_bindingX;
    */
    Context_::Context_() {
        //Allocate minimum memory to hold the first bind states in case we don't have GL_ARB_direct_state_access support
        multiMallocDescriptor md[] = {
            {&buffer_uniform_id,            &buffer_uniform_count,          1},
            {&buffer_uniform_offset,        &buffer_uniform_count,          1},
            {&buffer_uniform_size,          &buffer_uniform_count,          1},
            {&buffer_shaderStorage_id,      &buffer_shaderStorage_count,    1},
            {&buffer_shaderStorage_offset,  &buffer_shaderStorage_count,    1},
            {&buffer_shaderStorage_size,    &buffer_shaderStorage_count,    1},
            {&texture_id,                   &sampler_count,                 1},
            {&texture_target,               &sampler_count,                 1},
            {&sampler_id,                   &sampler_count,                 1},
            {&image_id,                     &image_count,                   1},
            {&image_format,                 &image_count,                   1},
            {&image_mipmapLevel,            &image_count,                   1},
            {&image_layer,                  &image_count,                   1},
        };
        multiMallocPtr = multiMalloc(md, sizeof(md));

        queryDisplayFramebufferFormat();

        contextId = nextContextId.fetch_add(1);

        Debug::enableDebugOutput();
        //TODO
        /*if (version.debug) {
            cout << "Using gl debug context, enabling error callbacks!" << endl;
            Debug::enableDebugOutput();
        } else {
            cout << "NOT using gl debug context, no error callbacks!" << endl;
        }

        const GLubyte* glVersionStringPtr = glGetString(GL_VERSION);
        string glVersionString(reinterpret_cast<const char*>(glVersionStringPtr));
        */

        defaultVaoId = 0;
        //Since OpenGL 3.0 (Core and Non-Core!) the default VAO is depricated! It only exist if the extension GL_ARB_compatibility (Not core) is present!
        //Without default VAO we just make our own "default VAO" per context via a single permanently bound one.
        if (!threadContextGroup_->extensions.GL_ARB_compatibility) {
            threadContextGroup_->functions.glGenVertexArrays(1, &defaultVaoId); //GL_ARB_direct_state_access also has glCreateVertexArrays, but we don't need it because we always bind before changing it
            threadContextGroup_->functions.glBindVertexArray(defaultVaoId);
        }
        if (threadContextGroup_->extensions.GL_ARB_vertex_attrib_binding) {
            //Default value for mapping vertex layout attributes to buffer indexes seems to be something invalid.
            //This sets all vertex attributes to the buffer index 0. This is easier then to hold a "invalid" status value in the state tracker!
            for (int i = 0; i < threadContextGroup_->values.GL_MAX_VERTEX_ATTRIBS; ++i) threadContextGroup_->functions.glVertexAttribBinding(i, 0);
        }

        defaultStatesActivate();
    }

    Context_::~Context_() {
        if (defaultVaoId) threadContextGroup_->functions.glDeleteVertexArrays(1, &defaultVaoId);
        threadContextGroup_->functions.glFinish(); //TODO: not sure if I need this here
        free(multiMallocPtr);
    }

    uint32_t Context_::getContextId() const {
        return contextId;
    }

    //TODO: maybe also use a bool that enables brute force setting all values to known, just in case any other libs fuck up?!
    void Context_::defaultStatesActivate() {
        threadContextGroup_->functions.glBindVertexArray(defaultVaoId);
        if (threadContextGroup_->extensions.GL_ARB_compatibility) {
            //TODO: set everything to default, or set the state tracker to all unknown for the default VAO states
            if (threadContextGroup_->extensions.GL_ARB_vertex_attrib_binding) {
                //This sets all vertex attributes to the buffer index 0.
                for(int i = 0; i < threadContextGroup_->values.GL_MAX_VERTEX_ATTRIBS; ++i) threadContextGroup_->functions.glVertexAttribBinding(i, 0);
            }
        }

        if (threadContextGroup_->extensions.GL_ARB_ES3_compatibility) threadContextGroup_->functions.glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    }

    void Context_::defaultStatesDeactivate() {
        if (threadContextGroup_->extensions.GL_ARB_compatibility) {
            //TODO: set all attribute values to default? Maybe just disable them?
        } else {
            threadContextGroup_->functions.glBindVertexArray(0);
        }

        if (threadContextGroup_->extensions.GL_ARB_ES3_compatibility) threadContextGroup_->functions.glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
        cachedScissorEnabled(false);

        //we may have changed this values and therefore we set them back to default
        threadContextGroup_->functions.glBindBuffer (GL_PIXEL_PACK_BUFFER,   0);
        threadContextGroup_->functions.glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);
        threadContextGroup_->functions.glPixelStorei(GL_UNPACK_SKIP_IMAGES,  0);
        threadContextGroup_->functions.glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
        buffer_pixelPackId   = 0;
        buffer_pixelUnpackId = 0;
        buffer_copyReadId    = 0;
        buffer_copyWriteId   = 0;
    }

    int32_t Context_::buffer_attribute_getHighestIndexNonNull() {
        while (buffer_attribute_highestIndexNonNull >= 0 && buffer_attribute_id[buffer_attribute_highestIndexNonNull] == 0) buffer_attribute_highestIndexNonNull--;
        return buffer_attribute_highestIndexNonNull;
    }

    int32_t Context_::buffer_uniform_getHighestIndexNonNull() {
        while (buffer_uniform_highestIndexNonNull >= 0 && buffer_uniform_id[buffer_uniform_highestIndexNonNull] == 0) buffer_uniform_highestIndexNonNull--;
        return buffer_uniform_highestIndexNonNull;
    }

    int32_t Context_::buffer_shaderStorage_getHighestIndexNonNull() {
        while (buffer_shaderStorage_highestIndexNonNull >= 0 && buffer_shaderStorage_id[buffer_shaderStorage_highestIndexNonNull] == 0) buffer_shaderStorage_highestIndexNonNull--;
        return buffer_shaderStorage_highestIndexNonNull;
    }

    int32_t Context_::texture_getHighestIndexNonNull() {
        while (texture_highestIndexNonNull >= 0 && texture_id[texture_highestIndexNonNull] == 0) texture_highestIndexNonNull--;
        return texture_highestIndexNonNull;
    }

    int32_t Context_::sampler_getHighestIndexNonNull() {
        while (sampler_highestIndexNonNull >= 0 && sampler_id[sampler_highestIndexNonNull] == 0) sampler_highestIndexNonNull--;
        return sampler_highestIndexNonNull;
    }

    int32_t Context_::image_getHighestIndexNonNull() {
        while (image_highestIndexNonNull >= 0 && image_id[image_highestIndexNonNull] == 0) image_highestIndexNonNull--;
        return image_highestIndexNonNull;
    }

    void Context_::forgetBufferId(uint32_t bufferId) {
        LOOPI(buffer_attribute_getHighestIndexNonNull()) if (buffer_attribute_id[i] == bufferId) {
            buffer_attribute_id    [i] = 0;
            buffer_attribute_offset[i] = 0;
        }
        if (buffer_attribute_index_id   == bufferId) buffer_attribute_index_id   = 0;
        if (buffer_draw_indirect_id     == bufferId) buffer_draw_indirect_id     = 0;
        if (buffer_dispatch_indirect_id == bufferId) buffer_dispatch_indirect_id = 0;
        if (buffer_parameter_id         == bufferId) buffer_parameter_id         = 0;

        LOOPI(buffer_uniform_getHighestIndexNonNull()) if (buffer_uniform_id[i] == bufferId) {
            buffer_uniform_id    [i] = 0;
            buffer_uniform_offset[i] = 0;
            buffer_uniform_size  [i] = 0;
        }

        LOOPI(buffer_shaderStorage_getHighestIndexNonNull()) if (buffer_shaderStorage_id[i] == bufferId) {
            buffer_shaderStorage_id    [i] = 0;
            buffer_shaderStorage_offset[i] = 0;
            buffer_shaderStorage_size  [i] = 0;
        }

        if (buffer_pixelPackId          == bufferId) buffer_pixelPackId          = 0;
        if (buffer_pixelUnpackId        == bufferId) buffer_pixelUnpackId        = 0;

        if (buffer_copyReadId           == bufferId) buffer_copyReadId           = 0;
        if (buffer_copyWriteId          == bufferId) buffer_copyWriteId          = 0;

        if (boundArrayBuffer            == bufferId) boundArrayBuffer            = 0;
    }

    /**
        This bind the texture on the specified unit for changes with non-DSA functions.

        In classical OpenGL, multiple targets can be bound to a single texture unit at the same time.
        Modern binding functions will take care of only having one target per unit active.
        To not overcomplicate our state tracker, we unbind any previous target type before we bind a different target type, when using this old-style functions.

        Only ARB DSA/storage functions can create texture or buffer objects without binding the ID at last once.
        This is why we may also need to use this old stile binding for new textures if we create them without ARB DSA/storage functions
    */
    void Context_::cachedBindTextureCompatibleOrFirstTime(
        uint32_t texSlot,
         int32_t texTarget,
        uint32_t texId
    ) {
         int32_t texTargetOld    = texture_target[texSlot];
        uint32_t texIdOld        = texture_id    [texSlot];
        bool     targetChange    = texTarget != texTargetOld;
        bool     textureChange   = texId     != texIdOld;
        bool     unbindOldTarget = targetChange  && texIdOld;
        bool     bindNewTexture  = textureChange && texId;
        if (unbindOldTarget || bindNewTexture) {
            cachedSetActiveTextureUnit(texSlot);
            if (pipeline) pipeline->texture_markSlotChange(texSlot);
        }
        if (unbindOldTarget) threadContextGroup_->functions.glBindTexture(texTargetOld, 0);
        if (bindNewTexture)  threadContextGroup_->functions.glBindTexture(texTarget   , texId);
        if (targetChange)    texture_target[texSlot] = texTarget;
        if (textureChange)   texture_id    [texSlot] = texId;
    }

    /**
        This binds the texture on the specified unit for changes with non-DSA functions.
    */
    void Context_::cachedBindTexture(
        uint32_t texSlot,
         int32_t texTarget,
        uint32_t texId
    ) {
        if (threadContextGroup_->extensions.GL_ARB_multi_bind) {
            if (texture_id[texSlot] != texId) {
                texture_id[texSlot] = texId;
                if (pipeline) pipeline->texture_markSlotChange(texSlot);
                threadContextGroup_->functions.glBindTextures(texSlot, 1, &texId);
            }
        } else {
            cachedBindTextureCompatibleOrFirstTime(texSlot, texTarget, texId);
        }
    }

    void Context_::cachedBindShader(
        uint32_t pipelineShaderId
    ) {
        if (isDiffThenAssign(this->pipelineShaderId, pipelineShaderId)) {
            threadContextGroup_->functions.glUseProgram(pipelineShaderId);
        }
    }

    void Context_::cachedBindArrayBuffer(
        uint32_t bufferId
    ) {
        if (isDiffThenAssign(boundArrayBuffer, bufferId)) {
            threadContextGroup_->functions.glBindBuffer(GL_ARRAY_BUFFER, bufferId);
        }
    }

    void Context_::cachedBindIndexBuffer(
        uint32_t bufferId
    ) {
        if (isDiffThenAssign(buffer_attribute_index_id, bufferId)) {
            threadContextGroup_->functions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
        }
    }

    //takes i and sets GL_TEXTURE0 + i
    void Context_::cachedSetActiveTextureUnit(
        uint32_t slot
    ) {
        //TODO: debug test for values over GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS-1
        UNLIKELY_IF (slot >= uint32_t(threadContextGroup_->values.GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS))
            throw runtime_error("Trying to set active texture bayond GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS(" + to_string(threadContextGroup_->values.GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS) + ")");
        if (isDiffThenAssign(activeTextureSlot, slot)) {
            threadContextGroup_->functions.glActiveTexture(GL_TEXTURE0 + slot);
        }
    }

    void Context_::cachedBindDrawFbo(
        uint32_t fboId
    ) {
        if (isDiffThenAssign(current_frame_drawId, fboId)) {
            threadContextGroup_->functions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);
        }
    }

    void Context_::cachedBindReadFbo(
        uint32_t fboId
    ) {
        if (isDiffThenAssign(current_frame_readId, fboId)) {
            threadContextGroup_->functions.glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
        }
    }

    void Context_::cachedSrgbTargetsReadWriteLinear(bool value) {
        if (isDiffThenAssign(current_srgbTargetsReadWriteLinear, value)) {
            if ( value) threadContextGroup_->functions.glEnable (GL_FRAMEBUFFER_SRGB);
            if (!value) threadContextGroup_->functions.glDisable(GL_FRAMEBUFFER_SRGB);
        }
    }

    void Context_::cachedViewport(
        glm::uvec2 offset,
        glm::uvec2 size
    ) {
        if (isDiffThenAssign(
            current_viewportOffset, offset,
            current_viewportSize,   size
        )) {
            threadContextGroup_->functions.glViewport(offset.x, offset.y, size.x, size.y);
        }
    }

    void Context_::cachedScissorEnabled(
        bool enabled
    ) {
        if (isDiffThenAssign(current_scissor_enabled, enabled)) {
            if (enabled)
                threadContextGroup_->functions.glEnable(GL_SCISSOR_TEST);
            else
                threadContextGroup_->functions.glDisable(GL_SCISSOR_TEST);
        }
    }

    void Context_::cachedScissor(
        glm::uvec2 offset,
        glm::uvec2 size
    ) {
        if (isDiffThenAssign(
            current_scissorOffset, offset,
            current_scissorSize,   size
        )) {
            threadContextGroup_->functions.glScissor(offset.x, offset.y, size.x, size.y);
        }
    }

    void Context_::cachedBindPixelPackBuffer(
        uint32_t bufferId
    ) {
        if (isDiffThenAssign(buffer_pixelPackId, bufferId)) {
            threadContextGroup_->functions.glBindBuffer(GL_PIXEL_PACK_BUFFER, bufferId);
        }
    }

    void Context_::cachedBindPixelUnpackBuffer(
        uint32_t bufferId
    ) {
        if (isDiffThenAssign(buffer_pixelUnpackId, bufferId)) {
            threadContextGroup_->functions.glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufferId);
        }
    }

    void Context_::cachedBindCopyReadBuffer(
        uint32_t bufferId
    ) {
        if (isDiffThenAssign(buffer_copyReadId, bufferId)) {
            threadContextGroup_->functions.glBindBuffer(GL_COPY_READ_BUFFER, bufferId);
        }
    }

    void Context_::cachedBindCopyWriteBuffer(
        uint32_t bufferId
    ) {
        if (isDiffThenAssign(buffer_copyWriteId, bufferId)) {
            threadContextGroup_->functions.glBindBuffer(GL_COPY_WRITE_BUFFER, bufferId);
        }
    }

    //Different kind of parameter buffer
    //DRAW_INDIRECT_BUFFER, DISPATCH_INDIRECT_BUFFER, PARAMETER_BUFFER_ARB
    void Context_::cachedBindDrawIndirectBuffer(
        uint32_t bufferId
    ) {
        if (isDiffThenAssign(buffer_draw_indirect_id, bufferId)) {
            threadContextGroup_->functions.glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buffer_draw_indirect_id);
        }
    }

    void Context_::cachedBindDispatchIndirectBuffer(
        uint32_t bufferId
    ) {
        if (isDiffThenAssign(buffer_dispatch_indirect_id, bufferId)) {
            threadContextGroup_->functions.glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, buffer_dispatch_indirect_id);
        }
    }

    //GL_PARAMETER_BUFFER is the smae as GL_PARAMETER_BUFFER_ARB!
    void Context_::cachedBindParameterBuffer(
        uint32_t bufferId
    ) {
        if (isDiffThenAssign(buffer_parameter_id, bufferId)) {
            threadContextGroup_->functions.glBindBuffer(GL_PARAMETER_BUFFER, buffer_parameter_id);
        }
    }
        }
    }

    void Context_::processPendingChangesDrawFrame() {
        UNLIKELY_IF(!pending_frame)
            throw runtime_error("No draw frame is set!");
        processPendingChangesDrawFrame(pending_frame);
    }

    void Context_::processPendingChangesDrawFrame(Frame* pendingFrame) {
        UNLIKELY_IF (!pendingFrame)
            throw runtime_error("This command needs an valid Frame set via setDrawFrame()!");
        if (isDiffThenAssign(current_frame, pendingFrame)) {
            pending_frame_drawId = pendingFrame->id;
        }
        threadContext_->cachedBindDrawFbo(threadContext_->pending_frame_drawId);
        threadContext_->cachedSrgbTargetsReadWriteLinear(current_frame->srgbTargetsReadWriteLinear);
        cachedViewport(pendingFrame->viewportOffset, pendingFrame->viewportSize);

        if (!pendingFrame->scissorEnabled) {
            cachedScissorEnabled(false);
        } else {
            cachedScissorEnabled(true);
            cachedScissor(pendingFrame->scissorOffset, pendingFrame->scissorSize);
        }
    }

    void Context_::processPendingChangesMemoryBarriers() {
        if (memoryBarrierMask) {
            threadContextGroup_->functions.glMemoryBarrier(memoryBarrierMask);
            memoryBarrierMask = 0;
        }
    }

    void Context_::processPendingChangesMemoryBarriersRasterizationRegion() {
        if (memoryBarrierRasterizationRegionMask) {
            threadContextGroup_->functions.glMemoryBarrierByRegion(memoryBarrierRasterizationRegionMask);
            memoryBarrierRasterizationRegionMask = 0;
        }
    }

    //This function queries the surface formats of the default framebuffer of this context
    //We need SurfaceFormat to have the correct bits set for rgba/depth/stencil, signed/unsiged, normalized/float
    //for copyConvert from it to memory/buffer ond to be able to name the format when we output errors.
    void Context_::queryDisplayFramebufferFormat() {
        //glGetFramebufferAttachmentParameteriv (Querying the default framebuffer is supported since GL 3.0/GLES 3.0)
        //In a new context GL_FRAMEBUFFER is set to the default framebuffer. So we don't need to set it here.
        auto getFboAttachmentParam = [](uint32_t attachment, uint32_t paramName) -> uint32_t {
            uint32_t ret = 0;
            threadContextGroup_->functions.glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, paramName, reinterpret_cast<GLint*>(&ret));
            return ret;
        };

        auto attachmentCommponentTypeToString = [](uint32_t commponentType) -> string {
            switch (commponentType) {
                case GL_FLOAT:              return "SFLOAT";
                case GL_INT:                return "SINT";
                case GL_UNSIGNED_INT:       return "UINT";
                case GL_SIGNED_NORMALIZED:  return "SNORM";
                case GL_UNSIGNED_NORMALIZED:return "UNORM";
                default: return "UNKNOWN_COMMPONENT_TYPE(" + to_string(commponentType) + ")";
            };
        };

        //In GL 3.2 and GLES 3.0 there are different rgba "attachments" used for the default framebuffer. Later GL versions also work with GL_BACK from GLES.
        bool usingGL = threadContextGroup_->version.gl != GlVersion::notSupported;
        uint32_t GL_GLES_RGBA = usingGL ? GL_FRONT_LEFT : GL_BACK;

        //DEPTH and/or STENCIL output format
        //returns GL_NONE == 0, GL_FRAMEBUFFER_DEFAULT, GL_TEXTURE, or GL_RENDERBUFFER
        uint32_t depthAttachment   =               getFboAttachmentParam(GL_DEPTH    , GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE);
        uint32_t stencilAttachment =               getFboAttachmentParam(GL_STENCIL  , GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE);
        uint32_t rgbaAttachment    =               getFboAttachmentParam(GL_GLES_RGBA, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE);

        uint32_t depthBits   = depthAttachment   ? getFboAttachmentParam(GL_DEPTH    , GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE    ) : 0;
        uint32_t depthType   = depthAttachment   ? getFboAttachmentParam(GL_DEPTH    , GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE) : 0;
        uint32_t stencilBits = stencilAttachment ? getFboAttachmentParam(GL_STENCIL  , GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE    ) : 0;
        uint32_t stencilType = stencilAttachment ? getFboAttachmentParam(GL_STENCIL  , GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE) : 0;
        uint32_t rBits       = rgbaAttachment    ? getFboAttachmentParam(GL_GLES_RGBA, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE      ) : 0;
        uint32_t gBits       = rgbaAttachment    ? getFboAttachmentParam(GL_GLES_RGBA, GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE    ) : 0;
        uint32_t bBits       = rgbaAttachment    ? getFboAttachmentParam(GL_GLES_RGBA, GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE     ) : 0;
        uint32_t aBits       = rgbaAttachment    ? getFboAttachmentParam(GL_GLES_RGBA, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE    ) : 0;
        uint32_t rgbaType    = rgbaAttachment    ? getFboAttachmentParam(GL_GLES_RGBA, GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE) : 0;
        bool     rgbaSrgb    = rgbaAttachment    ? getFboAttachmentParam(GL_GLES_RGBA, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING) == GL_SRGB : false;

        bool isDepth                        = depthAttachment != 0;
        bool isDepthSigned                  = depthType == GL_FLOAT || depthType == GL_INT || depthType == GL_SIGNED_NORMALIZED;
        bool isStencil                      = stencilAttachment != 0;
        bool isRgbaNormalizedIntegerOrFloat = rgbaType == GL_SIGNED_NORMALIZED || rgbaType == GL_UNSIGNED_NORMALIZED || rgbaType == GL_FLOAT;
        bool isRgbaInteger                  = rgbaType == GL_INT || rgbaType == GL_UNSIGNED_INT;
        bool isRgbaSigned                   = rgbaType == GL_FLOAT || rgbaType == GL_INT || rgbaType == GL_SIGNED_NORMALIZED;

        int16_t bitsPerPixelDepthStencil = depthBits + stencilBits;
        int16_t bitsPerPixelRgba = rBits + gBits + bBits + aBits;

        depthAndOrStencilSurfaceFormatString =
              "DISPLAY_FRAMEBUFFER_DEPTH_AND_OR_STENCIL"
            + (depthBits    ? (" D" + to_string(depthBits)   + "_" + attachmentCommponentTypeToString(  depthType)) : " NO_DEPTH")
            + (stencilBits  ? (" S" + to_string(stencilBits) + "_" + attachmentCommponentTypeToString(stencilType)) : " NO_STENCIL");

        rgbaSurfaceFormatString =
            "DISPLAY_FRAMEBUFFER_RGBA "
            + ((bitsPerPixelRgba) ? (
                  (rBits ? ("R" + to_string(rBits)) : "")
                + (gBits ? ("G" + to_string(gBits)) : "")
                + (bBits ? ("B" + to_string(bBits)) : "")
                + (aBits ? ("A" + to_string(aBits)) : "")
                +  (rgbaSrgb ? "_SRGB" : attachmentCommponentTypeToString(rgbaType))
            ) : "NO_RGBA");

        //                                    name                                          sizedFormat (Ignored for FB format)
        //                                    |                                             |  attachmentType (Ignored for FB format)
        //                                    |                                             |  |                          bitsPerPixel
        //                                    |                                             |  |                          |    blockSizeX
        //                                    |                                             |  |                          |    |   blockSizeY
        //                                    |                                             |  |                          |    |   |    isRenderable
        //                                    |                                             |  |                          |    |   |    |  isCompressed
        //                                    |                                             |  |                          |    |   |    |  |         isSrgb
        //                                    |                                             |  |                          |    |   |    |  |         |  imageSupport
        //                                    |                                             |  |                          |    |   |    |  |         |  |  sparseSupport
        //                                    |                                             |  |                          |    |   |    |  |         |  |  |                                isRgbaNormalizedIntegerOrFloat
        //                                    |                                             |  |                          |    |   |    |  |         |  |  |                                |              isRgbaInteger
        //                                    |                                             |  |                          |    |   |    |  |         |  |  |                                |              |        isDepth
        //                                    |                                             |  |                          |    |   |    |  |         |  |  |                                |              |        |          isStencil
        //                                    |                                             |  |                          |    |   |    |  |         |  |  |                                |              |        |          |  isSigned
        //                                    |                                             |  |                          |    |   |    |  |         |  |  |                                |              |        |          |  |
        defaultFramebufferSurfaceFormat[0] = {depthAndOrStencilSurfaceFormatString.c_str(), 0, 0,  bitsPerPixelDepthStencil,   1,  1,   1, 0,        0, 0, 0,                               1,             0, isDepth, isStencil, isDepthSigned};
        defaultFramebufferSurfaceFormat[1] = {rgbaSurfaceFormatString.c_str()             , 0, 0,          bitsPerPixelRgba,   1,  1,   1, 0, rgbaSrgb, 0, 0,  isRgbaNormalizedIntegerOrFloat, isRgbaInteger,       0,         0, isRgbaSigned};

        //This two integers are hardcoded to point to the correct outputFrameSurfaceFormat entries!
        displayFrame.depthAndOrStencilSurfaceFormat = static_cast<SurfaceFormat::FormatEnum>(2000);
        displayFrame.rgbaSurfaceFormat[0]           = static_cast<SurfaceFormat::FormatEnum>(2001);
    }
}
