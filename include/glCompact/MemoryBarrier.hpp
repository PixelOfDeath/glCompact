/*
    This file is part of glCompact.
    Copyright (C) 2019-2020 Frederik Uje vom Hofe

    glCompact is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    glCompact is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
*/
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
