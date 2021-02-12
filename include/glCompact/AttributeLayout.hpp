#pragma once
#include "glCompact/config.hpp"
#include "glCompact/AttributeFormat.hpp"

namespace glCompact {
    class AttributeLayout {
            friend class PipelineRasterization;
        public:
            void addBufferIndex();
            void addBufferIndexWithInstancing();
            void addLocation(uint8_t location, AttributeFormat attributeFormat);
            void addSpacing(uint16_t byteSize);
            void addSpacing(AttributeFormat attributeFormat);

            void reset();
        protected:
            int8_t uppermostActiveBufferIndex = -1;
            int8_t uppermostActiveLocation    = -1;

            uint32_t        bufferIndexStride       [config::MAX_ATTRIBUTES] = {}; //Must be uint32_t because this is an ABI parameter list for OpenGL
            bool            bufferIndexInstancing   [config::MAX_ATTRIBUTES] = {};
            AttributeFormat locationAttributeFormat [config::MAX_ATTRIBUTES] = {};
            uint16_t        locationOffset          [config::MAX_ATTRIBUTES] = {};
            uint8_t         locationBufferIndex     [config::MAX_ATTRIBUTES] = {};

            void addBufferIndex_(bool instancing);

            bool operator==(const AttributeLayout& rhs) const;
            bool operator!=(const AttributeLayout& rhs) const;
    };
}
