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
#pragma once
#include "glCompact/SurfaceInterface.hpp"
#include "glCompact/Config.hpp"
#include "glCompact/SurfaceSelector.hpp"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace glCompact {
    class Frame;
    class Frame {
            friend class Context;
            friend class GraphicsPipeline;
            friend void setDrawFrame(Frame& frame);
            friend void setWindowFrameSize(uint32_t x, uint32_t y);
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
                  SurfaceSelector rgba7 = {}) noexcept(false);
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

            void setConvertSrgb(bool enabled);
            bool getConvertSrgb(){return convertSrgb;}

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

            void copyToMemory(uint32_t rgbaSlot, void* mem, uintptr_t bufSize, MemorySurfaceFormat memorySurfaceFormat, glm::ivec2 offset, glm::ivec2 size);

            void setRgbaDrawMapping(int32_t slot0, int32_t slot1 = -1, int32_t slot2 = -1, int32_t slot3 = -1, int32_t slot4 = -1, int32_t slot5 = -1, int32_t slot6 = -1, int32_t slot7 = -1);

            uint32_t getX(){return x;}
            uint32_t getY(){return y;}
            uint32_t getZ(){return z;} //min amount of layers from all targets
            uint32_t getSamples(){return samples;}
            bool     isLayered(){return layered;}
            bool     isSrgb(){return srgb;}

            //TODO: And ref of active rgba/depth/stencil attachments?
        protected:
            uint32_t id      = 0;

            uint32_t x       = 0;
            uint32_t y       = 0;
            uint32_t z       = 0;
            uint32_t samples = 0;
            bool     layered = false;
            bool     srgb    = false;

            bool     convertSrgb = false;

            enum AttachmentDataType : uint8_t {
                unused,
                normalizedOrFloat,
                unsignedInteger,
                signedInteger
            };
            AttachmentDataType rgbaAttachmentDataType[Config::MAX_RGBA_ATTACHMENTS] = {};
            AttachmentDataType depthAttachmentDataType   = AttachmentDataType::unused;
            AttachmentDataType stencilAttachmentDataType = AttachmentDataType::unused;

            bool scissorEnabled = false;
            glm::uvec2 viewportOffset;
            glm::uvec2 viewportSize;
            glm::uvec2 scissorOffset;
            glm::uvec2 scissorSize;

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

            void detachFromThreadContextStateCurrent() const;
            void detachFromThreadContextState() const;

            void setDefaultValues();
    };
}
