#pragma once
#include "glCompact/SurfaceFormat.hpp"
#include "glCompact/MemorySurfaceFormat.hpp"

#include <glm/vec3.hpp>

namespace glCompact {
    class SurfaceInterface {
            friend class PipelineInterface;
            friend class SurfaceSelector;
            friend class Frame;
        public:
            void detachFromThreadContext();
            void free();

            //TODO: may swizzle from source to target?
            //negative size for inverting?
            void copyFromSurfaceComponents(const SurfaceInterface& srcSurface, uint32_t srcMipmapLevel, glm::ivec3 srcOffset, uint32_t dstMipmapLevel, glm::ivec3 dstOffset, glm::ivec3 size);
            void copyFromSurfaceMemory    (const SurfaceInterface& srcSurface, uint32_t srcMipmapLevel, glm::ivec3 srcOffset, uint32_t dstMipmapLevel, glm::ivec3 dstOffset, glm::ivec3 size);

            glm::uvec3 getSize() const {return size;}
            uint32_t getMipmapCount() const {return mipmapCount;}
            uint32_t getSamples() const {return samples;}
            SurfaceFormat getSurfaceFormat() const {return surfaceFormat;}

            bool isLayered() const;
        protected:
            SurfaceInterface() = default;
            SurfaceInterface& operator=(SurfaceInterface&& surfaceInterface);

            uint32_t id          = 0;
            uint32_t target      = 0;

            glm::uvec3 size; //size.z for cubemaps is 6 layers, and for cubemap arrays 6 * arraySize
            uint32_t mipmapCount = 0;
            uint32_t samples     = 0; //Using 0 for NON-multisample targets. Anything else (including 1) means this is a multisample target!
            SurfaceFormat surfaceFormat;

            void bindTemporalFirstTime() const;
            void bindTemporal() const;
    };
}
