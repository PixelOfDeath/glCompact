#pragma once
#include "glCompact/SurfaceInterface.hpp"

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

namespace glCompact {
    class BufferInterface;
    class TextureInterface : public SurfaceInterface {
            friend class Frame;
            friend class TextureSelector;
        public:
            void copyConvertFromMemory(                                         const void* mem, uint32_t maxCopySizeGuard, MemorySurfaceFormat memorySurfaceFormat, uint32_t mipmapLevel, glm::ivec3 texOffset, glm::ivec3 texSize);
            void copyConvertToMemory  (                                               void* mem, uint32_t maxCopySizeGuard, MemorySurfaceFormat memorySurfaceFormat, uint32_t mipmapLevel, glm::ivec3 texOffset, glm::ivec3 texSize) const;
            void copyConvertFromBuffer(const BufferInterface& bufferInterface, uintptr_t offset, uint32_t maxCopySizeGuard, MemorySurfaceFormat memorySurfaceFormat, uint32_t mipmapLevel, glm::ivec3 texOffset, glm::ivec3 texSize);
            void copyConvertToBuffer  (      BufferInterface& bufferInterface, uintptr_t offset, uint32_t maxCopySizeGuard, MemorySurfaceFormat memorySurfaceFormat, uint32_t mipmapLevel, glm::ivec3 texOffset, glm::ivec3 texSize) const;

            void setMipmapBaseLevel(uint32_t level);
            uint32_t getMipmapBaseLevel() const {return mipmapBaseLevel;}
            void generateMipmaps();

            void clear(uint32_t mipmapLevel);
            void clear(uint32_t mipmapLevel, MemorySurfaceFormat memorySurfaceFormat, const BufferInterface* buffer, const void* ptr);
            void clear(uint32_t mipmapLevel, glm::ivec3 offset, glm::ivec3 size);
            void clear(uint32_t mipmapLevel, glm::ivec3 offset, glm::ivec3 size, MemorySurfaceFormat memorySurfaceFormat, const BufferInterface* buffer, const void* ptr);

            void invalidate();
            void invalidate(uint32_t mipmapLevel);
            void invalidate(uint32_t mipmapLevel, glm::ivec3 offset, glm::ivec3 size);

            glm::uvec3 getMipmapLevelSize(uint32_t mipmapLevel) const;
        protected:
            TextureInterface() = default;
            TextureInterface(           const TextureInterface&  textureInterface);
            TextureInterface(                 TextureInterface&& textureInterface);
            TextureInterface& operator=(const TextureInterface&  textureInterface);
            TextureInterface& operator=(      TextureInterface&& textureInterface);

            void create(int32_t target, SurfaceFormat surfaceFormat, glm::uvec3 newSize, bool mipmap, uint8_t samples);
            void createView(TextureInterface& srcImages, int32_t target, SurfaceFormat surfaceFormat, uint32_t firstMipmap, bool mipmap, uint32_t firstLayer, uint32_t layerCount);
        private:
            uint8_t mipmapBaseLevel = 0;

            void copyConvertFrom(const BufferInterface* bufferInterface, const void* offsetPointer, uint32_t maxCopySizeGuard, MemorySurfaceFormat memorySurfaceFormat, uint32_t mipmapLevel, glm::ivec3 texOffset, glm::ivec3 texSize);
            void copyConvertTo  (      BufferInterface* bufferInterface,       void* offsetPointer, uint32_t maxCopySizeGuard, MemorySurfaceFormat memorySurfaceFormat, uint32_t mipmapLevel, glm::ivec3 texOffset, glm::ivec3 texSize) const;

            void setTextureParameter(int32_t pname, int32_t param);
            void setTextureParameter(int32_t pname, float param);

            void clearCheck(uint32_t mipmapLevel);
    };
}
