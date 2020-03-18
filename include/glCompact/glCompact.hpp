#pragma once
#include "glCompact/ContextScope.hpp"
#include "glCompact/GlTools.hpp"
#include "glCompact/Debug.hpp"
#include "glCompact/Buffer.hpp"
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
#include "glCompact/Fence.hpp"
#include "glCompact/MemoryBarrier.hpp"

namespace glCompact {
    extern void flush();
    extern void finish();

    extern void   setDrawFrame      (Frame& frame);
    extern Frame& getDrawFrame      ();
    extern void   setWindowFrameSize(uint32_t x, uint32_t y);
    extern Frame& getWindowFrame    ();
}
