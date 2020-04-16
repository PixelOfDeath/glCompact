#pragma once
#include "glCompact/config.hpp"
#include "glCompact/AttributeFormat.hpp"

namespace glCompact {
    class AttributeLayout {
            friend class PipelineRasterization;
        public:
            AttributeLayout();
            ~AttributeLayout();

            void addBufferIndex();
            void addBufferIndexWithInstancing();
            void addLocation(uint8_t location, AttributeFormat attributeFormat);
            void addSpacing(uint16_t byteSize);
            void addSpacing(AttributeFormat attributeFormat);

            void reset();
        private:
            int8_t uppermostActiveBufferIndex = -1;
            int8_t uppermostActiveLocation    = -1;

            struct BufferIndex {
                uint16_t stride     = 0;
                bool     instancing = 0;
            } bufferIndex[config::MAX_ATTRIBUTES];
            struct Location {
                AttributeFormat attributeFormat = AttributeFormat::NONE; //AttributeFormat::none = location disabled
                uint16_t        offset          = 0;
                uint8_t         bufferIndex     = 0;
            } location[config::MAX_ATTRIBUTES];

            void addBufferIndex_(bool instancing);
    };
}
