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
#include "glCompact/SurfaceFormat.hpp"
#include "glCompact/MemorySurfaceFormat.hpp"

#include <glm/fwd.hpp>

namespace glCompact {
    class SurfaceInterface {
        ///\cond HIDDEN_FROM_DOXYGEN
            friend class PipelineInterface;
            friend class SurfaceSelector;
            friend class Frame;
        ///\endcond
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
        ///\cond HIDDEN_FROM_DOXYGEN
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

            void bindTemporalNonMultiBind() const;
            void bindTemporalFirstTime()    const;
            void bindTemporal()             const;
        ///\endcond
    };
}
