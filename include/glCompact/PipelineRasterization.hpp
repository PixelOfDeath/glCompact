/*
    glCompact
    Copyright (C) 2019-2020 Frederik Uje vom Hofe

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#pragma once
#include "glCompact/PipelineInterface.hpp"
#include "glCompact/PipelineRasterizationStateChange.hpp"
#include "glCompact/FaceSelection.hpp"
#include "glCompact/CompareOperator.hpp"
#include "glCompact/StencilOperator.hpp"
#include "glCompact/BlendFactors.hpp"
#include "glCompact/BlendEquations.hpp"
#include "glCompact/AttributeLayout.hpp"
#include "glCompact/AttributeLayoutStates.hpp"
#include "glCompact/PrimitiveTopology.hpp"
#include "glCompact/IndexType.hpp"
#include "glCompact/Tribool.hpp"

#include <glm/vec4.hpp>

#include <string>
//#include <string_view>
#include <vector>
#include <array>

//#include <mutex>

namespace glCompact {
    class PipelineRasterization;
    class PipelineRasterization : public PipelineInterface {
        public:
            PipelineRasterization(
                const std::string& vertexString,
                const std::string& tessControlString,
                const std::string& tessEvalutionString,
                const std::string& geometryString,
                const std::string& fragmentString
            );
            PipelineRasterization(
                const std::string& path,
                const std::string& vertexFile,
                const std::string& tessControlFile,
                const std::string& tessEvalutionFile,
                const std::string& geometryFile,
                const std::string& fragmentFile
            );

            PipelineRasterization(PipelineRasterization&& pipelineRasterization)     = delete;
            PipelineRasterization(PipelineRasterization& shaderRaster)               = delete;
            PipelineRasterization& operator=(PipelineRasterization& shaderGraphics)  = delete;
            PipelineRasterization& operator=(PipelineRasterization&& shaderGraphics) = delete;
        public:
            void setVertexStageInputPrimitiveTopology(PrimitiveTopology primitiveTopology);

          //Triangle rotation and draw face
            void setFaceFrontClockwise(bool clockwise);
            void setFaceSideToDraw(FaceSelection faceSelection);

            void setAttributeLayout     (const AttributeLayout& attributeLayout);
            void setAttributeLayout     ();
            void setAttributeBuffer     (uint32_t slot, const BufferInterface& buffer, uintptr_t offset = 0);
            void setAttributeBuffer     (uint32_t slot);
            void setAttributeBuffer     ();
            void setAttributeIndexBuffer(IndexType indexType, const BufferInterface& buffer, uintptr_t offset = 0);
            void setAttributeIndexBuffer();

          //DEPTH
            void setDepthTest (CompareOperator compareOperator);
            void setDepthWrite(bool enabled);
            void setDepthBias (float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
            void setDepthRange(double near, double far);
            void setDepthClippingToClamping(bool enabled);

          //STENCIL
            void setStencilRefValue(FaceSelection faceSelection, int32_t refValue);
            void setStencilTest    (FaceSelection faceSelection, uint32_t readMask, CompareOperator compareOperator);
            void setStencilWrite   (FaceSelection faceSelection, uint32_t writeMask, StencilOperator stencilFailOperator, StencilOperator stencilPassDepthFailOperator, StencilOperator stencilPassDepthPassOrAbsentOperator);

          //RGBA
            void setRgbaWrite(               bool r, bool g, bool b, bool a);
            void setRgbaWrite(uint32_t slot, bool r, bool g, bool b, bool a);

          //RGBA BLEND
            void setRgbaBlendConst   (glm::vec4 rgba);
            void setRgbaBlend        (                   BlendFactorRgb srcFactorRgb, BlendFactorA srcFactorA, BlendEquation equationRgb, BlendEquation equationA, BlendFactorRgb dstFactorRgb, BlendFactorA dstFactorA);
            void setRgbaBlend        (uint32_t rgbaSlot, BlendFactorRgb srcFactorRgb, BlendFactorA srcFactorA, BlendEquation equationRgb, BlendEquation equationA, BlendFactorRgb dstFactorRgb, BlendFactorA dstFactorA);
            void setRgbaBlendDisabled(uint32_t rgbaSlot);
            void setRgbaBlendDisabled();

          //LOGIC OPERATION (deprecated in OpenGL ES 2.0, but still used in Vulkan!)
            //...

          //MULTI SAMPLE
            void setMultisample(bool enable);

            //its depricated but still usable, not insert this functionalites?
            //setLineWidth       (float width ); //default 1.0f
            //setLineAntialiasing(bool  enable); //default off

            void draw                    (uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,                       uint32_t firstInstance);
            void drawIndexed             (uint32_t indexCount,  uint32_t instanceCount, uint32_t firstIndex , int32_t vertexOffset, uint32_t firstInstance);
            void drawIndirect            (const BufferInterface& parameterBuffer, intptr_t parameterBufferOffset, uint32_t count, uint32_t stride = 16);
            void drawIndexedIndirect     (const BufferInterface& parameterbuffer, intptr_t parameterBufferOffset, uint32_t count, uint32_t stride = 20);
            void drawIndirectCount       (const BufferInterface& parameterbuffer, intptr_t parameterBufferOffset, const BufferInterface& countBuffer, intptr_t countBufferOffset, intptr_t maxDrawCount, uint32_t stride = 16);
            void drawIndexedIndirectCount(const BufferInterface& parameterbuffer, intptr_t parameterBufferOffset, const BufferInterface& countBuffer, intptr_t countBufferOffset, intptr_t maxDrawCount, uint32_t stride = 20);

        //information
            bool hasVertexStage       ()const{return hasShader[0];}
            bool hasTessControlStage  ()const{return hasShader[1];}
            bool hasTessEvalutionStage()const{return hasShader[2];}
            bool hasGeometryStage     ()const{return hasShader[3];}
            bool hasFragmentStage     ()const{return hasShader[4];}

        private:
            virtual std::string getPipelineIdentificationString();
            std::string getPipelineInformationQueryString();
        //LOADING
            bool loadStrings_(
                const std::string& stringVertex,
                const std::string& stringTessControl,
                const std::string& stringTessEvalution,
                const std::string& stringGeometry,
                const std::string& stringFragment
            );
            void setDefaultValues();

        //SHADER INFORMATION
            //Vertex info
            //int uppermostActiveLocation = -1;
            struct AttributeLocationInfo {
                uint32_t    type = 0; //0 = attribute location not in use; Maybe make custom class to hold all the sub info for each type?
                std::string name;
            } attributeLocationInfo[Config::MAX_ATTRIBUTES];

            void setAttributeLayoutThrow(const std::string& errorMessage);

            void collectInformation();

            bool hasShader[5] = {false};
            const bool loadedFromFiles = false;
            std::string fileName[5];

            //TODO: make this values public const accessable
            int32_t geometryMaxPrimitveOutput = 0;
            int32_t geometryInputType         = 0;
            int32_t geometryOutputType        = 0;


          //STATES
            void activate();

            PipelineRasterizationStateChange pipelineRasterizationStateChangePending;

            PrimitiveTopology vertexStageInputPrimitiveTopology = static_cast<PrimitiveTopology>(0xFFFFFFFF);

            //TRIANGLE ROTATION AND DRAW FACE
            bool          triangleFrontIsClockwiseRotation = false;
            FaceSelection faceToDraw                       = FaceSelection::frontAndBack;

            //STENCIL
            struct StencilTestState {
                int32_t         refValue        = 0;
                CompareOperator compareOperator = CompareOperator::disabled;
                uint32_t        readMask        = 0xFF;
            };
            StencilTestState stencilTestFront;
            StencilTestState stencilTestBack;

            struct StencilWriteState {
                uint32_t        writeMask                            = 0xFF;
                StencilOperator stencilFailOperator                  = StencilOperator::keep;
                StencilOperator stencilPassDepthFailOperator         = StencilOperator::keep;
                StencilOperator stencilPassDepthPassOrAbsentOperator = StencilOperator::keep;
            };
            StencilWriteState stencilWriteFront;
            StencilWriteState stencilWriteBack;
            bool stencilWriteFrontAndBackSame = true;

            //DEPTH
            CompareOperator depthCompareOperator    = CompareOperator::disabled;
            bool            depthWriteEnabled       = false;
            float           depthBiasConstantFactor = 0.0f;
            float           depthBiasClamp          = 0.0f;
            float           depthBiasSlopeFactor    = 0.0f;
            double          depthNearMapping        = 0.0;
            double          depthFarMapping         = 1.0;
            bool            depthClippingToClamping = false;

            //RGBA
            struct RgbaWriteMask {
                union {
                    uint8_t value = 0xF;
                    uint8_t r:1, g:1, b:1, a:1;
                };
            };
            RgbaWriteMask rgbaWriteMask[Config::MAX_RGBA_ATTACHMENTS];
            bool singleRgbaWriteMaskState = true;

            //RGBA BLEND
            Tribool   blendEnabledAny   = false;
            Tribool   blendEnabledAll   = false;
            Tribool   blendModesUniform = true;
            glm::vec4 blendConstRgba    = glm::vec4(0);
            bool blendEnabled[Config::MAX_RGBA_ATTACHMENTS] = {};
            BlendFactors   blendFactors  [Config::MAX_RGBA_ATTACHMENTS];
            BlendEquations blendEquations[Config::MAX_RGBA_ATTACHMENTS];

            //LOGIC OPERATION
            //...probbaly not going to implement this

            //Multi sample
            bool multiSample = true; //default enabled, if disabled fill all samples of a texel with the same value!


            //ATTRIBUTE LAYOUT STATES
            AttributeLayoutStates attributeLayoutStates;

            //BUFFER ATTRIBUTE
            uint32_t  buffer_attribute_id    [Config::MAX_ATTRIBUTES] = {};
            uintptr_t buffer_attribute_offset[Config::MAX_ATTRIBUTES] = {};

            //BUFFER ATTRIBUTE INDEX
            IndexType indexType                     = static_cast<IndexType>(0);
            uint32_t  buffer_attribute_index_id     = 0;
            uintptr_t buffer_attribute_index_offset = 0; //this is glCompact only thing. So it is not part of the state tracker

            void activateAttributeIndex(IndexType indexType);
            void deactivateAttributeIndex();

            void processPendingChanges();
            void processPendingChangesPipelineRasterization();
            void processPendingChangesAttributeLayoutAndBuffers();

            static const std::string shaderTypeString[];
            static const std::string shaderTypeStringSameLenght[];
    };
}
