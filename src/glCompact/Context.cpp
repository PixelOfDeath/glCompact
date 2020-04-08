#include "glCompact/Context.hpp"
#include "glCompact/ThreadContext.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/ToolsInternal.hpp"
#include "glCompact/Debug.hpp"
#include "glCompact/Frame.hpp"

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
    Context::Context() {
        frameWindow.rgbaAttachmentDataType[0] = Frame::AttachmentDataType::normalizedOrFloat;
        contextId = nextContextId.fetch_add(1);

        if (!threadContextGroup_->version.equalOrGreater(3, 3))
            crash("glCompact requires Opengl 3.3 or higher!");

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

    Context::~Context() {
        if (defaultVaoId) threadContextGroup_->functions.glDeleteVertexArrays(1, &defaultVaoId);
        threadContextGroup_->functions.glFinish(); //TODO: not sure if I need this here
    }

    uint32_t Context::getContextId() const {
        return contextId;
    }

    void Context::assertThreadHasActiveGlContext() {
        #ifdef GLCOMPACT_DEBUG_ASSERT_THREAD_HAS_ACTIVE_CONTEXT
            UNLIKELY_IF (!threadContextGroup_->functions.glGetString(GL_VERSION))
                crash("Trying to use OpenGL functions in a thread without active context!");
        #endif
    }

    //TODO: maybe also use a bool that enables brute force setting all values to known, just in case any other libs fuck up?!
    void Context::defaultStatesActivate() {
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

    void Context::defaultStatesDeactivate() {
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

    void Context::buffer_attribute_markSlotChange(
        int8_t slot
    ) {
        buffer_attribute_changedSlotMin = min(buffer_attribute_changedSlotMin, slot);
        buffer_attribute_changedSlotMax = max(buffer_attribute_changedSlotMax, slot);
        buffer_attribute_maxHighestNonNull = max(buffer_attribute_maxHighestNonNull, slot);
    }

    int8_t Context::buffer_attribute_getHighestNonNull() {
        while (buffer_attribute_maxHighestNonNull >= 0 && buffer_attribute_id[buffer_attribute_maxHighestNonNull] == 0) buffer_attribute_maxHighestNonNull--;
        return buffer_attribute_maxHighestNonNull;
    }

    void Context::buffer_uniform_markSlotChange(
        int32_t slot
    ) {
        buffer_uniform_changedSlotMin = min(buffer_uniform_changedSlotMin, slot);
        buffer_uniform_changedSlotMax = max(buffer_uniform_changedSlotMax, slot);
    }

    void Context::buffer_atomicCounter_markSlotChange(
        int32_t slot
    ) {
        buffer_atomicCounter_changedSlotMin = min(buffer_atomicCounter_changedSlotMin, slot);
        buffer_atomicCounter_changedSlotMax = max(buffer_atomicCounter_changedSlotMax, slot);
    }

    int32_t Context::buffer_uniform_getHighestNonNull() {
        while (buffer_uniform_maxHighestNonNull >= 0 && buffer_uniform_id[buffer_uniform_maxHighestNonNull] == 0) buffer_uniform_maxHighestNonNull--;
        return buffer_uniform_maxHighestNonNull;
    }

    int32_t Context::buffer_atomicCounter_getHighestNonNull() {
        while (buffer_atomicCounter_maxHighestNonNull >= 0 && buffer_atomicCounter_id[buffer_atomicCounter_maxHighestNonNull] == 0) buffer_atomicCounter_maxHighestNonNull--;
        return buffer_atomicCounter_maxHighestNonNull;
    }

    void Context::buffer_shaderStorage_markSlotChange(
        int32_t slot
    ) {
        buffer_shaderStorage_changedSlotMin = min(buffer_shaderStorage_changedSlotMin, slot);
        buffer_shaderStorage_changedSlotMax = max(buffer_shaderStorage_changedSlotMax, slot);
    }

    int32_t Context::buffer_shaderStorage_getHighestNonNull() {
        while (buffer_shaderStorage_maxHighestNonNull >= 0 && buffer_shaderStorage_id[buffer_shaderStorage_maxHighestNonNull] == 0) buffer_shaderStorage_maxHighestNonNull--;
        return buffer_shaderStorage_maxHighestNonNull;
    }

    void Context::texture_markSlotChange(
        int32_t slot
    ) {
        texture_changedSlotMin = min(texture_changedSlotMin, slot);
        texture_changedSlotMax = max(texture_changedSlotMax, slot);
    }

    int32_t Context::texture_getHighestNonNull() {
        while (texture_maxHighestNonNull >= 0 && texture_id[texture_maxHighestNonNull] == 0) texture_maxHighestNonNull--;
        return texture_maxHighestNonNull;
    }

    void Context::sampler_markSlotChange(
        int32_t slot
    ) {
        sampler_changedSlotMin = min(sampler_changedSlotMin, slot);
        sampler_changedSlotMax = max(sampler_changedSlotMax, slot);
    }

    int32_t Context::sampler_getHighestNonNull() {
        while (sampler_maxHighestNonNull >= 0 && sampler_id[sampler_maxHighestNonNull] == 0) sampler_maxHighestNonNull--;
        return sampler_maxHighestNonNull;
    }

    void Context::image_markSlotChange(
        int32_t slot
    ) {
        image_changedSlotMin = min(image_changedSlotMin, slot);
        image_changedSlotMax = max(image_changedSlotMax, slot);
    }

    int32_t Context::image_getHighestNonNull() {
        while (image_maxHighestNonNull >= 0 && image_id[image_maxHighestNonNull] == 0) image_maxHighestNonNull--;
        return image_maxHighestNonNull;
    }

    /**
        This bind the texture on the specified unit for changes with non-DSA functions.

        In classical OpenGL, multiple targets can be bound to a single texture unit at the same time.
        Modern binding functions will take care of only having one target per unit active.
        To not overcomplicate our state tracker, we unbind any previous target type before we bind a different target type, when using this old-style functions.

        Only ARB DSA/storage functions can create texture or buffer objects without binding the ID at last once.
        This is why we may also need to use this old stile binding for new textures if we create them without ARB DSA/storage functions
    */
    void Context::cachedBindTextureCompatibleOrFirstTime(
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
            texture_markSlotChange(texSlot);
        }
        if (unbindOldTarget) threadContextGroup_->functions.glBindTexture(texTargetOld, 0);
        if (bindNewTexture)  threadContextGroup_->functions.glBindTexture(texTarget   , texId);
        if (targetChange)    texture_target[texSlot] = texTarget;
        if (textureChange)   texture_id    [texSlot] = texId;
    }

    /**
        This binds the texture on the specified unit for changes with non-DSA functions.
    */
    void Context::cachedBindTexture(
        uint32_t texSlot,
         int32_t texTarget,
        uint32_t texId
    ) {
        if (threadContextGroup_->extensions.GL_ARB_multi_bind) {
            if (texture_id[texSlot] != texId) {
                texture_id[texSlot] = texId;
                texture_markSlotChange(texSlot);
                threadContextGroup_->functions.glBindTextures(texSlot, 1, &texId);
            }
        } else {
            cachedBindTextureCompatibleOrFirstTime(texSlot, texTarget, texId);
        }
    }

    void Context::cachedBindShader(
        uint32_t newShaderId
    ) {
        if (shaderId != newShaderId) {
            threadContextGroup_->functions.glUseProgram(newShaderId);
            shaderId = newShaderId;
        }
    }

    void Context::cachedBindArrayBuffer(
        uint32_t bufferId
    ) {
        if (boundArrayBuffer != bufferId) {
            threadContextGroup_->functions.glBindBuffer(GL_ARRAY_BUFFER, bufferId);
            boundArrayBuffer = bufferId;
        }
    }

    //takes i and sets GL_TEXTURE0 + i
    void Context::cachedSetActiveTextureUnit(
        uint32_t slot
    ) {
        //TODO: debug test for values over GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS-1
        UNLIKELY_IF (slot >= uint32_t(threadContextGroup_->values.GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS))
            throw runtime_error("Trying to set active texture bayond GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS(" + to_string(threadContextGroup_->values.GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS) + ")");
        if (activeTextureSlot != slot) {
            activeTextureSlot = slot;
            threadContextGroup_->functions.glActiveTexture(GL_TEXTURE0 + slot);
        }
    }

    void Context::cachedBindDrawFbo(
        uint32_t fboId
    ) {
        if (current_frame_drawId != fboId) {
            current_frame_drawId = fboId;
            threadContextGroup_->functions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);
        }
    }

    void Context::cachedBindReadFbo(
        uint32_t fboId
    ) {
        if (current_frame_readId != fboId) {
            current_frame_readId = fboId;
            threadContextGroup_->functions.glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
        }
    }

    void Context::cachedConvertSrgb(bool enabled) {
        if (current_convertSrgb != enabled) {
            if ( enabled) threadContextGroup_->functions.glEnable (GL_FRAMEBUFFER_SRGB);
            if (!enabled) threadContextGroup_->functions.glDisable(GL_FRAMEBUFFER_SRGB);
            current_convertSrgb = enabled;
        }
    }

    void Context::cachedViewport(
        glm::uvec2 offset,
        glm::uvec2 size
    ) {
        if (current_viewportOffset != offset
        ||  current_viewportSize   != size
        ) {
            current_viewportOffset = offset;
            current_viewportSize   = size;
            threadContextGroup_->functions.glViewport(offset.x, offset.y, size.x, size.y);
        }
    }

    void Context::cachedScissorEnabled(
        bool enabled
    ) {
        if (current_scissor_enabled != enabled) {
            current_scissor_enabled = enabled;
            if (enabled)
                threadContextGroup_->functions.glEnable(GL_SCISSOR_TEST);
            else
                threadContextGroup_->functions.glDisable(GL_SCISSOR_TEST);
        }
    }

    void Context::cachedScissor(
        glm::uvec2 offset,
        glm::uvec2 size
    ) {
        if (current_scissorOffset != offset
        ||  current_scissorSize   != size
        ) {
            current_scissorOffset = offset;
            current_scissorSize   = size;
            threadContextGroup_->functions.glScissor(offset.x, offset.y, size.x, size.y);
        }
    }

    void Context::cachedBindPixelPackBuffer(
        uint32_t bufferId
    ) {
        if (buffer_pixelPackId != bufferId) {
            threadContextGroup_->functions.glBindBuffer(GL_PIXEL_PACK_BUFFER, bufferId);
            buffer_pixelPackId = bufferId;
        }
    }

    void Context::cachedBindPixelUnpackBuffer(
        uint32_t bufferId
    ) {
        if (buffer_pixelUnpackId != bufferId) {
            threadContextGroup_->functions.glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufferId);
            buffer_pixelUnpackId = bufferId;
        }
    }

    void Context::cachedBindCopyReadBuffer(
        uint32_t bufferId
    ) {
        if (buffer_copyReadId != bufferId) {
            threadContextGroup_->functions.glBindBuffer(GL_COPY_READ_BUFFER, bufferId);
            buffer_copyReadId = bufferId;
        }
    }

    void Context::cachedBindCopyWriteBuffer(
        uint32_t bufferId
    ) {
        if (buffer_copyWriteId != bufferId) {
            threadContextGroup_->functions.glBindBuffer(GL_COPY_WRITE_BUFFER, bufferId);
            buffer_copyWriteId = bufferId;
        }
    }

    //Different kind of parameter buffer
    //DRAW_INDIRECT_BUFFER, DISPATCH_INDIRECT_BUFFER, PARAMETER_BUFFER_ARB
    void Context::cachedBindDrawIndirectBuffer(
        uint32_t bufferId
    ) {
        if (buffer_draw_indirect_id != bufferId) {
            threadContextGroup_->functions.glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buffer_draw_indirect_id);
            buffer_draw_indirect_id = bufferId;
        }
    }

    void Context::cachedBindDispatchIndirectBuffer(
        uint32_t bufferId
    ) {
        if (buffer_dispatch_indirect_id != bufferId) {
            threadContextGroup_->functions.glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, buffer_dispatch_indirect_id);
            buffer_dispatch_indirect_id = bufferId;
        }
    }

    //GL_PARAMETER_BUFFER is the smae as GL_PARAMETER_BUFFER_ARB!
    void Context::cachedBindParameterBuffer(
        uint32_t bufferId
    ) {
        if (buffer_parameter_id != bufferId) {
            threadContextGroup_->functions.glBindBuffer(GL_PARAMETER_BUFFER, buffer_parameter_id);
            buffer_parameter_id = bufferId;
        }
    }

    void Context::processPendingChangesDrawFrame() {
        UNLIKELY_IF(!pending_frame)
            throw runtime_error("No draw frame is set!");
        processPendingChangesDrawFrame(pending_frame);
    }

    void Context::processPendingChangesDrawFrame(Frame* pendingFrame) {
        UNLIKELY_IF (!pendingFrame)
            throw runtime_error("This command needs an valid Frame set via setFrame()!");
        if (current_frame != pendingFrame) {
            pending_frame_drawId = pendingFrame->id;
            current_frame = pendingFrame;
        }
        threadContext->cachedBindDrawFbo(threadContext->pending_frame_drawId);
        threadContext->cachedConvertSrgb(current_frame->convertSrgb);
        cachedViewport(pendingFrame->viewportOffset, pendingFrame->viewportSize);

        if (!pendingFrame->scissorEnabled) {
            cachedScissorEnabled(false);
        } else {
            cachedScissorEnabled(true);
            cachedScissor(pendingFrame->scissorOffset, pendingFrame->scissorSize);
        }
    }

    void Context::processPendingChangesMemoryBarriers() {
        if (memoryBarrierMask) {
            threadContextGroup_->functions.glMemoryBarrier(memoryBarrierMask);
            memoryBarrierMask = 0;
        }
    }

    void Context::processPendingChangesMemoryBarriersRasterizationRegion() {
        if (memoryBarrierRasterizationRegionMask) {
            threadContextGroup_->functions.glMemoryBarrierByRegion(memoryBarrierRasterizationRegionMask);
            memoryBarrierRasterizationRegionMask = 0;
        }
    }
}
