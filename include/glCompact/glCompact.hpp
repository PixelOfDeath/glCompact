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
#include "glCompact/ContextScope.hpp"
#include "glCompact/GlTools.hpp"
#include "glCompact/Debug.hpp"
#include "glCompact/Buffer.hpp"
#include "glCompact/BufferStaging.hpp"
#include "glCompact/BufferSparse.hpp"
#include "glCompact/RenderBuffer2d.hpp"
#include "glCompact/RenderBuffer2dMultisample.hpp"
#include "glCompact/TextureInterface.hpp"
#include "glCompact/Texture1d.hpp"
#include "glCompact/Texture2d.hpp"
#include "glCompact/Texture2dArray.hpp"
#include "glCompact/Texture2dMultisample.hpp"
#include "glCompact/Texture2dMultisampleArray.hpp"
#include "glCompact/Texture3d.hpp"
#include "glCompact/TextureCubemap.hpp"
#include "glCompact/TextureCubemapArray.hpp"
#include "glCompact/Sampler.hpp"
#include "glCompact/AttributeLayout.hpp"
#include "glCompact/PipelineRasterization.hpp"
#include "glCompact/PipelineCompute.hpp"
#include "glCompact/Frame.hpp"
#include "glCompact/Sync.hpp"
#include "glCompact/MemoryBarrier.hpp"

namespace glCompact {
    //control/flush commands
    extern void flush();
    extern void finish();

    //Maybe make setCondition a part of objects, like PipelineInterface. Or make it an optional parameter for e.g. the frame clear functions.
    //This also would give a clean overview of what commands can be conditional.
    //extern void setCondition(GLuint oqoId, GLenum waitMode);
    //extern void setCondition();

    //global bindings...
    extern void   setDrawFrame      (Frame& frame);
    extern Frame& getDrawFrame      ();
    extern void   setWindowFrameSize(uint32_t x, uint32_t y);
    extern Frame& getWindowFrame    ();

    //transform feedback
    //If I implement this it maybe will be part of PipelineRasterization or its own object.
    //extern void setTransformFeedbackLayout(const TransformFeedbackLayout& layout);
    //extern void setTransformFeedbackBuffer(uint32_t slot, BufferInterface& buffer);
}
