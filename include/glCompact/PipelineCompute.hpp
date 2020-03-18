#pragma once
#include "glCompact/PipelineInterface.hpp"

#include <glm/vec3.hpp>

#include <string>

namespace glCompact {
    class PipelineCompute;
    class PipelineCompute : public PipelineInterface {
        public:
            PipelineCompute(const std::string& shaderString);
            PipelineCompute(const std::string& path, const std::string& fileName);

            void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
            void dispatchIndirect(const BufferInterface& buffer, uintptr_t offset);

            glm::ivec3 getWorkGroupSize()const{return workGroupSize;}
        protected:
            virtual std::string getPipelineIdentificationString();
            std::string getPipelineInformationQueryString();
        private:
            glm::ivec3 workGroupSize;
            void loadString_(const std::string& computeShaderString);
            void collectInformation();
            const bool loadedFromFile = false;
            std::string fileName;
            void activate();

            void processPendingChanges();
    };
}
