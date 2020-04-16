#pragma once
#include "glCompact/config.hpp"
#include "glCompact/AttributeLayoutStates.hpp"
#include "glCompact/IndexType.hpp"
#include "glCompact/Frame.hpp"
#include "glCompact/PipelineRasterizationStateChange.hpp"
#include "glCompact/FaceSelection.hpp"
#include "glCompact/CompareOperator.hpp"
#include "glCompact/StencilOperator.hpp"
#include "glCompact/BlendFactors.hpp"
#include "glCompact/BlendEquations.hpp"

#include <glm/fwd.hpp>

#include <string>
#include <vector>

namespace glCompact {
    class Debug;
    class PipelineInterface;
    class PipelineRasterization;
    class PipelineCompute;
    class BufferInterface;
    class Buffer;
    class BufferStaging;
    class BufferSparse;
    class TextureInterface;
    class Sampler;
    class Frame;

    class Context;
    class Context {
            friend class ContextScope;
            friend class Debug;
            friend class PipelineInterface;
            friend class PipelineRasterization;
            friend class PipelineCompute;
            friend class BufferInterface;
            friend class Buffer;
            friend class BufferStaging;
            friend class BufferSparse;
            friend class SurfaceInterface;
            friend class TextureInterface;
            friend class Sampler;
            friend class Frame;

            friend void flush();
            friend void finish();

            friend void   setDrawFrame(Frame& frame);
            friend void   setDrawFrame();
            friend Frame& getDrawFrame();
            friend void   setWindowFrameSize(uint32_t x, uint32_t y);
            friend Frame& getWindowFrame();

            //Memory Barrier commands
            friend class MemoryBarrier;
        public:
            Context();
            //prevent  copy constructor/Assignment operators
            Context(const Context &c)          = delete;
            Context& operator=(const Context&) = delete;
            //prevent  Move constructors/Move assignment (C++11).
            Context(Context&&)                 = delete;
            Context& operator=(Context&&)      = delete;
            ~Context();

            uint32_t getContextId() const;
            //if an external gl library is used this functions will set some GL states back to default
            void defaultStatesActivate();
            void defaultStatesDeactivate();
        private:
            static void assertThreadHasActiveGlContext();

            uint32_t contextId;
            //Relevant for e.g. access to the frameWindow
            bool isMainContext = false;

            //FRAME
            Frame frameWindow;
            Frame* pending_frame = 0;
            Frame* current_frame = 0;

            uint32_t pending_frame_drawId = 0;
          //uint32_t pending_frame_readId = 0;
            uint32_t current_frame_drawId = 0;
            uint32_t current_frame_readId = 0;
            bool current_scissor_enabled = false;
            bool current_convertSrgb = false;

            glm::uvec2 current_viewportOffset;
            glm::uvec2 current_viewportSize;
            glm::uvec2 current_scissorOffset;
            glm::uvec2 current_scissorSize;

            //SHADER
            uint32_t           pipelineShaderId = 0;
            PipelineInterface* pipeline = 0;
            PipelineInterface* pipelineThatCausedLastWarning = 0; //using this to only print out the pipeline information/identification once until a different pipeline causes warnings

            uint32_t  defaultVaoId = 0;
            uint32_t  boundArrayBuffer = 0;
            //ATTRIBUTE LAYOUT STATES
            bool      attributeLayoutChanged = false; //TODO: make this one of the state bits
            AttributeLayoutStates attributeLayoutStates;

            //BUFFER ATTRIBUTE
             int8_t   buffer_attribute_getHighestNonNull();
             int8_t   buffer_attribute_maxHighestNonNull = -1;
            uint32_t  buffer_attribute_id    [config::MAX_ATTRIBUTES] = {};
            uintptr_t buffer_attribute_offset[config::MAX_ATTRIBUTES] = {};

            //BUFFER INDEX
            bool      buffer_attribute_index_enabled = 0;
            IndexType buffer_attribute_index_type    = static_cast<IndexType>(0);
            uint32_t  buffer_attribute_index_id      = 0;

            //BUFFER INDIRECT
            uint32_t  buffer_draw_indirect_id     = 0;
            uint32_t  buffer_dispatch_indirect_id = 0;
            uint32_t  buffer_parameter_id         = 0;

            //BUFFER UNIFORM
             int32_t  buffer_uniform_getHighestNonNull();
             int32_t  buffer_uniform_maxHighestNonNull = -1;
            uint32_t  buffer_uniform_id    [config::MAX_UNIFORM_BUFFER_BINDINGS] = {};
            uintptr_t buffer_uniform_offset[config::MAX_UNIFORM_BUFFER_BINDINGS] = {};
            uintptr_t buffer_uniform_size  [config::MAX_UNIFORM_BUFFER_BINDINGS] = {};

            //BUFFER ATOMIC COUNTER
             int32_t  buffer_atomicCounter_getHighestNonNull();
             int32_t  buffer_atomicCounter_maxHighestNonNull = -1;
            uint32_t  buffer_atomicCounter_id    [config::MAX_ATOMIC_COUNTER_BUFFER_BINDINGS] = {};
            uintptr_t buffer_atomicCounter_offset[config::MAX_ATOMIC_COUNTER_BUFFER_BINDINGS] = {};
            uintptr_t buffer_atomicCounter_size  [config::MAX_ATOMIC_COUNTER_BUFFER_BINDINGS] = {};

            //BUFFER SHADER STORAGE
             int32_t  buffer_shaderStorage_getHighestNonNull();
             int32_t  buffer_shaderStorage_maxHighestNonNull = -1;
            uint32_t  buffer_shaderStorage_id    [config::MAX_SHADERSTORAGE_BUFFER_BINDINGS] = {};
            uintptr_t buffer_shaderStorage_offset[config::MAX_SHADERSTORAGE_BUFFER_BINDINGS] = {};
            uintptr_t buffer_shaderStorage_size  [config::MAX_SHADERSTORAGE_BUFFER_BINDINGS] = {};

            //PIXEL PACK BUFFERS (INTERNAL ONLY)
            uint32_t  buffer_pixelPackId   = 0;
            uint32_t  buffer_pixelUnpackId = 0;

            //COPY READ/WRITE BUFFERS (INTERNAL ONLY)
            uint32_t  buffer_copyReadId  = 0;
            uint32_t  buffer_copyWriteId = 0;

            //TEXTURE
            uint32_t  activeTextureSlot = 0; //caching of "GL_TEXTURE0 + i" value for old style binding
             int32_t  texture_getHighestNonNull();
             int32_t  texture_maxHighestNonNull = -1;
            uint32_t  texture_id    [config::MAX_SAMPLER_BINDINGS] = {};
             int32_t  texture_target[config::MAX_SAMPLER_BINDINGS] = {};

            //SAMPLER
             int32_t  sampler_getHighestNonNull();
             int32_t  sampler_maxHighestNonNull = -1;
            uint32_t  sampler_id[config::MAX_SAMPLER_BINDINGS] = {};

            //IMAGE
             int32_t  image_getHighestNonNull();
             int32_t  image_maxHighestNonNull = -1;
            uint32_t  image_id         [config::MAX_IMAGE_BINDINGS] = {};
            uint32_t  image_format     [config::MAX_IMAGE_BINDINGS] = {};
            uint32_t  image_mipmapLevel[config::MAX_IMAGE_BINDINGS] = {};
             int32_t  image_layer      [config::MAX_IMAGE_BINDINGS] = {};

            //Graphics pipeline state
            PipelineRasterizationStateChange pipelineRasterizationStateChangePending;

            //TRIANGLE ROTATION AND DRAW FACE
            bool          triangleFrontIsClockwiseRotation = false;
            FaceSelection faceToDraw                       = FaceSelection::frontAndBack;

            //STENCIL
            bool stencilEnabled = false;

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
            bool            depthEnabled            = false;
            CompareOperator depthCompareOperator    = CompareOperator::less;
            bool            depthWriteEnabled       = true;
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
            RgbaWriteMask rgbaWriteMask[config::MAX_RGBA_ATTACHMENTS];
            bool singleRgbaWriteMaskState = true;

            //RGBA BLEND
            bool      blendEnabledAny   = false;
            bool      blendModesUniform = true;
            glm::vec4 blendConstRgba    = glm::vec4(0);
            bool blendEnabled[config::MAX_RGBA_ATTACHMENTS] = {};
            BlendFactors   blendFactors  [config::MAX_RGBA_ATTACHMENTS];
            BlendEquations blendEquations[config::MAX_RGBA_ATTACHMENTS];

            //LOGIC OPERATION
            //...probbaly not going to implement this

            //Multi sample
            bool multiSample = true; //default enabled, if disabled fill all samples of a texel with the same value!


            //BARRIER
            uint32_t memoryBarrierMask = 0;
            uint32_t memoryBarrierRasterizationRegionMask = 0;

            //bool current_depthOffsetEnabled;
            float current_depthMultiplicator   = 0.0f;
            float current_addMinimumDepthUnits = 0.0f;

            //helper
            void cachedBindTextureCompatibleOrFirstTime(uint32_t texSlot, int32_t texTarget, uint32_t texId);
            void cachedBindTexture                     (uint32_t texSlot, int32_t texTarget, uint32_t texId);

            void cachedBindShader          (uint32_t pipelineShaderId);
            void cachedBindArrayBuffer     (uint32_t bufferId);
            void cachedSetActiveTextureUnit(uint32_t slot);
            void cachedBindDrawFbo         (uint32_t fboId);
            void cachedBindReadFbo         (uint32_t fboId);
            void cachedConvertSrgb         (bool enabled);
            void cachedViewport            (glm::uvec2 offset, glm::uvec2 size);
            void cachedScissorEnabled      (bool enabled);
            void cachedScissor             (glm::uvec2 offset, glm::uvec2 size);

            void cachedBindPixelPackBuffer  (uint32_t bufferId);
            void cachedBindPixelUnpackBuffer(uint32_t bufferId);
            void cachedBindCopyReadBuffer   (uint32_t bufferId);
            void cachedBindCopyWriteBuffer  (uint32_t bufferId);

          //different kind of parameter buffers
            void cachedBindDrawIndirectBuffer    (uint32_t bufferId);
            void cachedBindDispatchIndirectBuffer(uint32_t bufferId);
            void cachedBindParameterBuffer       (uint32_t bufferId);

          //processPending
            //we need this also outside of GraphicsPipeline
            void processPendingChangesDrawFrame();
            void processPendingChangesDrawFrame(Frame* pendingFrame);
            void processPendingChangesMemoryBarriers();
            //TODO: move this to the graphics shader?
            void processPendingChangesMemoryBarriersRasterizationRegion();
    };
}
