#pragma once
#include "glCompact/config.hpp"
#include "glCompact/SurfaceInterface.hpp"
#include "glCompact/SurfaceSelector.hpp"
#include "glCompact/BufferInterface.hpp"
#include "glCompact/SurfaceFormat.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace glCompact {
    class Frame;
    class Frame {
            friend class Context;
            friend class Context_;
            friend void setDrawFrame(Frame& frame);
            friend void setDisplayFrameSize(uint32_t x, uint32_t y);
        public:
            Frame() = default;
            Frame(SurfaceSelector depthAndOrStencilSurface,
                  SurfaceSelector rgba0,
                  SurfaceSelector rgba1 = {},
                  SurfaceSelector rgba2 = {},
                  SurfaceSelector rgba3 = {},
                  SurfaceSelector rgba4 = {},
                  SurfaceSelector rgba5 = {},
                  SurfaceSelector rgba6 = {},
                  SurfaceSelector rgba7 = {});
            Frame(uint32_t sizeX, uint32_t sizeY, uint32_t layers, uint32_t samples);
            //Frame           (const Frame& frame);
            //Frame& operator=(const Frame& frame);
            Frame           (Frame&& frame);
            Frame& operator=(Frame&& frame);
            ~Frame();
            void free();

            void setViewport(glm::uvec2 offset, glm::uvec2 size);
            void setScissor (glm::uvec2 offset, glm::uvec2 size);
            void setViewport();
            void setScissor ();

            //By default access to sRGB targets gets automatically converted between sRGB and linear color space.
            //Note that this has has no influence on sampling from sRGB textures.
            //TODO: GLES always converts?!
            bool srgbTargetsReadWriteLinear = true;

            //TODO: negative size to flip axis?
            //TODO: allow negative offsets?
            void blitRgba        (uint32_t slot);
            void blitRgba        (uint32_t slot, glm::uvec2 srcOffset, glm::uvec2 dstOffset);
            void blitRgba        (uint32_t slot, glm::uvec2 srcOffset, glm::uvec2 dstOffset, glm::ivec2    size);
            void blitRgba        (uint32_t slot, glm::uvec2 srcOffset, glm::uvec2 dstOffset, glm::ivec2 srcSize, glm::ivec2 dstSize, bool filterLinear = true);
            void blitDepth       ();
            void blitDepth       (               glm::uvec2 srcOffset, glm::uvec2 dstOffset);
            void blitDepth       (               glm::uvec2 srcOffset, glm::uvec2 dstOffset, glm::ivec2    size);
            void blitStencil     ();
            void blitStencil     (               glm::uvec2 srcOffset, glm::uvec2 dstOffset);
            void blitStencil     (               glm::uvec2 srcOffset, glm::uvec2 dstOffset, glm::ivec2    size);
            void blitDepthStencil();
            void blitDepthStencil(               glm::uvec2 srcOffset, glm::uvec2 dstOffset);
            void blitDepthStencil(               glm::uvec2 srcOffset, glm::uvec2 dstOffset, glm::ivec2    size);

            void clearRgba        ();
            void clearRgba        (uint32_t slot, glm::vec4  rgba = glm::vec4(0));
            void clearRgba        (uint32_t slot, glm::uvec4 rgba);
            void clearRgba        (uint32_t slot, glm::ivec4 rgba);
            void clearDepth       (float depth);
            void clearStencil     (             uint32_t stencil);
            void clearDepthStencil(float depth, uint32_t stencil);

            void invalidateRgba();
            void invalidateRgba(uint32_t slot);
            void invalidateDepth();
            void invalidateStencil();
            void invalidateDepthStencil();
            void invalidate();

            void copyConvertRgbaToMemory        (uint32_t rgbaSlot,                                   void*     mem         , uintptr_t sizeGuard, MemorySurfaceFormat memorySurfaceFormat, glm::ivec2 offset, glm::ivec2 size);
            void copyConvertDepthToMemory       (                                                     void*     mem         , uintptr_t sizeGuard, MemorySurfaceFormat memorySurfaceFormat, glm::ivec2 offset, glm::ivec2 size);
            void copyConvertStencilToMemory     (                                                     void*     mem         , uintptr_t sizeGuard, MemorySurfaceFormat memorySurfaceFormat, glm::ivec2 offset, glm::ivec2 size);
            void copyConvertDepthStencilToMemory(                                                     void*     mem         , uintptr_t sizeGuard, MemorySurfaceFormat memorySurfaceFormat, glm::ivec2 offset, glm::ivec2 size);

            void copyConvertRgbaToBuffer        (uint32_t rgbaSlot, BufferInterface& bufferInterface, uintptr_t bufferOffset, uintptr_t sizeGuard, MemorySurfaceFormat memorySurfaceFormat, glm::ivec2 offset, glm::ivec2 size);
            void copyConvertDepthToBuffer       (                   BufferInterface& bufferInterface, uintptr_t bufferOffset, uintptr_t sizeGuard, MemorySurfaceFormat memorySurfaceFormat, glm::ivec2 offset, glm::ivec2 size);
            void copyConvertStencilToBuffer     (                   BufferInterface& bufferInterface, uintptr_t bufferOffset, uintptr_t sizeGuard, MemorySurfaceFormat memorySurfaceFormat, glm::ivec2 offset, glm::ivec2 size);
            void copyConvertDepthStencilToBuffer(                   BufferInterface& bufferInterface, uintptr_t bufferOffset, uintptr_t sizeGuard, MemorySurfaceFormat memorySurfaceFormat, glm::ivec2 offset, glm::ivec2 size);

            glm::uvec3 getSize()            const {return size;}
            uint32_t   getRgbaTargetCount() const {return rgbaTargetCount;}
            uint32_t   getSamples()         const {return samples;}
            bool       isLayered()          const {return layered;}
            bool       isDisplayFrame()     const;

            //TODO: And ref of active rgba/depth/stencil attachments?
        protected:
            uint32_t    id              = 0;
            glm::uvec3  size            = {0, 0, 0};
            uint32_t    rgbaTargetCount = 0;
            uint32_t    samples         = 0;
            bool        layered         = false;
            bool        scissorEnabled  = false;
            glm::uvec2  viewportOffset  = {0, 0};
            glm::uvec2  viewportSize    = {0, 0};
            glm::uvec2  scissorOffset   = {0, 0};
            glm::uvec2  scissorSize     = {0, 0};
            uint8_t currentRgbaReadSlot = 0;

            SurfaceFormat depthAndOrStencilSurfaceFormat;
            SurfaceFormat rgbaSurfaceFormat[config::MAX_RGBA_ATTACHMENTS];

            static bool isSingleLayer(SurfaceSelector sel);
            static bool isMultiLayer (SurfaceSelector sel);

            void setAttachment                 (SurfaceSelector sel, int32_t attachmentType);
            void setDepthAndOrStencilAttachment(SurfaceSelector sel);
            void setRgbaAttachment             (SurfaceSelector sel, uint32_t rgbaSlot);

            void blit(int32_t mask, uint32_t srcRgbaSlot, uint32_t dstFboId, glm::uvec2 srcOffset, glm::uvec2 dstOffset, glm::ivec2 srcSize, glm::ivec2 dstSize, bool filterLinear);

            void clearRgbaNormalizedOrFloat(uint32_t slot, glm::vec4  rgba);
            void clearRgbaUnsigned         (uint32_t slot, glm::uvec4 rgba);
            void clearRgbaSigned           (uint32_t slot, glm::ivec4 rgba);

            void invalidate(int32_t attachment);

            void detachPtrFromThreadContextState() const;
            void detachFromThreadContextState() const;

            void copyConvertTo(bool isDepth, bool isStencil, bool isRgba, uint32_t rgbaSlot, BufferInterface* bufferInterface, void* mem, uintptr_t sizeGuard, MemorySurfaceFormat memorySurfaceFormat, glm::ivec2 offset, glm::ivec2 size);

            void setDefaultValues();
    };
}
