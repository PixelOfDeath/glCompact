#pragma once
#include "glCompact/SurfaceInterface.hpp"
#include "glCompact/BufferInterface.hpp"

#include <glm/fwd.hpp>

namespace glCompact {
    class TextureInterface : public SurfaceInterface {
            friend class Frame;
            friend class TextureSelector;
        public:
            TextureInterface& operator=(TextureInterface&& textureInterface) = default;

            /*TextureInterface(const TextureInterface& images);
            TextureInterface& operator=(const TextureInterface& images);
            TextureInterface(TextureInterface&& images);
            TextureInterface& operator=(TextureInterface&& images);*/

            void copyFromMemory(                                         const void* mem, uint32_t maxCopySizeGuard, MemorySurfaceFormat memorySurfaceFormat, uint32_t mipmapLevel, glm::ivec3 texOffset, glm::ivec3 texSize);
            void copyToMemory  (                                               void* mem, uint32_t maxCopySizeGuard, MemorySurfaceFormat memorySurfaceFormat, uint32_t mipmapLevel, glm::ivec3 texOffset, glm::ivec3 texSize);
            void copyFromBuffer(const BufferInterface& bufferInterface, uintptr_t offset, uint32_t maxCopySizeGuard, MemorySurfaceFormat memorySurfaceFormat, uint32_t mipmapLevel, glm::ivec3 texOffset, glm::ivec3 texSize);
            void copyToBuffer  (      BufferInterface& bufferInterface, uintptr_t offset, uint32_t maxCopySizeGuard, MemorySurfaceFormat memorySurfaceFormat, uint32_t mipmapLevel, glm::ivec3 texOffset, glm::ivec3 texSize);

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
        protected:
            TextureInterface() = default;
            ~TextureInterface();
            void create(int32_t target, SurfaceFormat surfaceFormat, uint32_t x, uint32_t y, uint32_t z, bool mipmap, uint8_t samples);
            void createView(TextureInterface& srcImages, int32_t target, SurfaceFormat surfaceFormat, uint32_t firstMipmap, bool mipmap, uint32_t firstLayer, uint32_t layerCount);
        private:
            void copyFrom(const BufferInterface* bufferInterface, const void* offsetPointer, uint32_t maxCopySizeGuard, MemorySurfaceFormat memorySurfaceFormat, uint32_t mipmapLevel, glm::ivec3 texOffset, glm::ivec3 texSize);
            void copyTo  (      BufferInterface* bufferInterface,       void* offsetPointer, uint32_t maxCopySizeGuard, MemorySurfaceFormat memorySurfaceFormat, uint32_t mipmapLevel, glm::ivec3 texOffset, glm::ivec3 texSize);

            void setTextureParameter(int32_t pname, int32_t param);
            void setTextureParameter(int32_t pname, float param);

            void clearCheck(uint32_t mipmapLevel);

            uint8_t mipmapBaseLevel = 0;

            static void checkSurfaceFormatCompatibleToMemorySurfaceFormat(SurfaceFormat surfaceFormat, MemorySurfaceFormat memorySurfaceFormat);
    };
}
