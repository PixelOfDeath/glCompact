#pragma once

namespace glCompact {
    //read access barriers
    extern void setAttributeBufferBarrier();
    extern void setAttributeIndexBufferBarrier();
    extern void setParameterBufferBarrier();
    extern void setUniformBufferBarrier();
    extern void setTextureBarrier();
    //read/write access barriers
    extern void setImageBarrier();
    extern void setShaderStorageBufferBarrier();
    extern void setBufferCopyToFromImageBarrier();
    extern void setBufferCreateClearCopyInvalidateBarrier();
    extern void setImageUploadDownloadClearBarrier();
    extern void setFrameBarrier();
    extern void setTransformFeedbackBarrier();
    extern void setQueryBarrier();
    extern void setAtomicCounterBarrier();
    extern void setMappedMemoryClientReadBarrier();
    extern void setAllBarrier();

    //void setFrameTextureBarrier(); //void glTextureBarrier(void); NV_texture_barrier or GL_ARB_texture_barrier (Core since 4.5), barrier to sample from a texture that is still bound as the current frame render target

    //only for fragment shader read/writes (optimisation for tilled hardware only?)
    extern void setUniformBufferBarrierFragmentShaderOnly();
    extern void setTextureBarrierFragmentShaderOnly();
    extern void setImageBarrierFragmentShaderOnly();
    extern void setShaderStorageBufferBarrierFragmentShaderOnly();
    extern void setAtomicCounterBarrierFragmentShaderOnly();
    extern void setFrameBarrierFragmentShaderOnly();
    extern void setAllBarrierFragmentShaderOnly();
}
