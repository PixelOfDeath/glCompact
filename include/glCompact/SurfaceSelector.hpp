#pragma once
#include "glCompact/SurfaceInterface.hpp"

namespace glCompact {
    class SurfaceSelector {
            friend class Frame;
        public:
            SurfaceSelector();
            SurfaceSelector(SurfaceInterface& surface, uint32_t mipmapLevel = 0);
            SurfaceSelector(SurfaceInterface& surface, uint32_t mipmapLevel    , uint32_t layer);
            //TODO if we stay with non virtual interface class this can be specialized to only take layer parameter for texture classes with layers
        private:
            SurfaceInterface*const surface;
            const uint32_t mipmapLevel;
            const  int32_t layer;
    };
}
