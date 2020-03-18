#include "glCompact/MemoryBarrier.hpp"
#include "glCompact/ThreadContext.hpp"
#include "glCompact/gl/Constants.hpp"

/**
    MEMORY BARRIER COMMANDS

    All this barriers are only needed in the context of read/writes via:
    Image Load Store  GL_ARB_shader_image_load_store      (Core since 4.2)
                      GL_ARB_shader_storage_buffer_object (Core since 4.3)
    and (?) bindless textures, bindless buffers(nv only)

    Core in GL 4.2
    glMemoryBarrier(GLbitfield barriers);

    Core in GL 4.5 (ARB_ES3_1_compatibility)
    glMemoryBarrierByRegion(GLbitfield barriers);
        only applies to memory transactions that may be read by or written by a fragment shader (Also other laod/store operations in other shader stages???)
        Seems to be mainly aimed at tilled hardware!?


    glMemoryBarrier
    |  glMemoryBarrierByRegion
    |  |
    y     GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT    GL_VERTEX_ATTRIB_ARRAY_BUFFER
    y     GL_ELEMENT_ARRAY_BARRIER_BIT          GL_ELEMENT_ARRAY_BUFFER
    y     GL_COMMAND_BARRIER_BIT                GL_DRAW_INDIRECT_BUFFER, GL_DISPATCH_INDIRECT_BUFFER
    y     GL_PIXEL_BUFFER_BARRIER_BIT           GL_PIXEL_PACK_BUFFER, GL_PIXEL_UNPACK_BUFFER
    y     GL_TEXTURE_UPDATE_BARRIER_BIT         Tex(Sub)Image*, ClearTex*Image, CopyTex*, or CompressedTex*, and reads via GetTexImage after the barrier will not execute until all shader writes initiated prior to the barrier complete.
    y     GL_BUFFER_UPDATE_BARRIER_BIT
    y     GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT
    y     GL_QUERY_BUFFER_BARRIER_BIT
    y     GL_TRANSFORM_FEEDBACK_BARRIER_BIT

    y  y  GL_FRAMEBUFFER_BARRIER_BIT
    y  y  GL_UNIFORM_BARRIER_BIT                All uniform buffers
    y  y  GL_TEXTURE_FETCH_BARRIER_BIT          Textures and buffer textures
    y  y  GL_SHADER_IMAGE_ACCESS_BARRIER_BIT    image load, store, and atomic functions
    y  y  GL_ATOMIC_COUNTER_BARRIER_BIT
    y  y  GL_SHADER_STORAGE_BARRIER_BIT
    y  y  GL_ALL_BARRIER_BITS                   (In case of glMemoryBarrierByRegion, only includes all other values that are supported by it)


    Call this function directly before issuing GL commands that access data that needs a barrier

    Simple upload functions like TexSubImage* or BufferSubData are automatically synchronized by OpenGL.



    For performance reasons writes to directly mapped memory or by shaders (Excluding FB or FBO rendering) are NOT synchronised by OpenGL.
    They need explicit synchronization (memory barriers) before commands are issues that access data that is written by previous commands or before changing data that may still is accessed by previous commands!



    GL Docs:
    Calling memoryBarrier guarantees that any memory transactions issued by the shader invocation prior to the call
    complete prior to the memory transactions issued after the call.

    "To permit cases where textures or buffers may
    be read or written in different pipeline stages without the overhead of automatic
    synchronization, buffer object and texture stores performed by shaders are not automatically synchronized with other GL operations using the same memory.
    Explicit synchronization is required to ensure that the effects of buffer and texture data stores performed by shaders will be visible to subsequent operations using
    the same objects and will not overwrite data still to be read by previously requested operations."
*/


/*enum class MemoryBarrierType : GLenum {
    attributeBuffer                     = GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT,
    attributeIndexBuffer                = GL_ELEMENT_ARRAY_BARRIER_BIT,
    uniformBuffer                       = GL_UNIFORM_BARRIER_BIT,
    textureFetch                        = GL_TEXTURE_FETCH_BARRIER_BIT,             //also texture-buffer objects
    imageShaderAccess                   = GL_SHADER_IMAGE_ACCESS_BARRIER_BIT,
    parameterBuffer                     = GL_COMMAND_BARRIER_BIT,
    bufferUploadDownloadImage           = GL_PIXEL_BUFFER_BARRIER_BIT,
    imageUploadClearDownload            = GL_TEXTURE_UPDATE_BARRIER_BIT,
    bufferCreateClearCopyInvalidate     = GL_BUFFER_UPDATE_BARRIER_BIT, //copy only means from/to another buffer, not upload download to unmanaged client memory(?!)
    frame                               = GL_FRAMEBUFFER_BARRIER_BIT,
                                        = GL_TRANSFORM_FEEDBACK_BARRIER_BIT,
                                        = GL_QUERY_BUFFER_BARRIER_BIT,
                                        = GL_ATOMIC_COUNTER_BARRIER_BIT,
                                        = GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT,
    shaderStorage                       = GL_SHADER_STORAGE_BARRIER_BIT,
    all                                 = GL_ALL_BARRIER_BITS
};*/

using namespace glCompact::gl;

namespace glCompact {
    //read access after barrier
    void setAttributeBufferBarrier() {
        threadContext->memoryBarrierMask |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
    }

    void setAttributeIndexBufferBarrier() {
        threadContext->memoryBarrierMask |= GL_ELEMENT_ARRAY_BARRIER_BIT;
    }

    //GL_DRAW_INDIRECT_BUFFER and GL_DISPATCH_INDIRECT_BUFFER
    //Also GL_PARAMETER_BUFFER_ARB if the extension GL_ARB_indirect_parameters (Not part of Core) is present
    void setParameterBufferBarrier() {
        threadContext->memoryBarrierMask |= GL_COMMAND_BARRIER_BIT;
    }

    void setUniformBufferBarrier() {
        threadContext->memoryBarrierMask |= GL_UNIFORM_BARRIER_BIT;
    }

    //also texture-buffer objects
    //This also affects bufferTexture objects!
    //NOT for glTex(Sub)Image*, glCopyTex(Sub)Image*, glClearTex*Image, glCompressedTex(Sub)Image* !
    void setTextureBarrier() {
        threadContext->memoryBarrierMask |= GL_TEXTURE_FETCH_BARRIER_BIT;
    }

    //read/write access after barrier
    void setImageBarrier() {
        threadContext->memoryBarrierMask |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
    }

    //needs GL_ARB_shader_storage_buffer_object (Core since 4.3) or throws error if glMemoryBarrier is called with it
    void setShaderStorageBufferBarrier() {
        threadContext->memoryBarrierMask |= GL_SHADER_STORAGE_BARRIER_BIT;
    }

    //the barrier is for buffers only, that are involved in copys from/to images
    void setBufferCopyToFromImageBarrier() {
        threadContext->memoryBarrierMask |= GL_PIXEL_BUFFER_BARRIER_BIT;
    }

    //also delete?; Only copy from/to other buffers?!
    void setBufferCreateClearCopyInvalidateBarrier() {
        threadContext->memoryBarrierMask |= GL_BUFFER_UPDATE_BARRIER_BIT;
    }

    //Barrier for textures that was or will be changed by:
    //glTex(Sub)Image*, glCopyTex(Sub)Image*, glClearTex*Image, glCompressedTex(Sub)Image*
    //TODO: also need this as a barrier in the image download function! (glGetTexImage)
    //not sure if glGetTexImage needs its own explecite barrier or if a previous barrier on a shader call is enough?!?!?!
    void setImageUploadDownloadClearBarrier() {
        threadContext->memoryBarrierMask |= GL_TEXTURE_UPDATE_BARRIER_BIT;
    }

    void setFrameBarrier() {
        threadContext->memoryBarrierMask |= GL_FRAMEBUFFER_BARRIER_BIT;
    }

    void setTransformFeedbackBarrier() {
        threadContext->memoryBarrierMask |= GL_TRANSFORM_FEEDBACK_BARRIER_BIT;
    }

    void setQueryBarrier() {
        threadContext->memoryBarrierMask |= GL_QUERY_BUFFER_BARRIER_BIT;
    }

    void setAtomicCounterBarrier() {
        threadContext->memoryBarrierMask |= GL_ATOMIC_COUNTER_BARRIER_BIT;
    }

    /*
        For server->client this one is needed for persistenly mapped buffers that do NOT have the MAP_COHERENT_BIT set and then waiting for a sync object (or glFinish).
        For client->server data without MAP_COHERENT_BIT one must use a FlushMapped*BufferRange command before issuing commands using the data changed by the client side.
    */
    void setMappedMemoryClientReadBarrier() {
        threadContext->memoryBarrierMask |= GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT;
    }

    void setAllBarrier() {
        threadContext->memoryBarrierMask |= GL_ALL_BARRIER_BITS;
    }

    //FragmentShaderOnly barriers
    void setUniformBufferBarrierFragmentShaderOnly() {
        threadContext->memoryBarrierMaskFragemtShaderOnly |= GL_UNIFORM_BARRIER_BIT;
    }

    void setTextureBarrierFragmentShaderOnly() {
        threadContext->memoryBarrierMaskFragemtShaderOnly |= GL_TEXTURE_FETCH_BARRIER_BIT;
    }

    void setImageBarrierFragmentShaderOnly() {
        threadContext->memoryBarrierMaskFragemtShaderOnly |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
    }

    void setShaderStorageBufferBarrierFragmentShaderOnly() {
        threadContext->memoryBarrierMaskFragemtShaderOnly |= GL_SHADER_STORAGE_BARRIER_BIT;
    }

    void setAtomicCounterBarrierFragmentShaderOnly() {
        threadContext->memoryBarrierMaskFragemtShaderOnly |= GL_ATOMIC_COUNTER_BARRIER_BIT;
    }

    void setFrameBarrierFragmentShaderOnly() {
        threadContext->memoryBarrierMaskFragemtShaderOnly |= GL_FRAMEBUFFER_BARRIER_BIT;
    }

    void setAllBarrierFragmentShaderOnly() {
        threadContext->memoryBarrierMaskFragemtShaderOnly |= GL_ALL_BARRIER_BITS;
    }
}
