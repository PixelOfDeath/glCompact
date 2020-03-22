#pragma once
#include "glCompact/SurfaceFormat.hpp"
#include "glCompact/MemorySurfaceFormat.hpp"

#include <glm/fwd.hpp>

namespace glCompact {
    class SurfaceInterface {
            friend class PipelineInterface;
            friend class SurfaceSelector;
            friend class Frame;
            friend class Context;
        public:
            void detachFromThreadContext();
            void free();

            //TODO: may swizzle from source to target?
            //negative size for inverting?
            void copyFromSurfaceComponents(const SurfaceInterface& srcSurface, uint32_t srcMipmapLevel, glm::ivec3 srcOffset, uint32_t dstMipmapLevel, glm::ivec3 dstOffset, glm::ivec3 size);
            void copyFromSurfaceMemory    (const SurfaceInterface& srcSurface, uint32_t srcMipmapLevel, glm::ivec3 srcOffset, uint32_t dstMipmapLevel, glm::ivec3 dstOffset, glm::ivec3 size);

            uint32_t getX()                 {return x;}
            uint32_t getY()                 {return y;}
            uint32_t getZ()                 {return z;}
            uint32_t getMipmapCount()       {return mipmapCount;}
            uint32_t getSamples()           {return samples;}
            SurfaceFormat getSurfaceFormat(){return surfaceFormat;}

            bool isLayered();
        protected:
            SurfaceInterface() = default;
            SurfaceInterface& operator=(SurfaceInterface&& surfaceInterface);

            uint32_t id          = 0;
            uint32_t target      = 0;

            uint32_t x           = 0;
            uint32_t y           = 0;
            uint32_t z           = 0; //in case of cube maps, ever cube maps is 6 layers. cubemap = 6; cubemaparray = 6 * arraySize
            uint32_t mipmapCount = 0;
            uint32_t samples     = 0; //Using 0 for NON-multisample targets. Anything else (including 1) means this is a multisample target!
            SurfaceFormat surfaceFormat;

            void bindTemporalFirstTime() const;
            void bindTemporal() const;
    };
}
