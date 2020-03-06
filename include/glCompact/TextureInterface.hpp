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
#include "glCompact/SurfaceInterface.hpp"
#include "glCompact/BufferInterface.hpp"

#include <glm/fwd.hpp>

namespace glCompact {
    class TextureInterface : public SurfaceInterface {
        ///\cond HIDDEN_FROM_DOXYGEN
            friend class Frame;
            friend class TextureSelector;
        ///\endcond
        public:
            TextureInterface& operator=(TextureInterface&& textureInterface) = default;

            /*TextureInterface(const TextureInterface& images);
            TextureInterface& operator=(const TextureInterface& images);
            TextureInterface(TextureInterface&& images);
            TextureInterface& operator=(TextureInterface&& images);*/

            void copyFromMemory(                                         const void* mem, uintptr_t bufSize, MemorySurfaceFormat memorySurfaceFormat, uint32_t dstMipmapLevel, glm::ivec3 dstOffset, glm::ivec3 size);
            void copyToMemory  (                                               void* mem, uintptr_t bufSize, MemorySurfaceFormat memorySurfaceFormat, uint32_t dstMipmapLevel, glm::ivec3 dstOffset, glm::ivec3 size);
            void copyFromBuffer(const BufferInterface& bufferInterface, uintptr_t offset, uintptr_t bufSize, MemorySurfaceFormat memorySurfaceFormat, uint32_t dstMipmapLevel, glm::ivec3 dstOffset, glm::ivec3 size);
            void copyToBuffer  (      BufferInterface& bufferInterface, uintptr_t offset, uintptr_t bufSize, MemorySurfaceFormat memorySurfaceFormat, uint32_t dstMipmapLevel, glm::ivec3 dstOffset, glm::ivec3 size);

            void setMipmapBaseLevel(uint32_t level);
            void generateMipmaps();

            void clear(uint32_t mipmapLevel);
            void clear(uint32_t mipmapLevel, MemorySurfaceFormat memorySurfaceFormat, const BufferInterface* buffer, const void* ptr);
            void clear(uint32_t mipmapLevel, glm::ivec3 offset, glm::ivec3 size);
            void clear(uint32_t mipmapLevel, glm::ivec3 offset, glm::ivec3 size, MemorySurfaceFormat memorySurfaceFormat, const BufferInterface* buffer, const void* ptr);

            void invalidate();
            void invalidate(uint32_t mipmapLevel);
            void invalidate(uint32_t mipmapLevel, glm::ivec3 offset, glm::ivec3 size);

            glm::ivec3 getMipmapLevelSize(uint32_t mipmapLevel);
        ///\cond HIDDEN_FROM_DOXYGEN
        protected:
            TextureInterface() = default;
            ~TextureInterface();
            void create(int32_t target, SurfaceFormat surfaceFormat, uint32_t x, uint32_t y, uint32_t z, bool mipmap, uint8_t samples);
            void createView(TextureInterface& srcImages, int32_t target, SurfaceFormat surfaceFormat, uint32_t firstMipmap, bool mipmap, uint32_t firstLayer, uint32_t layerCount);
        private:
            void copyFrom(const BufferInterface* bufferInterface, const void* offsetPointer, uintptr_t bufSize, MemorySurfaceFormat memorySurfaceFormat, uint32_t dstMipmapLevel, glm::ivec3 dstOffset, glm::ivec3 size);
            void copyTo  (      BufferInterface* bufferInterface,       void* offsetPointer, uintptr_t bufSize, MemorySurfaceFormat memorySurfaceFormat, uint32_t dstMipmapLevel, glm::ivec3 dstOffset, glm::ivec3 size);

            void setTextureParameter(int32_t pname, int32_t param);
            void setTextureParameter(int32_t pname, float param);

            void clearCheck(uint32_t mipmapLevel);

            uint8_t mipmapBaseLevel = 0;

            static void checkSurfaceFormatCompatibleToMemorySurfaceFormat(SurfaceFormat surfaceFormat, MemorySurfaceFormat memorySurfaceFormat);
        ///\endcond
    };
}
