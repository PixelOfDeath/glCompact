#pragma once
#include "glCompact/TextureInterface.hpp"

namespace glCompact {
    class TextureSelector {
            friend class Frame;
            friend class PipelineInterface;
        public:
            //TODO check for valid input param
            TextureSelector();
            TextureSelector(TextureInterface& texture, uint32_t mipmapLevel = 0);
            TextureSelector(TextureInterface& texture, uint32_t mipmapLevel    , uint32_t layer);
        private:
            TextureInterface*const texture;
            const uint32_t mipmapLevel;
            const uint32_t layer;
    };
}
