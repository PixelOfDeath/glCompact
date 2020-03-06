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
#include "glCompact/PipelineInterface.hpp"

#include <glm/vec3.hpp>

#include <string>

namespace glCompact {
    class PipelineCompute;
    class PipelineCompute : public PipelineInterface {
            friend class PipelineComputeFromString;
            friend class PipelineComputeFromFile;
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
