#include "glCompact/PipelineRasterization.hpp"
#include "glCompact/Context_.hpp"
#include "glCompact/threadContext_.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/threadContextGroup_.hpp"
#include "glCompact/config.hpp"
#include "glCompact/BufferGpu.hpp"

#include "glCompact/Tools_.hpp"
#include "glCompact/gl/Helper.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <algorithm>

///\cond HIDDEN_FROM_DOXYGEN
/*
    Seperate shader parts that can be mixed at runtime is an anti patern from Direct3D! Not going to implement it. (ARB_separate_shader_objects)

    We need ARB_explicit_attrib_location (core since 3.3) for setting the location of the vertex layouts in the vertex program string:
    layout (location = 0) in vec3 pos;

    GL_MAX_TEXTURE_IMAGE_UNITS          (getFragmentMaxTextures)
    GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS (getStagesCombinedMaxTextures)

    glValidateProgram <- does this give any usefull info/debug output?
*/
///\endcond

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    /** \brief loads Graphics GLSL shader from strings
     * NOTE: They syntax PipelineRasterization myPipeline = glCompact::PipelineRasterizationFromStrings(... does not work with C++11 or lower
     * Use PipelineRasterization myPipeline(PipelineRasterizationFromStrings(...));
     */
    PipelineRasterization::PipelineRasterization(
        PrimitiveTopology  primitiveTopology,
        const std::string& vertexString,
        const std::string& tessControlString,
        const std::string& tessEvalutionString,
        const std::string& geometryString,
        const std::string& fragmentString
    ) {
        UNLIKELY_IF (!loadStrings_(vertexString, tessControlString, tessEvalutionString, geometryString, fragmentString))
            throw std::runtime_error(infoLog_);
        vertexStageInputPrimitiveTopology = primitiveTopology;
    }

    //TODO: one of the only instances where I depend on catch to work (blocks people form changing throw into a simple crash)
    /** \brief loads Graphics GLSL shader from files
     * NOTE: They syntax PipelineRasterization myPipeline = glCompact::PipelineRasterizationFromFiles(... does not work with C++11 or lower
     * Use PipelineRasterization myPipeline(PipelineRasterizationFromFiles(...));
     */
    PipelineRasterization::PipelineRasterization(
        const std::string& path,
        PrimitiveTopology  primitiveTopology,
        const std::string& vertexFile,
        const std::string& tessControlFile,
        const std::string& tessEvalutionFile,
        const std::string& geometryFile,
        const std::string& fragmentFile
    ):
        loadedFromFiles(true)
    {
        string fileName[5] = {
            vertexFile,
            tessControlFile,
            tessEvalutionFile,
            geometryFile,
            fragmentFile
        };
        string fileContent[5];
        //string shaderFilesList;
        LOOPI(5) {
            if (!fileName[i].empty()) {
                string fullFileName = path + "/" + fileName[i];
                this->fileName[i] = fullFileName;
                try {
                    fileContent[i] = fileToString(fullFileName);
                } catch (const std::exception& e) {
                    throw std::runtime_error("Can not read " + shaderTypeString[i] + " shader file \"" + fullFileName + "\". " + e.what());
                }
            }
        }

        UNLIKELY_IF (!loadStrings_(fileContent[0], fileContent[1], fileContent[2], fileContent[3], fileContent[4])) {
            string s = "Error during shader file load\n";
            LOOPI(5) if (!fileName[i].empty()) s += " " + shaderTypeStringSameLenght[i] + ": " + this->fileName[i] + "\n";
            s += infoLog_;
            throw std::runtime_error(s);
        }
        vertexStageInputPrimitiveTopology = primitiveTopology;
    }

    void PipelineRasterization::setVertexStageInputPrimitiveTopology(
        PrimitiveTopology primitiveTopology
    ) {
        this->vertexStageInputPrimitiveTopology = primitiveTopology;
    }

    void PipelineRasterization::setAttributeLayoutThrow(
        const std::string& errorMessage
    ) {
        const int uppermostActiveLocation = attributeLayoutStates.uppermostActiveLocation;
        std::string throwString;

        throwString += errorMessage + "\n";
        throwString += " AttributeLayout\n";
        throwString += "  TODO...\n";
        throwString += " Shader attribute locations:\n";
        LOOPI(uppermostActiveLocation + 1) {
            throwString += "  layout (location = " + to_string(i) + ") in " + gl::typeToGlslAndCNameString(attributeLocationInfo[i].type) + " " + attributeLocationInfo[i].name + "\n";
        }
        throw std::runtime_error(throwString);
    }

    /*
        This function also sets all attribute buffers if the pipeline to null

        test for limits (Probably want to put most of this into the AttributeLayout creation.
        MAX_VERTEX_ATTRIBS
        MAX_VERTEX_ATTRIB_BINDINGS
        MAX_VERTEX_ATTRIB_STRIDE

        Transform Feedback only?
            MAX_VERTEX_OUTPUT_COMPONENTS
            MAX_VERTEX_STREAMS

        TODO:
            - Need warning for attributes in layout that do not exist in shader, and for attribute that does exist but that is not set by the layout
            - All source type counts (1..4) are compatible to all shader type counts (1..4). But normaly it makes no sense, so maybe alos give out warning messages for that?
    */
    void PipelineRasterization::setAttributeLayout(
        const AttributeLayout& attributeLayout
    ) {
        //first we clear all current attribute buffer set in this pipeline to null
        for (int32_t i = 0; i <= attributeLayoutStates.uppermostActiveBufferIndex; ++i) {
            buffer_attribute_id    [i] = 0;
            buffer_attribute_offset[i] = 0;
        }

        const int uppermostActiveBufferIndex = attributeLayout.uppermostActiveBufferIndex;
        //We only have to set locations that are used by this shader, everything else just stays on Usage::indifferent.
        //We need to go over all locations that are used by this shader to overwrite everything that maybe was set by a previous AttributeLayout
        const int uppermostActiveLocation    = attributeLayoutStates.uppermostActiveLocation;

        //test types compatibility...
        LOOPI(uppermostActiveLocation + 1) {
            const auto& attributeFormat = attributeLayout.location[i].attributeFormat;

            if (attributeLocationInfo[i].type == 0) continue;
            GLenum shaderLocationBaseType = gl::typeToBaseType(attributeLocationInfo[i].type);

            if (attributeFormat == AttributeFormat::B10G11R11_UFLOAT) {
                UNLIKELY_IF (!(threadContextGroup_->version.gl >= GlVersion::v44))
                    setAttributeLayoutThrow("AttributeFormat::B10G11R11_UFLOAT (GL_UNSIGNED_INT_10F_11F_11F_REV) is only supported with OpenGL 4.4 or higher");
            }
            switch (shaderLocationBaseType) {
                case GL_FLOAT:
                    switch (attributeFormat->componentsType) {
                        case GL_FLOAT:
                        case GL_HALF_FLOAT:
                        case GL_DOUBLE:
                            break;
                        default: {
                            if (!attributeFormat->normalized)
                                setAttributeLayoutThrow("Shader base type float only takes integer type as normalized format!");
                        }
                    }
                    break;
                case GL_DOUBLE:
                    switch (attributeFormat->componentsType) {
                        case GL_DOUBLE:
                            break;
                        default: setAttributeLayoutThrow("Shader base type double only takes double source base type!");
                    }
                    break;
                case GL_INT:
                    switch (attributeFormat->componentsType) {
                        case GL_BYTE:
                        case GL_SHORT:
                        case GL_INT:
                            break;
                        default: setAttributeLayoutThrow("Shader base type signed integer only takes signed integer source base type!");
                    }
                    break;
                case GL_UNSIGNED_INT:
                    switch (attributeFormat->componentsType) {
                        case GL_UNSIGNED_BYTE:
                        case GL_UNSIGNED_SHORT:
                        case GL_UNSIGNED_INT:
                            break;
                        default: setAttributeLayoutThrow("Shader base type unsigned integer only takes unsigned integer source base type!");
                    }
                    break;
            }
        }


        //copy layout
        LOOPI(uppermostActiveBufferIndex + 1) {
            attributeLayoutStates.bufferIndexStride[i] = attributeLayout.bufferIndex[i].stride;
        }
        LOOPI(uppermostActiveLocation + 1) {
            if (attributeLocationInfo[i].type == 0) {
                attributeLayoutStates.location[i].usage = AttributeLayoutStates::Usage::indifferent;
            } else {
                if (attributeLayout.location[i].attributeFormat != AttributeFormat::NONE) {
                    attributeLayoutStates.location[i].usage = AttributeLayoutStates::Usage::enabled;
                } else {
                    attributeLayoutStates.location[i].usage = AttributeLayoutStates::Usage::disabled;
                }
            }
            attributeLayoutStates.location[i].attributeFormat = attributeLayout.location[i].attributeFormat;
            attributeLayoutStates.location[i].bufferIndex     = attributeLayout.location[i].bufferIndex;
            attributeLayoutStates.location[i].offset          = attributeLayout.location[i].offset;
        }
        attributeLayoutStates.uppermostActiveBufferIndex = attributeLayout.uppermostActiveBufferIndex;
        //attributeLayoutStates.uppermostActiveLocation is set by what locations are actually used in the shader. All other can be ingnored and do not have to be disabled for this shader.
        //Only need to disable Attribute locations if the shader uses them and layout does not define them, to prevent error if the current still active layout setting would cause GL error.

        if (threadContextGroup_->extensions.GL_ARB_vertex_attrib_binding) {
            LOOPI(uppermostActiveBufferIndex + 1)
                attributeLayoutStates.instancing[i] = attributeLayout.bufferIndex[i].instancing;
        } else {
            LOOPI(uppermostActiveLocation + 1)
                attributeLayoutStates.instancing[i] = attributeLayout.bufferIndex[attributeLayout.location[i].bufferIndex].instancing;
        }

        if (this == threadContext_->pipeline) {
            threadContext_->attributeLayoutChanged = true;
        }
        buffer_attribute_markSlotChange(0);
        buffer_attribute_markSlotChange(attributeLayout.uppermostActiveBufferIndex);
    }

    void PipelineRasterization::setAttributeLayout() {
        LOOPI(attributeLayoutStates.uppermostActiveLocation + 1) {
            if (attributeLayoutStates.location[i].usage != AttributeLayoutStates::Usage::indifferent)
                attributeLayoutStates.location[i].usage = AttributeLayoutStates::Usage::disabled;
        }
        if (this == threadContext_->pipeline) {
            threadContext_->attributeLayoutChanged = true;
        }
    }

    void PipelineRasterization::setAttributeBuffer(
        uint32_t               slot,
        const BufferInterface& buffer,
        uintptr_t              offset
    ) {
        if (int32_t(slot) >= config::MAX_ATTRIBUTES)
            throw runtime_error("setAttributeBuffer(slot = " + to_string(slot) + ", ...) is >= config::MAX_ATTRIBUTES(" + to_string(slot) + ")");

        buffer_attribute_id    [slot] = buffer.id;
        buffer_attribute_offset[slot] = buffer.id ? offset : 0;
        buffer_attribute_markSlotChange(slot);
    }

    void PipelineRasterization::setAttributeBuffer(
        uint32_t               slot
    ) {
        if (int32_t(slot) >= config::MAX_ATTRIBUTES)
            throw runtime_error("setAttributeBuffer(slot = " + to_string(slot) + ", ...) is >= config::MAX_ATTRIBUTES(" + to_string(slot) + ")");

        buffer_attribute_id    [slot] = 0;
        buffer_attribute_offset[slot] = 0;
        buffer_attribute_markSlotChange(slot);
    }

    void PipelineRasterization::setAttributeBuffer() {
        for (int32_t i = 0; i < config::MAX_ATTRIBUTES; ++i) {
            buffer_attribute_id    [i] = 0;
            buffer_attribute_offset[i] = 0;
        }
        buffer_attribute_changedSlotMin = 0;
        buffer_attribute_changedSlotMax = config::MAX_ATTRIBUTES - 1;
    }

    /*

    */
    void PipelineRasterization::setAttributeIndexBuffer(
        IndexType              indexType,
        const BufferInterface& buffer,
        uintptr_t              offset
    ) {
        if (indexType == IndexType::UINT16) {
            UNLIKELY_IF (offset % 2)
                throw std::runtime_error("Error: offset must be aligned to indexType size! IndexType::UINT16 = 2, offset is " + to_string(offset));
        } else {
            UNLIKELY_IF (offset % 4)
                throw std::runtime_error("Error: offset must be aligned to indexType size! IndexType::UINT32 = 4, offset is " + to_string(offset));
        }

        this->indexType                     = indexType;
        this->buffer_attribute_index_id     = buffer.id;
        this->buffer_attribute_index_offset = offset;
    }

    void PipelineRasterization::setAttributeIndexBuffer() {
        buffer_attribute_index_id = 0;
    }

    /**
        Changes front of triangles between clockwise and counter clockwise rotation of vertex on screen
        Default is false (counter clockwise)
    */
    void PipelineRasterization::setFaceFrontClockwise(
        bool clockwise
    ) {
        this->triangleFrontIsClockwiseRotation = clockwise;
        stateChange.triangleFace = true;
    }

    /**
        Sets what face side to draw. (side not to cull)
        Default is that FaceSelection::frontAndBack
    */
    void PipelineRasterization::setFaceSideToDraw(
        FaceSelection faceSelection
    ) {
        this->faceToDraw = faceSelection;
        stateChange.triangleFace = true;
    }

    /** \brief Sets if writing is enabled for all RGBA slots
     * By default writing is enabled for all
     * \param r write enabled for r
     * \param g write enabled for g
     * \param b write enabled for b
     * \param a write enabled for a
    */
    void PipelineRasterization::setRgbaWrite(
        bool r,
        bool g,
        bool b,
        bool a
    ) {
        rgbaWriteMask[0].r = r;
        rgbaWriteMask[0].g = g;
        rgbaWriteMask[0].b = b;
        rgbaWriteMask[0].a = a;
        singleRgbaWriteMaskState = true;
        stateChange.rgbaMask = true;
    }

    //core since 3.0

    /** \brief Sets if writing is enabled for a specific RGBA slot
     * By default writing is enabled for all
     * \param slot rgba slot
     * \param r write enabled for r
     * \param g write enabled for g
     * \param b write enabled for b
     * \param a write enabled for a
    */
    void PipelineRasterization::setRgbaWrite(
        uint32_t slot,
        bool     r,
        bool     g,
        bool     b,
        bool     a
    ) {
        if (singleRgbaWriteMaskState) for (int i = 1; i < config::MAX_RGBA_ATTACHMENTS; ++i) {
            rgbaWriteMask[i].r = rgbaWriteMask[0].r;
            rgbaWriteMask[i].g = rgbaWriteMask[0].g;
            rgbaWriteMask[i].b = rgbaWriteMask[0].b;
            rgbaWriteMask[i].a = rgbaWriteMask[0].a;
        }
        rgbaWriteMask[slot].r = r;
        rgbaWriteMask[slot].g = g;
        rgbaWriteMask[slot].b = b;
        rgbaWriteMask[slot].a = a;
        singleRgbaWriteMaskState = false;
        stateChange.rgbaMask = true;
    }

    /*
        Sets the operator to test the output agains the frame value, or disables the test.

        Default value is CompareOperator::disabled

        If this test fails, no rgba or depth fragment is written. The stencil value can still be written, depending on the stencil setup.

        Note that depth write is an independend setting. And that depth write only depends on the depth test, if depth test is enabled.
    */
    void PipelineRasterization::setDepthTest(
        CompareOperator compareOperator
    ) {
        depthCompareOperator = compareOperator;
        stateChange.depth = true;
    }

    /*
     * Enable/Disable depth write. Depth never will be written if the stencil or depth test failed!
     */
    void PipelineRasterization::setDepthWrite(
        bool enabled
    ) {
        depthWriteEnabled = enabled;
        stateChange.depth = true;
    }

    //TODO: add GL_EXT_polygon_offset_clamp support?

    /** \brief add biad to depth value
     *
     * \param depthBiasConstantFactor
     * \param depthBiasClamp          must be 0.0f without GL_ARB_polygon_offset_clamp (Core since 4.6)
     * \param depthBiasSlopeFactor
     *
     * This function should be prefered over writing to the depth value in the fragemnt shader!
     * Manually overwriting depth in the fragemnt shader prevents hardware optimisation (early z-rejection, hierarchical-z buffering, etc...)
     *
     * TODO: If vertex have exact same coordinates, a value of 1 or -1 will work fine. Otherwise it probably should be at last 2 or -2, to prevent z-fighting?
    */
    void PipelineRasterization::setDepthBias(
        float depthBiasConstantFactor,
        float depthBiasClamp,
        float depthBiasSlopeFactor
    ) {
        UNLIKELY_IF (    depthBiasClamp != 0.0f
                     &&  !threadContextGroup_->extensions.GL_ARB_polygon_offset_clamp)
            throw std::runtime_error("depthBiasClamp must be 0.0f without GL_ARB_polygon_offset_clamp (Core since 4.6)");

        this->depthBiasConstantFactor = depthBiasConstantFactor;
        this->depthBiasClamp          = depthBiasClamp;
        this->depthBiasSlopeFactor    = depthBiasSlopeFactor;

        stateChange.depth = true;
    }

    /*
        This defines the shader output to depth buffer mapping. In a normalized (not float32) depth buffer, 0.0 to 1.0 represents the full range.

        While this mapping can be inverted (e.g. 1.0, 0.0), both values are clamped to [0, 1]. To map depth outside of this range one has to use GL_ARB_clip_control.

        The default value is 0.0, 1.0

        Note that this function takes 64bit floating point values. Because 32bit floating point (IEEE 754) values only have 23bits mantissa,
        and therefore can not represent all steps of a normalized 32bit or 24bit unsigned integer.
    */
    void PipelineRasterization::setDepthRange(
        double near,
        double far
    ) {
        depthNearMapping = near;
        depthFarMapping  = far;
        stateChange.depth = true;
    }

    /*
        Depth clipping is enabled by default.

        To switch to depth clamping set this to true.

        boundry test (culling in front of near and behind far of projection)
    */
    void PipelineRasterization::setDepthClippingToClamping(
        bool enabled
    ) {
        depthClippingToClamping = enabled;
        stateChange.depth = true;
    }

    /*
        Sets the stencil reference value. Used by stencil testing and writing (via StencilOperator::replace).

        Default is 0
    */
    void PipelineRasterization::setStencilRefValue(
        FaceSelection faceSelection,
        int32_t       refValue
    ) {
        if (faceSelection == FaceSelection::frontAndBack || faceSelection == FaceSelection::front) {
            stencilTestFront.refValue        = refValue;
        }
        if (faceSelection == FaceSelection::frontAndBack || faceSelection == FaceSelection::back) {
            stencilTestBack.refValue        = refValue;
        }
        stateChange.stencil = true;
    }

    /*
        Sets the stencil test mode.

        Default is  FaceSelection::frontAndBack, CompareOperator::disabled, 0

        If the stencil test fails, no RGBA or DEPTH value is written. Stencil values can still be written depending on the stencil write settings.

        Note: Stencil writing is an independend setting but shares the stencil reference value!
    */
    void PipelineRasterization::setStencilTest(
        FaceSelection   faceSelection,
        uint32_t        readMask,
        CompareOperator compareOperator
    ) {
        if (faceSelection == FaceSelection::frontAndBack || faceSelection == FaceSelection::front) {
            stencilTestFront.compareOperator = compareOperator;
            stencilTestFront.readMask        = readMask;
        }
        if (faceSelection == FaceSelection::frontAndBack || faceSelection == FaceSelection::back) {
            stencilTestBack.compareOperator = compareOperator;
            stencilTestBack.readMask        = readMask;
        }
        stateChange.stencil = true;
    }

    /*
        Sets the stencil write mode.

        Default for all triangle sides is no writing (StencilOperator::keep)
    */
    void PipelineRasterization::setStencilWrite(
        FaceSelection   faceSelection,
        uint32_t        writeMask,
        StencilOperator stencilFailOperator,
        StencilOperator stencilPassDepthFailOperator,
        StencilOperator stencilPassDepthPassOrAbsentOperator
    ) {
        if (faceSelection == FaceSelection::frontAndBack || faceSelection == FaceSelection::front) {
            stencilWriteFront.writeMask                            = writeMask;
            stencilWriteFront.stencilFailOperator                  = stencilFailOperator;
            stencilWriteFront.stencilPassDepthFailOperator         = stencilPassDepthFailOperator;
            stencilWriteFront.stencilPassDepthPassOrAbsentOperator = stencilPassDepthPassOrAbsentOperator;
        }
        if (faceSelection == FaceSelection::frontAndBack || faceSelection == FaceSelection::back) {
            stencilWriteBack.writeMask                            = writeMask;
            stencilWriteBack.stencilFailOperator                  = stencilFailOperator;
            stencilWriteBack.stencilPassDepthFailOperator         = stencilPassDepthFailOperator;
            stencilWriteBack.stencilPassDepthPassOrAbsentOperator = stencilPassDepthPassOrAbsentOperator;
        }
        stateChange.stencil = true;
    }

    /*
        rgba blend

        Blending is ignored for non normalized integer render targets!

        MAX_DUAL_SOURCE_DRAW_BUFFERS (Min. 1)

        is depricated since 3.0, can all be done in the shader via discard
            void glAlphaFunc(GLenum func, GLclampf ref)

        Core since 1.3
            void glBlendColor               (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
            void glBlendFunc                (            GLenum sfactor, GLenum dfactor);
            void glBlendEquation            (            GLenum mode);
        Core since 1.4
            void glBlendFuncSeparate        (            GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
        Core sinec 2.0
            void glBlendEquationSeparate    (            GLenum modeRGB, GLenum modeAlpha);
        GL_ARB_draw_buffers_blend (Core as non-ARB since 4.0)
            void glBlendFunciARB            (GLuint buf, GLenum src, GLenum dst);
            void glBlendFuncSeparateiARB    (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
            void glBlendEquationiARB        (GLuint buf, GLenum mode);
            void glBlendEquationSeparateiARB(GLuint buf, GLenum modeRGB, GLenum modeAlpha);
        Core since 4.0
            void glBlendFunci               (GLuint buf, GLenum sfactor, GLenum dfactor);
            void glBlendFuncSeparatei       (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
            void glBlendEquationi           (GLuint buf, GLenum mode);
            void glBlendEquationSeparatei   (GLuint buf, GLenum modeRGB, GLenum modeAlpha);

        The maximum number of draw buffers that may be attached to a single frame-buffer when using dual-source blending functions is implementation-dependent
        and may be queried by calling GetIntegerv with pname MAX_DUAL_SOURCE_DRAW_BUFFERS . When using dual-source blending, MAX_DUAL_SOURCE_DRAW_-
        BUFFERS should be used in place of MAX_DRAW_BUFFERS to determine the maximum number of draw buffers that may be attached to a single framebuffer. The
        value of MAX_DUAL_SOURCE_DRAW_BUFFERS must be at least 1. If the value of MAX_DUAL_SOURCE_DRAW_BUFFERS is 1, then dual-source blending and multiple
        draw buffers cannot be used simultaneously.

        TODO: GL_ARB_blend_func_extended (Core since 3.3, but buggy on AMD?) (Dual blending)
        rgba blending is undefined behavior for int/uint targets? Or just always disabled?
    */

    void PipelineRasterization::setRgbaBlendConst(
        glm::vec4 rgba
    ) {
        blendConstRgba = rgba;
        stateChange.blend = true;
    }

    /** \brief Setup a single blend mode and enable it for all Rgba targets that have a normalized or floatingpoint format
     *
     * \param constRgba
     * \param srcFactorRgb
     * \param srcFactorA
     * \param equationRgb
     * \param equationA
     * \param dstFactorRgb
     * \param dstFactorA
     */
    void PipelineRasterization::setRgbaBlend(
        BlendFactorRgb srcFactorRgb,
        BlendFactorA   srcFactorA,
        BlendEquation  equationRgb,
        BlendEquation  equationA,
        BlendFactorRgb dstFactorRgb,
        BlendFactorA   dstFactorA
    ) {
        blendEnabledAny   = true;
        blendEnabledAll   = true;
        blendModesUniform = true;
        LOOPI(config::MAX_RGBA_ATTACHMENTS) {
            blendEnabled  [i]        = true;
            blendFactors  [i].srcRgb = srcFactorRgb;
            blendFactors  [i].srcA   = srcFactorA;
            blendFactors  [i].dstRgb = dstFactorRgb;
            blendFactors  [i].dstA   = dstFactorA;
            blendEquations[i].rgb    = equationRgb;
            blendEquations[i].a      = equationA;
        }
        stateChange.blend = true;
    }

    /** \brief Set a single blend mode for all rgba slots
     *
        Without GL_ARB_draw_buffers_blend (Core since 4.0) blending can only be selective enabled/disabled per rgbaSlot, but factors and equations must be the same for all!

        Without GL_ARB_draw_buffers_blend:
            If blending is already enabled for some rgbaSlots and this function is called with different blending parameters, this function will throw!

        Note from OpenGL wiki: All AMD/ATI 3.x-class hardware, the HD2000, 3000, and 4000 series, implement ARB_draw_buffers_blend. No pre-4.0 hardware from NVIDIA implements this.

        \param rgbaSlot
        \param srcFactorRgb
        \param srcFactorA
        \param equationRgb
        \param equationA
        \param dstFactorRgb
        \param dstFactorA
    */
    void PipelineRasterization::setRgbaBlend(
        uint32_t       rgbaSlot,
        BlendFactorRgb srcFactorRgb,
        BlendFactorA   srcFactorA,
        BlendEquation  equationRgb,
        BlendEquation  equationA,
        BlendFactorRgb dstFactorRgb,
        BlendFactorA   dstFactorA
    ) {
        UNLIKELY_IF (rgbaSlot >= config::MAX_RGBA_ATTACHMENTS)
            throw std::runtime_error("Trying to set slot(" + to_string(rgbaSlot) + ") that is bayond config::MAX_RGBA_ATTACHMENTS(" + to_string(config::MAX_RGBA_ATTACHMENTS) + ")");

        blendEnabledAny   = true;
        if (blendEnabledAll.isFalse()) blendEnabledAll = {};
        blendModesUniform = {};
        blendEnabled  [rgbaSlot]        = true;
        blendFactors  [rgbaSlot].srcRgb = srcFactorRgb;
        blendFactors  [rgbaSlot].srcA   = srcFactorA;
        blendFactors  [rgbaSlot].dstRgb = dstFactorRgb;
        blendFactors  [rgbaSlot].dstA   = dstFactorA;
        blendEquations[rgbaSlot].rgb    = equationRgb;
        blendEquations[rgbaSlot].a      = equationA;
        stateChange.blend = true;
    }

    /** \brief disables rgba blend for a specific rgba slot (Rgba blend is disabled for all slots by default)
     */
    void PipelineRasterization::setRgbaBlendDisabled(uint32_t rgbaSlot) {
        UNLIKELY_IF (rgbaSlot >= config::MAX_RGBA_ATTACHMENTS)
            throw std::runtime_error("Trying to set slot(" + to_string(rgbaSlot) + ") that is bayond config::MAX_RGBA_ATTACHMENTS(" + to_string(config::MAX_RGBA_ATTACHMENTS) + ")");

        blendEnabledAny = {};
        blendEnabledAll = false;
        blendEnabled[rgbaSlot] = false;
        stateChange.blend = true;
    }

    /** \brief disables rgba blend for all rgba slots (Rgba blend is disabled for all slots by default)
     */
    void PipelineRasterization::setRgbaBlendDisabled() {
        blendEnabledAny = false;
        blendEnabledAll = false;
        LOOPI(config::MAX_RGBA_ATTACHMENTS) blendEnabled[i] = false;
        stateChange.blend = true;
    }

    //LOGIC OPERATION
    /*LOGIC OPERATION
        There is no per-color-target selection of logicOp in core 4.6! All active color targets will be drawen with the same set logic operation!

        setLogicOperation(LogicOperation logicOperation);

        blocks any blending, so only logicOp OR blending can be used

        Operats on each component (R, G, B, A) and render target seperatly!

        Has no effect on floating point render targets!
        Has no effect on SRGB Frame buffers!

        glEnable (GL_COLOR_LOGIC_OP);
        glDisable(GL_COLOR_LOGIC_OP);
        glLogicOp(LogicOperation logicOperation);
    */

    /** \brief Enable/Disable multi sample rendering (default is enabled)
     * if disabled rendering to a multisample targets will fill all samples of a texel with the same value
    */
    void PipelineRasterization::setMultisample(bool enable) {
        multiSample = enable;
    }

    /*
        Removed GL_UNSIGNED_BYTE as valid option! In some GPUs from the last century it is NOT natively supported and brings a heavy performance penalty if used!

        before 3.1 there also was GL_NV_primitive_restart that still used "client states commands" for this

        TODO: where to document that patches primitives are not always support restart index? (GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED)
        GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED is first listed in the 4.4 core spec.
        Somewhat related to ARB_tessellation_shader (core since 4.0)?! (At last in mesa)
        Documentation is shit and the two bug numbers (10364, 10250) in ref can't be found in Khronos bug tracker.
        At this point in time I do not even know how to know if I can query this value without causing an GL error...
        And if I query it and querying is not supported that tells me nothing about the actually support for using restart on patches...
        I also do not know if there is any core version with minimum support of either.

    */
    void PipelineRasterization::activateAttributeIndex(
        IndexType indexType
    ) {
        if (threadContext_->buffer_attribute_index_id != buffer_attribute_index_id) {
            threadContextGroup_->functions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_attribute_index_id);
            threadContext_->buffer_attribute_index_id = buffer_attribute_index_id;
        }
        if (threadContextGroup_->extensions.GL_ARB_ES3_compatibility) return;
        if (threadContext_->buffer_attribute_index_type != indexType) {
            switch (indexType) {
                case IndexType::UINT16: threadContextGroup_->functions.glPrimitiveRestartIndex(    0xFFFF); break;
                case IndexType::UINT32: threadContextGroup_->functions.glPrimitiveRestartIndex(0xFFFFFFFF); break;
              //case IndexType::UINT8 : threadContextGroup_->functions.glPrimitiveRestartIndex(      0xFF); break;
            }
            threadContext_->buffer_attribute_index_type = indexType;
        }
        if (!threadContext_->buffer_attribute_index_enabled) {
            threadContextGroup_->functions.glEnable(GL_PRIMITIVE_RESTART);
            threadContext_->buffer_attribute_index_enabled = true;
        }
    }

    /**
        From https://www.opengl.org/wiki/Vertex_Rendering
            "Note: OpenGL 4.4 and below defined primitive restarting to work with all drawing commands, including the non-indexed commands.
            However, implementations did not implement this, and the rule itself makes no sense, so GL 4.5 specifies
            (OpenGL 4.5, Section 10.6.3, page 342)that restart state has no effect on non-indexed drawing commands."

        Some drivers actually folow this nonsensical standard. That of course breaks stuff, so we disable primitive restart for non index drawing here!

        GL 4.5 specs:
            "Note that primitive restart is not performed for array elements transferred by
            any drawing command not taking a type parameter, including all of the *Draw*
            commands other than *DrawElements*."

        GL_ARB_ES3_compatibility (Core since 4.3) via GL_PRIMITIVE_RESTART_FIXED_INDEX does NOT use restart index in case of non index drawing!

        Because we already test for GL_ARB_ES3_compatibility (Core since 4.3), we do not have to check for GL 4.5
    */
    void PipelineRasterization::deactivateAttributeIndex() {
        if (threadContextGroup_->extensions.GL_ARB_ES3_compatibility) return;
        if (threadContext_->buffer_attribute_index_enabled) {
            threadContextGroup_->functions.glDisable(GL_PRIMITIVE_RESTART);
            threadContext_->buffer_attribute_index_enabled = false;
        }
    }

    bool PipelineRasterization::loadStrings_(
        const std::string& stringVertex,
        const std::string& stringTessControl,
        const std::string& stringTessEvalution,
        const std::string& stringGeometry,
        const std::string& stringFragment
    ) {
        const string *const shaderSrc[] = {
            &stringVertex,
            &stringTessControl,
            &stringTessEvalution,
            &stringGeometry,
            &stringFragment
        };
        const GLenum shaderType[] = {
            GL_VERTEX_SHADER,
            GL_TESS_CONTROL_SHADER,
            GL_TESS_EVALUATION_SHADER,
            GL_GEOMETRY_SHADER,
            GL_FRAGMENT_SHADER
        };
        GLuint shaderId[5] = {0};
        LOOPI(5) if (!shaderSrc[i]->empty()) {
            GLuint shaderPartId = threadContextGroup_->functions.glCreateShader(shaderType[i]);
            const char* pCString = shaderSrc[i]->c_str();
            threadContextGroup_->functions.glShaderSource(shaderPartId, 1, reinterpret_cast<const GLchar**>(&pCString), NULL);
            threadContextGroup_->functions.glCompileShader(shaderPartId);
            GLint compileSuccessful;
            threadContextGroup_->functions.glGetShaderiv(shaderPartId, GL_COMPILE_STATUS, &compileSuccessful);
            string shaderLog = getShaderInfoLog(shaderPartId);
            if (!shaderLog.empty())
                infoLog_ += shaderTypeString[i] + " LOG:\n" + shaderLog + "\n";
            shaderId[i] = shaderPartId;
            if (!compileSuccessful) {
                LOOPI(5) if (shaderId[i]) threadContextGroup_->functions.glDeleteShader(shaderId[i]);
                //throw std::runtime_error("Error loading shader:\n" + infoLog_);
                infoLog_ = "Error loading shader:\n" + infoLog_;
                return false;
            }
        }
        id = threadContextGroup_->functions.glCreateProgram();
        //if (binaryRetrievableHint && threadContextGroup_->extensions.GL_ARB_get_program_binary)
        //    threadContextGroup_->functions.glProgramParameteri(id, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
        //else
        //    glProgramParameteri(id, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_FALSE);

        LOOPI(5) if (shaderId[i]) threadContextGroup_->functions.glAttachShader(id, shaderId[i]);
        threadContextGroup_->functions.glLinkProgram(id);

        GLint linkStatus;
        threadContextGroup_->functions.glGetProgramiv(id, GL_LINK_STATUS, &linkStatus);
        string programLog = getProgramInfoLog(id);
        if (!programLog.empty())
            infoLog_ += "PROGRAM LINK STATUS:\n" + getProgramInfoLog(id) + "\n";

        LOOPI(5) if (shaderId[i]) {
            threadContextGroup_->functions.glDetachShader(id, shaderId[i]);
            threadContextGroup_->functions.glDeleteShader(shaderId[i]);
        }

        if (!linkStatus) {
            threadContextGroup_->functions.glDeleteProgram(id);
            id = 0;
            //throw std::runtime_error("Error linking shader:\n" + infoLog_);
            infoLog_ = "Error linking shader:\n" + infoLog_;
            return false;
        }

        //TODO: maybe make this better
        hasShader[0] = !stringVertex.empty();
        hasShader[1] = !stringTessControl.empty();
        hasShader[2] = !stringTessEvalution.empty();
        hasShader[3] = !stringGeometry.empty();
        hasShader[4] = !stringFragment.empty();

        collectInformation();
        allocateMemory();
        setDefaultValues();
        return true;
    }

    void PipelineRasterization::setDefaultValues() {
        //PipelineRasterizationState is initialized to OpenGL defaults. But that is not what we always want for the PipelineRasterization objects!
        depthWriteEnabled = false;
    }

    void PipelineRasterization::collectInformation() {
        PipelineInterface::collectInformation();

        //Attributes
        int32_t activeAttributeCount         = 0;
        int32_t activeattributeNameLengthMax = 0;
        std::vector<char> nameBuffer;

        threadContextGroup_->functions.glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES,           &activeAttributeCount);
        threadContextGroup_->functions.glGetProgramiv(id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &activeattributeNameLengthMax);
        nameBuffer.resize(activeattributeNameLengthMax);

        //int nextEntry = 0;
        LOOPI(activeAttributeCount) {
            //AttributeListEntry newEntry;
            GLsizei stringLenght = 0;
            GLint   typeCount    = 0;
            GLenum  type         = 0;

            //core since 2.0
            threadContextGroup_->functions.glGetActiveAttrib(id, GLuint(i), activeattributeNameLengthMax, &stringLenght, &typeCount, &type, &nameBuffer[0]);

            //On r290 (win10 blop/mesa) typeCount always is 1 even with all vec3/vec4 double types!
            //For older hardware dvec3/dvec4 or dmat could use up two times the location! They probably give out a typeCount higher then 1?! (TODO: Test on rusty hardware!)
            //Only useful here to give out a debug warning on systems that have value of "1" on all relevant double values...
            //IF const GL requirements may allows this applicaiton to run on hardware where it could encounter multi-location limitation and static set locations in shader could overlap or go bayond max. location count!
            /*if (typeCount == 1) {
                switch (type) {
                    case GL_DOUBLE_VEC3:
                    case GL_DOUBLE_VEC4:
                    case GL_DOUBLE_MAT3:
                    case GL_DOUBLE_MAT4:
                    case GL_DOUBLE_MAT3x2:
                    case GL_DOUBLE_MAT3x4:
                    case GL_DOUBLE_MAT4x2:
                    case GL_DOUBLE_MAT4x3:
                    //TODO runtime debug warning here
                }
            }*/

            /*newEntry.location = threadContextGroup_->functions.glGetAttribLocation(id, &nameBuffer[0]);
            newEntry.type     = type;
            newEntry.name     = std::string(&nameBuffer[0], stringLenght);
            //for some reason on r290, GL_ACTIVE_ATTRIBUTES always return 1 if no vertex attributes exist (on Win10 blob and also mesa!)
            //So we just filter out attribute entries with location == -1
            if (newEntry.location != -1) attributeList[nextEntry++] = newEntry;*/

            int8_t location = threadContextGroup_->functions.glGetAttribLocation(id, &nameBuffer[0]);
            string name  = std::string(&nameBuffer[0], stringLenght);
            //for some reason on r290, GL_ACTIVE_ATTRIBUTES always return 1 if no vertex attributes exist (on Win10 blob and also mesa!)
            //So we just filter out attribute entries with location == -1
            if (location != -1) {
                attributeLocationInfo[location].type = type;
                attributeLocationInfo[location].name = name;

                attributeLayoutStates.uppermostActiveLocation = max(attributeLayoutStates.uppermostActiveLocation, location);
                auto& gpuType = attributeLayoutStates.location[location].gpuType;
                switch (gl::typeToBaseType(type)) {
                    case GL_FLOAT       : gpuType = AttributeLayoutStates::GpuType::f32; break;
                    case GL_INT         :
                    case GL_UNSIGNED_INT:
                    case GL_BOOL        : gpuType = AttributeLayoutStates::GpuType::i32; break;
                    case GL_DOUBLE      : gpuType = AttributeLayoutStates::GpuType::f64; break;
                }
            }
            //attributeLayoutStates.uppermostActiveLocation = uppermostActiveLocation;
        }

        //TESSELATION CONTROL SHADER
        //TESS_CONTROL_OUTPUT_VERTICES
        //TESS_GEN_MODE                //=QUADS, TRIANGLES, ISOLINES
        //TESS_GEN_SPACING             //=EQUAL, FRACTIONAL_EVEN, FRACTIONAL_ODD
        //TESS_GEN_VERTEX_ORDER        //=CCW, CW
        //TESS_GEN_POINT_MODE          //=bool

        //Core since 3.2
        if (hasGeometryStage()) {
            threadContextGroup_->functions.glGetProgramiv(id, GL_GEOMETRY_VERTICES_OUT, &geometryMaxPrimitveOutput);
            threadContextGroup_->functions.glGetProgramiv(id, GL_GEOMETRY_INPUT_TYPE,   reinterpret_cast<int32_t*>(&geometryInputType));  //=POINTS, LINES, LINES_ADJACENCY, TRIANGLES, TRIANGLES_ADJACENCY
            threadContextGroup_->functions.glGetProgramiv(id, GL_GEOMETRY_OUTPUT_TYPE,  reinterpret_cast<int32_t*>(&geometryOutputType)); //=POINTS, LINE_STRIP, TRIANGLE_STRIP

            //GEOMETRY_SHADER_INVOCATIONS
        }

        //int32_t transformFeedbackVaryingCount         = 0;
        //int32_t transformFeedbackVaryingNameLengthMax = 0;
        //int32_t transformFeedbackBufferMode;
        //threadContextGroup_->functions.glGetProgramiv(id, GL_TRANSFORM_FEEDBACK_VARYINGS,           &transformFeedbackVaryingCount);
        //threadContextGroup_->functions.glGetProgramiv(id, GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH, &transformFeedbackVaryingNameLengthMax);
        //threadContextGroup_->functions.glGetProgramiv(id, GL_TRANSFORM_FEEDBACK_BUFFER_MODE,        &transformFeedbackBufferMode); //= GL_SEPARATE_ATTRIBS or GL_INTERLEAVED_ATTRIBS.
    }

    /*
        Draw/rasterize primitive commands

        Ignoring the range commands, because they don't cover all cases and are most likely completely useless in today's hardware?! (Found no real info about this so far, GLES does not have range commands at all)

        still need to think about multiDraw... it makes sense for indirect. The non indirect doesn't fit in because it uses pointer-to-pointer-list style. Also the performance benefit is in doubt. Basically it would just be a loop of the non multi versions.
        and it is not supported by instanced for a reason I guess?!

        In glCompact the restart index is always automatically ~0!

        GL_MAX_ELEMENTS_VERTICES and GL_MAX_ELEMENTS_INDICES are JUST HINTS.
    */


    /*void PipelineRasterization::draw(
        uint32_t firstVertex,
        uint32_t vertexCount
    ) {
        processPendingChanges();
        deactivateAttributeIndex();
        threadContextGroup_->functions.glDrawArrays(static_cast<GLenum>(vertexStageInputPrimitiveTopology), firstVertex, vertexCount);
    }*/

    //depends on GL_ARB_draw_instanced (Core since 3.1)
    //we can not emulate GL_ARB_base_instance with rebinding the instance buffer offset, because it would change gl_InstanceID, but instanceOffset does not!

    /** \brief draw vertex
     *
     * \param vertexCount
     * \param instanceCount
     * \param firstVertex
     * \param firstInstance must be 0 if GL_ARB_base_instance (Core since 4.2) is not supported!
     */
    void PipelineRasterization::draw(
        uint32_t  vertexCount,
        uint32_t  instanceCount,
        uint32_t  firstVertex,
        uint32_t  firstInstance
    ) {
        UNLIKELY_IF (firstInstance > 0 && !threadContextGroup_->extensions.GL_ARB_base_instance)
            throw std::runtime_error("firstInstance must be 0 without support for GL_ARB_base_instance (Core since 4.2)!");

        processPendingChanges();
        deactivateAttributeIndex();
        if (firstInstance) {
            UNLIKELY_IF (!threadContextGroup_->extensions.GL_ARB_base_instance)
                throw std::runtime_error("GL_ARB_base_instance not supportet in driver, firstInstance must be 0!");
            threadContextGroup_->functions.glDrawArraysInstancedBaseInstance(static_cast<GLenum>(vertexStageInputPrimitiveTopology), firstVertex, vertexCount, instanceCount, firstInstance);
        } else {
            threadContextGroup_->functions.glDrawArraysInstanced            (static_cast<GLenum>(vertexStageInputPrimitiveTopology), firstVertex, vertexCount, instanceCount);
        }
    }

    //Needs ARB_draw_elements_base_vertex (Core since 3.2)
    /*void PipelineRasterization::drawIndexed(
        uint32_t firstIndex,
        uint32_t indexCount,
        int32_t  vertexOffset
    ) {
        //error out if no index buffer is set!? Do not want to support drawing from client pointer!!!
        //if ()
        processPendingChanges();
        activateAttributeIndex(indexType);
        uintptr_t indexBufferByteOffset = this->buffer_attribute_index_offset + (firstIndex * (indexType == IndexType::UINT16 ? 2 : 4)); //assuming we never support UINT8 index
        threadContextGroup_->functions.glDrawElementsBaseVertex(static_cast<GLenum>(vertexStageInputPrimitiveTopology), indexCount, static_cast<GLenum>(indexType), reinterpret_cast<const void*>(indexBufferByteOffset), vertexOffset);
    }*/

    //Needs ARB_draw_elements_base_vertex (Core since 3.2)

    /** \brief draw vertex via index buffer
     *
     * \param indexCount
     * \param instanceCount
     * \param firstIndex
     * \param vertexOffset
     * \param firstInstance must be 0 if GL_ARB_base_instance (Core since 4.2) is not supported!
     */
    void PipelineRasterization::drawIndexed(
        uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t firstIndex,
        int32_t  vertexOffset,
        uint32_t firstInstance
    ) {
        UNLIKELY_IF (firstInstance > 0 && !threadContextGroup_->extensions.GL_ARB_base_instance)
            throw std::runtime_error("firstInstance must be 0 without support for GL_ARB_base_instance (Core since 4.2)!");

        processPendingChanges();
        activateAttributeIndex(indexType);
        uintptr_t indexBufferByteOffset = this->buffer_attribute_index_offset + (firstIndex * (indexType == IndexType::UINT16 ? 2 : 4)); //assuming we never support UINT8 index
        if (firstInstance) {
            //debug test for threadContextGroup_->extensions.GL_ARB_base_instance
            threadContextGroup_->functions.glDrawElementsInstancedBaseVertexBaseInstance(static_cast<GLenum>(vertexStageInputPrimitiveTopology), indexCount, static_cast<GLenum>(indexType), reinterpret_cast<const void*>(indexBufferByteOffset), instanceCount, vertexOffset, firstInstance);
        } else {
            threadContextGroup_->functions.glDrawElementsInstancedBaseVertex            (static_cast<GLenum>(vertexStageInputPrimitiveTopology), indexCount, static_cast<GLenum>(indexType), reinterpret_cast<const void*>(indexBufferByteOffset), instanceCount, vertexOffset);
        }
    }

    //automatically uses GL_ARB_multi_draw_indirect (Core since 4.2) if available

    /** \brief draw vertex command with parameters coming from a buffer object
     * This function needs GL_ARB_draw_indirect (core since 4.0)
     * <pre>
     * typedef struct DrawIndirectCommand {
     *     uint32_t vertexCount;
     *     uint32_t instanceCount;
     *     uint32_t firstVertex;
     *     uint32_t firstInstance; //Must be 0 without GL_ARB_base_instance (Core since 4.2), otherwise causes undefined behavior
     * } DrawIndirectCommand;
     * </pre>
     * \param parameterBuffer source buffer of DrawIndirectCommand strctures
     * \param parameterBufferOffset byte offset to the first DrawIndirectCommand structure
     * \param count  how many DrawIndirectCommand structure
     * \param stride byte distence to next DrawIndirectCommand structur. Must be at last 16 (packed format) and be a power of 4!
     */
    void PipelineRasterization::drawIndirect(
        const BufferInterface& parameterBuffer,
        intptr_t               parameterBufferOffset,
        uint32_t               count,
        uint32_t               stride
    ) {
        UNLIKELY_IF (!threadContextGroup_->extensions.GL_ARB_draw_indirect)
            throw std::runtime_error("missing support for GL_ARB_draw_indirect (Core since 4.0)!");
        UNLIKELY_IF (!parameterBuffer.id)
            throw std::runtime_error("does not take empty indirect parameterBuffer!");
        UNLIKELY_IF (stride < 16 || stride % 4)
            throw std::runtime_error("stride must be >= 16 and aligned to 4!");

        processPendingChanges();
        deactivateAttributeIndex(); //does GL_ARB_draw_indirect/GL_ARB_multi_draw_indirect really need this?

        //threadContext->cachedBindDrawIndirectBuffer(buffer_parameter_id);
        threadContext_->cachedBindDrawIndirectBuffer(parameterBuffer.id);

        if (threadContextGroup_->extensions.GL_ARB_multi_draw_indirect) {
            threadContextGroup_->functions.glMultiDrawArraysIndirect(static_cast<GLenum>(vertexStageInputPrimitiveTopology), reinterpret_cast<const void*>(parameterBufferOffset), count, stride);
        } else {
            for (unsigned i = 0; i < count; i++) {
                threadContextGroup_->functions.glDrawArraysIndirect(static_cast<GLenum>(vertexStageInputPrimitiveTopology), reinterpret_cast<const void*>(parameterBufferOffset));
                parameterBufferOffset += stride;
            }
        }
    }

    //automatically uses GL_ARB_multi_draw_indirect (Core since 4.2) if available

    /** \brief draw vertex indexed with parameters coming from a buffer
     * This function needs GL_ARB_draw_indirect (core since 4.0)
     * <pre>
     * typedef struct DrawIndexedIndirectCommand {
     *     uint32_t indexCount;
     *     uint32_t instanceCount;
     *     uint32_t firstIndex;
     *      int32_t vertexOffset;
     *     uint32_t firstInstance; //must be 0 without GL_ARB_base_instance (Core since 4.2), otherwise causes undefined behavior
     * } DrawIndexedIndirectCommand;
     * </pre>
     * \param parameterBuffer source buffer of DrawIndexedIndirectCommand strctures
     * \param parameterBufferOffset byte offset to the first DrawIndexedIndirectCommand structure
     * \param count  how many DrawIndexedIndirectCommand structure
     * \param stride byte distence to next DrawIndexedIndirectCommand structur. Must be at last 20 (packed format) and be a power of 4!
     */
    void PipelineRasterization::drawIndexedIndirect(
        const BufferInterface& parameterBuffer,
        intptr_t               parameterBufferOffset,
        uint32_t               count,
        uint32_t               stride
    ) {
        //NOTE: Can't test for GL_ARB_base_instance != 0 here, except if we read buffer content manually! (Could be done as a really slow debug assert!)
        UNLIKELY_IF (!threadContextGroup_->extensions.GL_ARB_draw_indirect)
            throw std::runtime_error("missing support for GL_ARB_draw_indirect (Core since 4.0)!");
        UNLIKELY_IF (!parameterBuffer.id)
            throw std::runtime_error("does not take empty draw indirect parameterBuffer!");
        UNLIKELY_IF (stride < 20|| stride % 4)
            throw std::runtime_error("stride must be >= 20 and aligned to 4!");

        processPendingChanges();
        activateAttributeIndex(indexType);
        threadContext_->cachedBindDrawIndirectBuffer(parameterBuffer.id);

        if (threadContextGroup_->extensions.GL_ARB_multi_draw_indirect) {
            threadContextGroup_->functions.glMultiDrawElementsIndirect(static_cast<GLenum>(vertexStageInputPrimitiveTopology), static_cast<GLenum>(indexType), reinterpret_cast<const void*>(parameterBufferOffset), count, stride);
        } else {
            for (unsigned i = 0; i < count; i++) {
                threadContextGroup_->functions.glDrawElementsIndirect(static_cast<GLenum>(vertexStageInputPrimitiveTopology), static_cast<GLenum>(indexType), reinterpret_cast<const void*>(parameterBufferOffset));
                parameterBufferOffset += stride;
            }
        }
    }

    ///\cond HIDDEN_FROM_DOXYGEN
    /*
        GL_ARB_indirect_parameters (Core since 4.6!) Needs GL4.3 or higher

        Note: core has no ARB in function name!

        function parameter layout is designed like:
        VK_AMD_draw_indirect_count
        VK_KHR_draw_indirect_count (draft)

        parameterBufferByteOffsetToDrawCount must be a multiple of four! Like accessing an GLsizei[X]

        maxDrawCount is the maximum drawCount that gets used even if PARAMETER_BUFFER[parameterBufferByteOffsetToDrawCount] is larger.

        NOTE: GL_COMMAND_BARRIER_BIT (e.g. if a computer shader is used to generate values for the PARAMETER_BUFFER) may not work correctly on Nvidia because it was added very late to GL_ARB_indirect_parameters!
    */
    ///\endcond

    /** \brief draw vertex command with draw parameter structures coming from one buffer, and their count coming from another buffer
     * \details Depends on GL_ARB_indirect_parameters (Core since 4.6)
     *
     * \param countBufferOffset pointer to an uint32_t inside countBuffer that contains the count of DrawIndirectCommand structures
     */
    void PipelineRasterization::drawIndirectCount(
        const BufferInterface& parameterBuffer,
        intptr_t               parameterBufferOffset,
        const BufferInterface& countBuffer,
        intptr_t               countBufferOffset,
        intptr_t               maxDrawCount,
        uint32_t               stride
    ) {
        UNLIKELY_IF (!threadContextGroup_->extensions.GL_ARB_indirect_parameters)
            throw std::runtime_error("Missing GL_ARB_indirect_parameters (Core since 4.6)");
        UNLIKELY_IF (!parameterBuffer.id)
            throw std::runtime_error("does not take empty draw indirect parameterBuffer!");
        UNLIKELY_IF (!countBuffer.id)
            throw std::runtime_error("does not take empty count buffer!");
        UNLIKELY_IF (stride < 16 || stride % 4)
            throw std::runtime_error("stride must be >= 16 and aligned to 4!");

        processPendingChanges();
        //deactivateAttributeIndex(); //does GL_ARB_indirect_parameters need this?
        threadContext_->cachedBindDrawIndirectBuffer(parameterBuffer.id);
        threadContext_->cachedBindParameterBuffer(countBuffer.id);
        //TODO: use single function pointer set at init here? ARB should be the same as core!?
        if (threadContextGroup_->version.gl >= GlVersion::v46) {
            threadContextGroup_->functions.glMultiDrawArraysIndirectCount   (static_cast<GLenum>(vertexStageInputPrimitiveTopology), reinterpret_cast<const void*>(parameterBufferOffset), countBufferOffset, maxDrawCount, stride);
        } else {
            threadContextGroup_->functions.glMultiDrawArraysIndirectCountARB(static_cast<GLenum>(vertexStageInputPrimitiveTopology), reinterpret_cast<const void*>(parameterBufferOffset), countBufferOffset, maxDrawCount, stride);
        }
    }

    /** \brief draw vertex indexed with draw parameter structures coming from one buffer, and their count coming from another buffer
     *
     */
    void PipelineRasterization::drawIndexedIndirectCount(
        const BufferInterface& parameterBuffer,
        intptr_t               parameterBufferOffset,
        const BufferInterface& countBuffer,
        intptr_t               countBufferOffset,
        intptr_t               maxDrawCount,
        uint32_t               stride
    ) {
        UNLIKELY_IF (!threadContextGroup_->extensions.GL_ARB_indirect_parameters)
            throw std::runtime_error("Missing GL_ARB_indirect_parameters (Core since 4.6)");
        UNLIKELY_IF (!parameterBuffer.id)
            throw std::runtime_error("does not take empty draw indirect parameterBuffer!");
        UNLIKELY_IF (!countBuffer.id)
            throw std::runtime_error("does not take empty count buffer!");
        UNLIKELY_IF (stride < 20|| stride % 4)
            throw std::runtime_error("stride must be >= 20 and aligned to 4!");

        processPendingChanges();
        activateAttributeIndex(indexType);
        threadContext_->cachedBindDrawIndirectBuffer(parameterBuffer.id);
        threadContext_->cachedBindParameterBuffer(countBuffer.id);
        //TODO: use single function pointer set at init here? ARB should be the same as core!?
        if (threadContextGroup_->version.gl >= GlVersion::v46) {
            threadContextGroup_->functions.glMultiDrawElementsIndirectCount   (static_cast<GLenum>(vertexStageInputPrimitiveTopology), static_cast<GLenum>(indexType), reinterpret_cast<const void*>(parameterBufferOffset), countBufferOffset, maxDrawCount, stride);
        } else {
            threadContextGroup_->functions.glMultiDrawElementsIndirectCountARB(static_cast<GLenum>(vertexStageInputPrimitiveTopology), static_cast<GLenum>(indexType), reinterpret_cast<const void*>(parameterBufferOffset), countBufferOffset, maxDrawCount, stride);
        }
    }

    void PipelineRasterization::processPendingChanges() {
        processPendingChangesPipeline();
        PipelineInterface::processPendingChanges();
        processPendingChangesAttributeLayoutAndBuffers();
        threadContext_->processPendingChangesDrawFrame();
        processPendingChangesPipelineRasterization();
        threadContext_->processPendingChangesMemoryBarriersRasterizationRegion();
    }

    void PipelineRasterization::processPendingChangesPipeline() {
        threadContext_->cachedBindShader(id); //pipelineShaderID and pipeline activation are independent! (e.g. setting a uniform will bind the shaderId in the background)
        if (threadContext_->pipeline != this) {
            PipelineInterface::processPendingChangesPipeline();
            buffer_attribute_changedSlotMin = 0;
            buffer_attribute_changedSlotMax = attributeLayoutStates.uppermostActiveBufferIndex;
            stateChange.all = ~0;
            threadContext_->pipeline = this;
        }
    }

    /*
        GL_ARB_viewport_array (Core since 4.1)
            GL_MAX_VIEWPORTS min. 16

            void ViewportArrayv(uint first, sizei count, const float * v);
            void ViewportIndexedf(uint index, float x, float y, float w, float h);
            void ViewportIndexedfv(uint index, const float * v);
            void ScissorArrayv(uint first, sizei count, const int * v);
            void ScissorIndexed(uint index, int left, int bottom, sizei width, sizei height);
            void ScissorIndexedv(uint index, const int * v);
            void DepthRangeArrayv(uint first, sizei count, const clampd * v);
            void DepthRangeIndexed(uint index, clampd n, clampd f);
            void GetFloati_v(enum target, uint index, float *data);
            void GetDoublei_v(enum target, uint index, double *data);


            void EnableIndexedEXT(enum target, uint index);
            void DisableIndexedEXT(enum target, uint index);

            Note that GetIntegerIndexedvEXT, EnableIndexedEXT, DisableIndexedEXT and IsEnabledIndexedEXT are introduced by other OpenGL extensions such as
            EXT_draw_buffers2. If this extension is implemented against an earlier version of OpenGL that does not support GetIntegeri_v and so on, the
            'Indexed' versions of these functions may be used in their place.

        LOGIC OPERATION (Probably not going to implement this)
            There is no per-color-target selection of logicOp in core 4.6! All active color targets will be drawen with the same set logic operation!

            setLogicOperation(LogicOperation logicOperation);

            blocks any blending, so only logicOp OR blending can be used

            Operats on each component (R, G, B, A) and render target seperatly!

            Has no effect on floating point render targets!
            Has no effect on SRGB Frame buffers!

            glEnable (GL_COLOR_LOGIC_OP);
            glDisable(GL_COLOR_LOGIC_OP);
            glLogicOp(LogicOperation logicOperation);
    */
    void PipelineRasterization::processPendingChangesPipelineRasterization() {
        PipelineRasterizationStateChange stateChangeBoth;
        stateChangeBoth.all = threadContext_->stateChange.all | stateChange.all;
        threadContext_->stateChange.all = 0;
                        stateChange.all = 0;

        //Check if PrimitiveTopology is set
        UNLIKELY_IF (vertexStageInputPrimitiveTopology == static_cast<PrimitiveTopology>(0xFFFFFFFF))
            throw std::runtime_error("Trying to issue draw command. But vertexStageInputPrimitiveTopology is not set!");

        /*
        if (bool(change & PipelineRasterizationStateChange::viewportScissor)) {
            glm::uvec4 pending_viewport;

            //TODO: move this to frame setting only
            if (threadContext->viewportFromFrame) {
                threadContext->pending_viewportOffset = {0, 0};
                threadContext->pending_viewportSize   = {threadContext->pending_frame->x, threadContext->pending_frame->y};
            }
            threadContext->cachedViewport();

            glm::uvec4 pending_scissor;
            if (threadContext->scissorFromFrame) {
                threadContext->cachedScissorEnabled(false);
            } else {
                threadContext->cachedViewport();
                threadContext->cachedScissorEnabled(true);
            }
        }

        if (bool(change & PipelineRasterizationStateChange::stencil)) {
            //TODO...
        }
        */

        //FACE FRONT AND CULLING
        if (threadContext_->triangleFrontIsClockwiseRotation != triangleFrontIsClockwiseRotation) {
            threadContextGroup_->functions.glFrontFace(triangleFrontIsClockwiseRotation ? GL_CW : GL_CCW);
            threadContext_->triangleFrontIsClockwiseRotation = triangleFrontIsClockwiseRotation;
        }
        if (threadContext_->faceToDraw != faceToDraw) {
            if (faceToDraw == FaceSelection::frontAndBack) {
                threadContextGroup_->functions.glDisable(GL_CULL_FACE);
            } else {
                threadContextGroup_->functions.glEnable(GL_CULL_FACE);
                if (faceToDraw == FaceSelection::front) {
                    threadContextGroup_->functions.glCullFace(GL_BACK);
                } else {
                    threadContextGroup_->functions.glCullFace(GL_FRONT);
                }
            }
            threadContext_->faceToDraw = faceToDraw;
        }

        //DEPTH
        if (bool(stateChangeBoth.depth)) {
            bool depthEnabled = depthWriteEnabled || depthCompareOperator != CompareOperator::disabled;

            if (threadContext_->depthEnabled != depthEnabled) {
                if (depthEnabled) {
                    threadContextGroup_->functions.glEnable(GL_DEPTH_TEST);
                    threadContext_->depthEnabled = true;
                } else {
                    threadContextGroup_->functions.glDisable(GL_DEPTH_TEST);
                    threadContext_->depthEnabled = false;
                }
            }

            if (depthEnabled) {
                if (threadContext_->depthCompareOperator != depthCompareOperator) {
                    threadContextGroup_->functions.glDepthFunc(static_cast<GLenum>(depthCompareOperator));
                    threadContext_->depthCompareOperator = depthCompareOperator;
                }
                if (threadContext_->depthWriteEnabled != depthWriteEnabled) {
                    threadContextGroup_->functions.glDepthMask(depthWriteEnabled);
                    threadContext_->depthWriteEnabled = depthWriteEnabled;
                }

                //TODO: maybe just always enable this states???
                if (threadContext_->depthBiasConstantFactor != depthBiasConstantFactor
                ||  threadContext_->depthBiasClamp          != depthBiasClamp
                ||  threadContext_->depthBiasSlopeFactor    != depthBiasSlopeFactor
                ) {
                    bool current_usingDepthOffset =
                            threadContext_->depthBiasConstantFactor != 0
                        ||  threadContext_->depthBiasClamp          != 0
                        ||  threadContext_->depthBiasSlopeFactor    != 0;

                    bool pending_usingDepthOffset =
                            depthBiasConstantFactor != 0
                        ||  depthBiasClamp          != 0
                        ||  depthBiasSlopeFactor    != 0;

                    if (pending_usingDepthOffset) {
                        if (threadContextGroup_->extensions.GL_ARB_polygon_offset_clamp) {
                            threadContextGroup_->functions.glPolygonOffsetClamp(depthBiasSlopeFactor, depthBiasConstantFactor, depthBiasClamp);
                        } else {
                            threadContextGroup_->functions.glPolygonOffset     (depthBiasSlopeFactor, depthBiasConstantFactor);
                        }
                    }
                    if (current_usingDepthOffset != pending_usingDepthOffset) {
                        if (pending_usingDepthOffset) {
                            threadContextGroup_->functions.glEnable(GL_POLYGON_OFFSET_FILL);
                            threadContextGroup_->functions.glEnable(GL_POLYGON_OFFSET_LINE);
                            threadContextGroup_->functions.glEnable(GL_POLYGON_OFFSET_POINT);
                        } else {
                            threadContextGroup_->functions.glDisable(GL_POLYGON_OFFSET_FILL);
                            threadContextGroup_->functions.glDisable(GL_POLYGON_OFFSET_LINE);
                            threadContextGroup_->functions.glDisable(GL_POLYGON_OFFSET_POINT);
                        }
                    }
                    threadContext_->depthBiasConstantFactor = depthBiasConstantFactor;
                    threadContext_->depthBiasClamp          = depthBiasClamp;
                    threadContext_->depthBiasSlopeFactor    = depthBiasSlopeFactor;
                }

                if (threadContext_->depthNearMapping != depthNearMapping
                ||  threadContext_->depthFarMapping  != depthFarMapping
                ) {
                    //There also is glDepthRangef, Core since 4.1
                    threadContextGroup_->functions.glDepthRange(depthNearMapping, depthFarMapping);
                    threadContext_->depthNearMapping = depthNearMapping;
                    threadContext_->depthFarMapping  = depthFarMapping;
                }

                if (threadContext_->depthClippingToClamping != depthClippingToClamping) {
                    if (depthClippingToClamping) {
                        threadContextGroup_->functions.glDisable(GL_DEPTH_CLAMP);
                    } else {
                        threadContextGroup_->functions.glEnable(GL_DEPTH_CLAMP);
                    }
                    threadContext_->depthClippingToClamping = depthClippingToClamping;
                }
            }
        }

        //STENCIL
        /*
            All Core since 2.0!
            glEnable(GL_STENCIL_TEST);

            void glStencilFunc
            void glStencilMask
            void glStencilOp

            //same, just that they can set back and front facing triangle funcs. sepperately
            void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
            void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
            void glStencilMaskSeparate(GLenum face, GLuint mask);
                mask is a write mask, only bits set via it (default all set) can/will be changed by stencil operations

            sepperate for front and back face. lines and points use same setting as front.

            stencilMode(testBitMask, compareRefValue, compareOperator)
                                                                    changableBitMask, stencilFailOperator, stencilPassDepthFailOperator, stencilPassDepthPassOrAbsentOperator


            GL_KEEP
                Keeps the current value.
            GL_ZERO
                Sets the stencil buffer value to 0.
            GL_REPLACE
                Sets the stencil buffer value to ref, as specified by glStencilFunc.
            GL_INCR
                Increments the current stencil buffer value. Clamps to the maximum representable unsigned value.
            GL_INCR_WRAP
                Increments the current stencil buffer value. Wraps stencil buffer value to zero when incrementing the maximum representable unsigned value.
            GL_DECR
                Decrements the current stencil buffer value. Clamps to 0.
            GL_DECR_WRAP
                Decrements the current stencil buffer value. Wraps stencil buffer value to the maximum representable unsigned value when decrementing a stencil buffer value of zero.
            GL_INVERT
        */
        if (bool(stateChangeBoth.stencil)) {
            bool stencilEnabled =
                stencilTestFront.compareOperator                       != CompareOperator::disabled
            ||  stencilTestBack.compareOperator                        != CompareOperator::disabled
            ||  stencilWriteFront.stencilFailOperator                  != StencilOperator::keep
            ||  stencilWriteFront.stencilPassDepthFailOperator         != StencilOperator::keep
            ||  stencilWriteFront.stencilPassDepthPassOrAbsentOperator != StencilOperator::keep
            ||  stencilWriteBack.stencilFailOperator                   != StencilOperator::keep
            ||  stencilWriteBack.stencilPassDepthFailOperator          != StencilOperator::keep
            ||  stencilWriteBack.stencilPassDepthPassOrAbsentOperator  != StencilOperator::keep;

            if (stencilEnabled) {
                if (!threadContext_->stencilEnabled) {
                    threadContextGroup_->functions.glEnable(GL_STENCIL_TEST);
                    threadContext_->stencilEnabled = true;
                }
                if (faceToDraw == FaceSelection::frontAndBack || faceToDraw == FaceSelection::front) {
                    if (    threadContext_->stencilTestFront.refValue        != stencilTestFront.refValue
                        ||  threadContext_->stencilTestFront.compareOperator != stencilTestFront.compareOperator
                        ||  threadContext_->stencilTestFront.readMask        != stencilTestFront.readMask
                    ) {
                        threadContextGroup_->functions.glStencilFuncSeparate(
                            GL_FRONT,
                            static_cast<GLenum>(stencilTestFront.compareOperator),
                            stencilTestFront.refValue,
                            stencilTestFront.readMask);
                        threadContext_->stencilTestFront.refValue        = stencilTestFront.refValue;
                        threadContext_->stencilTestFront.compareOperator = stencilTestFront.compareOperator;
                        threadContext_->stencilTestFront.readMask        = stencilTestFront.readMask;
                    }
                    if (threadContext_->stencilWriteFront.writeMask != stencilWriteFront.writeMask) {
                        threadContextGroup_->functions.glStencilMaskSeparate(GL_FRONT, stencilWriteFront.writeMask);
                        threadContext_->stencilWriteFront.writeMask = stencilWriteFront.writeMask;
                    }
                    if (    threadContext_->stencilWriteFront.stencilFailOperator                  != stencilWriteFront.stencilFailOperator
                        ||  threadContext_->stencilWriteFront.stencilPassDepthFailOperator         != stencilWriteFront.stencilPassDepthFailOperator
                        ||  threadContext_->stencilWriteFront.stencilPassDepthPassOrAbsentOperator != stencilWriteFront.stencilPassDepthPassOrAbsentOperator
                    ) {
                        threadContextGroup_->functions.glStencilOpSeparate(
                            GL_FRONT,
                            static_cast<GLenum>(stencilWriteFront.stencilFailOperator),
                            static_cast<GLenum>(stencilWriteFront.stencilPassDepthFailOperator),
                            static_cast<GLenum>(stencilWriteFront.stencilPassDepthPassOrAbsentOperator));
                        threadContext_->stencilWriteFront.stencilFailOperator                  = stencilWriteFront.stencilFailOperator;
                        threadContext_->stencilWriteFront.stencilPassDepthFailOperator         = stencilWriteFront.stencilPassDepthFailOperator;
                        threadContext_->stencilWriteFront.stencilPassDepthPassOrAbsentOperator = stencilWriteFront.stencilPassDepthPassOrAbsentOperator;
                    }
                }
                if (faceToDraw == FaceSelection::frontAndBack || faceToDraw == FaceSelection::back) {
                    if (    threadContext_->stencilTestBack.refValue        != stencilTestBack.refValue
                        ||  threadContext_->stencilTestBack.compareOperator != stencilTestBack.compareOperator
                        ||  threadContext_->stencilTestBack.readMask        != stencilTestBack.readMask
                    ) {
                        threadContextGroup_->functions.glStencilFuncSeparate(
                            GL_BACK,
                            static_cast<GLenum>(stencilTestBack.compareOperator),
                            stencilTestBack.refValue,
                            stencilTestBack.readMask);
                        threadContext_->stencilTestBack.refValue        = stencilTestBack.refValue;
                        threadContext_->stencilTestBack.compareOperator = stencilTestBack.compareOperator;
                        threadContext_->stencilTestBack.readMask        = stencilTestBack.readMask;
                    }
                    if (threadContext_->stencilWriteBack.writeMask != stencilWriteBack.writeMask) {
                        threadContextGroup_->functions.glStencilMaskSeparate(GL_BACK, stencilWriteBack.writeMask);
                        threadContext_->stencilWriteBack.writeMask = stencilWriteBack.writeMask;
                    }
                    if (    threadContext_->stencilWriteBack.stencilFailOperator                  != stencilWriteBack.stencilFailOperator
                        ||  threadContext_->stencilWriteBack.stencilPassDepthFailOperator         != stencilWriteBack.stencilPassDepthFailOperator
                        ||  threadContext_->stencilWriteBack.stencilPassDepthPassOrAbsentOperator != stencilWriteBack.stencilPassDepthPassOrAbsentOperator
                    ) {
                        threadContextGroup_->functions.glStencilOpSeparate(
                            GL_BACK,
                            static_cast<GLenum>(stencilWriteBack.stencilFailOperator),
                            static_cast<GLenum>(stencilWriteBack.stencilPassDepthFailOperator),
                            static_cast<GLenum>(stencilWriteBack.stencilPassDepthPassOrAbsentOperator));
                        threadContext_->stencilWriteBack.stencilFailOperator                  = stencilWriteBack.stencilFailOperator;
                        threadContext_->stencilWriteBack.stencilPassDepthFailOperator         = stencilWriteBack.stencilPassDepthFailOperator;
                        threadContext_->stencilWriteBack.stencilPassDepthPassOrAbsentOperator = stencilWriteBack.stencilPassDepthPassOrAbsentOperator;
                    }
                }
            } else {
                if (threadContext_->stencilEnabled) {
                    threadContextGroup_->functions.glDisable(GL_STENCIL_TEST);
                    threadContext_->stencilEnabled = false;
                }
            }
        }

        //RGBA WRITE MASK
        //glColorMask  core since 2.0
        //glColorMaski core since 3.0
        if (singleRgbaWriteMaskState) {
            if (!threadContext_->singleRgbaWriteMaskState || threadContext_->rgbaWriteMask[0].value != rgbaWriteMask[0].value) {
                threadContextGroup_->functions.glColorMask(rgbaWriteMask[0].r, rgbaWriteMask[0].g, rgbaWriteMask[0].b, rgbaWriteMask[0].a);
                LOOPI(config::MAX_RGBA_ATTACHMENTS) {
                    threadContext_->rgbaWriteMask[i].value = rgbaWriteMask[i].value;
                }
            }
        } else {
            LOOPI(config::MAX_RGBA_ATTACHMENTS) if (threadContext_->rgbaWriteMask[i].value != rgbaWriteMask[i].value) {
                threadContextGroup_->functions.glColorMaski(i, rgbaWriteMask[i].r, rgbaWriteMask[i].g, rgbaWriteMask[i].b, rgbaWriteMask[i].a);
                threadContext_->rgbaWriteMask[i].value = rgbaWriteMask[i].value;
            }
        }

        /*
            rgba blending
            glEnablei(GL_BLEND, i)
            void blendColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
            void blendEquation(GLenum modeRGB, GLenum modeAlpha);
            void blendEquation(GLenum modeRGB, GLenum modeAlpha, GLuint drawSlot);
            void blendFunc(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
            void blendFunc(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha, GLuint drawSlot);

            NOTE: GL_ARB_draw_buffers_blend (Core since 4.0) has ARB function names, and core function names!
            Note that the glEnablei/glDisablei functions are not part of this functionality. The ability to enable/disable blending to specific buffers is core 3.0 functionality.
            Whereas draw buffers blend is about the ability to specify different blend parameters for different buffers.
        */
        if (bool(stateChangeBoth.blend)) {
            blendEnabledAny = any_of(begin(blendEnabled), end(blendEnabled), [](bool b){return b;});
            if (blendEnabledAny.isTrue()) {
                threadContext_->blendEnabledAny = true;
                if (blendEnabledAll.isUnknown())
                    blendEnabledAll = all_of(begin(blendEnabled), end(blendEnabled), [](bool b){return b;});
                if (blendEnabledAll.isTrue()) {
                    for (bool& e : threadContext_->blendEnabled) e = true;
                    threadContextGroup_->functions.glEnable(GL_BLEND);
                } else {
                    LOOPI(config::MAX_RGBA_ATTACHMENTS) {
                        if (threadContext_->blendEnabled[i] != blendEnabled[i]) {
                            threadContext_->blendEnabled[i] = blendEnabled[i];
                            if (blendEnabled[i])
                                threadContextGroup_->functions.glEnablei(GL_BLEND, i);
                            else
                                threadContextGroup_->functions.glDisablei(GL_BLEND, i);
                        }
                    }
                }
                if (threadContext_->blendConstRgba != blendConstRgba) {
                    threadContext_->blendConstRgba = blendConstRgba;
                    threadContextGroup_->functions.glBlendColor(blendConstRgba.r, blendConstRgba.g, blendConstRgba.b, blendConstRgba.a);
                }
                int firstActiveIndex = 0;
                for (;firstActiveIndex < config::MAX_RGBA_ATTACHMENTS; firstActiveIndex++) {
                    if (blendEnabled[firstActiveIndex]) break;
                }
                if (blendModesUniform.isUnknown()) {
                    blendModesUniform = true;
                    for (int i = firstActiveIndex + 1; i < config::MAX_RGBA_ATTACHMENTS; i++) {
                        if (blendEnabled[i]) {
                            if (    blendFactors  [i] != blendFactors  [firstActiveIndex]
                                ||  blendEquations[i] != blendEquations[firstActiveIndex]
                            ) {
                                blendModesUniform = false;
                                break;
                            }
                        }
                    }
                }
                if (blendModesUniform.isTrue()) {
                    bool blendFactorsChanged   = true;
                    bool blendEquationsChanged = true;
                    if (threadContext_->blendModesUniform) {
                        if (threadContext_->blendFactors  [0] == blendFactors  [firstActiveIndex]) blendFactorsChanged   = false;
                        if (threadContext_->blendEquations[0] == blendEquations[firstActiveIndex]) blendEquationsChanged = false;
                    }
                    if (blendFactorsChanged) {
                        LOOPI(config::MAX_RGBA_ATTACHMENTS)
                            threadContext_->blendFactors[i] = blendFactors[i];
                        threadContextGroup_->functions.glBlendFuncSeparate(
                            static_cast<GLenum>(blendFactors[firstActiveIndex].srcRgb),
                            static_cast<GLenum>(blendFactors[firstActiveIndex].srcA),
                            static_cast<GLenum>(blendFactors[firstActiveIndex].dstRgb),
                            static_cast<GLenum>(blendFactors[firstActiveIndex].dstA)
                        );
                    }
                    if (blendEquationsChanged) {
                        LOOPI(config::MAX_RGBA_ATTACHMENTS)
                            threadContext_->blendEquations[i] = blendEquations[i];
                        threadContextGroup_->functions.glBlendEquationSeparate(
                            static_cast<GLenum>(blendEquations[firstActiveIndex].rgb),
                            static_cast<GLenum>(blendEquations[firstActiveIndex].a)
                        );
                    }
                    threadContext_->blendModesUniform = true;
                } else {
                    //TODO: implement ARB version, too
                    //UNLIKELY_IF (!threadContextGroup_->extensions.GL_ARB_draw_buffers_blend)
                    //    throw std::runtime_error("Trying to set multible rgba blend factors/equations, but not supported by this system (missing GL_ARB_draw_buffers_blend (Core since 4.0))");
                    UNLIKELY_IF (!(threadContextGroup_->version.gl >= GlVersion::v40))
                        throw std::runtime_error("Trying to set multible rgba blend factors/equations, but not supported by this system (missing OpenGL 4.0 or higher)");
                    for (int i = firstActiveIndex; i < config::MAX_RGBA_ATTACHMENTS; i++) {
                        if (threadContext_->blendFactors[i] != blendFactors[i]) {
                            threadContext_->blendFactors[i] = blendFactors[i];
                            threadContextGroup_->functions.glBlendFuncSeparatei(
                                i,
                                static_cast<GLenum>(blendFactors[i].srcRgb),
                                static_cast<GLenum>(blendFactors[i].srcA),
                                static_cast<GLenum>(blendFactors[i].dstRgb),
                                static_cast<GLenum>(blendFactors[i].dstA)
                            );
                        }
                        if (threadContext_->blendEquations[i] != blendEquations[i]) {
                            threadContext_->blendEquations[i] = blendEquations[i];
                            threadContextGroup_->functions.glBlendEquationSeparatei(
                                i,
                                static_cast<GLenum>(blendEquations[i].rgb),
                                static_cast<GLenum>(blendEquations[i].a)
                            );
                        }
                    }
                    threadContext_->blendModesUniform = false;
                }
            } else {
                if (threadContext_->blendEnabledAny) {
                    threadContext_->blendEnabledAny = false;
                    LOOPI(config::MAX_RGBA_ATTACHMENTS)
                        threadContext_->blendEnabled[i] = false;
                    threadContextGroup_->functions.glDisable(GL_BLEND);
                }
            }
        }

        //MULTISAMPLE
        if (threadContext_->multiSample != multiSample) {
            threadContext_->multiSample = multiSample;
            if (multiSample)
                threadContextGroup_->functions.glEnable(GL_MULTISAMPLE);
            else
                threadContextGroup_->functions.glDisable(GL_MULTISAMPLE);
        }
    }

    /**
        NOTE:
            At the moment this does extreme aggressive state tracking. E.g. attribute locations that are not used in the active shader will not be changed here AT ALL. (including not disabled!!!)
            This probably will cause issues with some drivers! :(

        Internals:
            GL_ARB_instanced_arrays      (core since 3.3) NOTE: non core uses EXT/ANGLE/NV functions!
            GL_EXT_instanced_arrays      (is a gles2 extension)
            GL_NV_instanced_arrays
            GL_ANGLE_instanced_arrays

            GL_ARB_vertex_attrib_binding (Core since 4.3)
            GL_ARB_multi_bind            (Core since 4.4)


            glVertexAttribDivisor exist as core/ARB/EXT/NV/ANGLE
            glVertexAttribDivisor  sets instance divisor for a attribute location
            glVertexBindingDivisor sets instance divisor for a buffer index (ARB_vertex_attrib_binding Core since 4.3)
    */
    void PipelineRasterization::processPendingChangesAttributeLayoutAndBuffers() {
        if (threadContextGroup_->extensions.GL_ARB_vertex_attrib_binding) {
            if (threadContext_->attributeLayoutChanged) {
              //int uppermostActiveBufferIndex = max(threadContext->attributeLayoutStates.uppermostActiveBufferIndex, attributeLayoutStates.uppermostActiveBufferIndex);
                int uppermostActiveBufferIndex = attributeLayoutStates.uppermostActiveBufferIndex;
                LOOPI(uppermostActiveBufferIndex + 1) {
                    auto& currentInstancing = threadContext_->attributeLayoutStates.instancing[i];
                    auto& pendingInstancing =                        attributeLayoutStates.instancing[i];
                    if (currentInstancing != pendingInstancing) {
                        threadContextGroup_->functions.glVertexBindingDivisor(i, pendingInstancing);
                        currentInstancing = pendingInstancing;
                    }
                }
              //int uppermostActiveLocation = max(threadContext->attributeLayoutStates.uppermostActiveLocation, attributeLayoutStates.uppermostActiveLocation);
                int uppermostActiveLocation = attributeLayoutStates.uppermostActiveLocation;
                LOOPI(uppermostActiveLocation + 1) {
                    auto& currentLoc = threadContext_->attributeLayoutStates.location[i];
                    auto& pendingLoc =                        attributeLayoutStates.location[i];
                    if (pendingLoc.usage == AttributeLayoutStates::Usage::enabled) {
                        if (currentLoc.usage != AttributeLayoutStates::Usage::enabled) {
                            threadContextGroup_->functions.glEnableVertexAttribArray(i);
                            currentLoc.usage = AttributeLayoutStates::Usage::enabled;
                        }
                        if (currentLoc.bufferIndex != pendingLoc.bufferIndex) {
                            threadContextGroup_->functions.glVertexAttribBinding(i, pendingLoc.bufferIndex);
                            currentLoc.bufferIndex = pendingLoc.bufferIndex;
                        }
                        if (currentLoc.attributeFormat != pendingLoc.attributeFormat
                        ||  currentLoc.offset          != pendingLoc.offset
                        ||  currentLoc.gpuType         != pendingLoc.gpuType)
                        {
                            auto& af = pendingLoc.attributeFormat;
                            switch (pendingLoc.gpuType) {
                                case AttributeLayoutStates::GpuType::f32: threadContextGroup_->functions.glVertexAttribFormat (i, af->componentsCountOrBGRA, af->componentsType, af->normalized, pendingLoc.offset); break;
                                case AttributeLayoutStates::GpuType::i32: threadContextGroup_->functions.glVertexAttribIFormat(i, af->componentsCountOrBGRA, af->componentsType,                 pendingLoc.offset); break;
                                case AttributeLayoutStates::GpuType::f64: threadContextGroup_->functions.glVertexAttribLFormat(i, af->componentsCountOrBGRA, af->componentsType,                 pendingLoc.offset); break;
                                case AttributeLayoutStates::GpuType::unknown: break;
                            }
                            currentLoc.attributeFormat = pendingLoc.attributeFormat;
                            currentLoc.offset          = pendingLoc.offset;
                            currentLoc.gpuType         = pendingLoc.gpuType;
                        }
                    } else if (pendingLoc.usage == AttributeLayoutStates::Usage::disabled) {
                        if (currentLoc.usage != AttributeLayoutStates::Usage::disabled) {
                            threadContextGroup_->functions.glDisableVertexAttribArray(i);
                            currentLoc.usage = AttributeLayoutStates::Usage::disabled;
                        }
                    } else {
                        //indifferent attribute state because shader does not use attribute location at all so we do not need to change anything.
                    }
                }
              //threadContext->attributeLayoutStates.uppermostActiveLocation = attributeLayoutStates.uppermostActiveLocation;
                threadContext_->attributeLayoutChanged = false;
            }

            int8_t changedSlotMin = buffer_attribute_changedSlotMin;
            int8_t changedSlotMax = buffer_attribute_changedSlotMax;

            //ignore states of buffer index if not used by shader layout
            changedSlotMax = std::min(changedSlotMax, attributeLayoutStates.uppermostActiveBufferIndex);

            if (changedSlotMin <= changedSlotMax) {
                if (threadContextGroup_->extensions.GL_ARB_multi_bind) {
                    int first = changedSlotMin;
                    int last  = changedSlotMax;

                    //Filter out unchanged slots at the beginning and end of the list
                    //Have to test how much performance impact (good or bad) this has

                    //while (first <= last) {
                    //  if (current_buffer_vertex_id                       [first] != pending_buffer_vertex_id                       [first]
                    //  ||  current_buffer_vertex_offset                   [first] != pending_buffer_vertex_offset                   [first]
                    //    ||  current_vertexAttributeLayout.bufferIndexStride[first] != pending_vertexAttributeLayout.bufferIndexStride[first]) break;
                    //    first++;
                    //}
                    //while (first <= last) {
                    //    if (current_buffer_vertex_id                       [last] != pending_buffer_vertex_id                       [last]
                    //    ||  current_buffer_vertex_offset                   [last] != pending_buffer_vertex_offset                   [last]
                    //    ||  current_vertexAttributeLayout.bufferIndexStride[last] != pending_vertexAttributeLayout.bufferIndexStride[last]) break;
                    //    last--;
                    //}

                    //first = 255;
                    //last  = -1;
                    //for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                    //    if (
                    //            current_buffer_vertex_id                       [i] != pending_buffer_vertex_id                       [i]
                    //        ||  current_buffer_vertex_offset                   [i] != pending_buffer_vertex_offset                   [i]
                    //        ||  current_vertexAttributeLayout.bufferIndexStride[i] != pending_vertexAttributeLayout.bufferIndexStride[i]
                    //    ){
                    //        first = min(first, i);
                    //        last  = max(last,  i);
                    //    }
                    //}

                    //first = 0;
                    //last  = 28;//config::MAX_ATTRIBUTES - 1;  //if value is higher then 28, the Win7 AMD blobs break without error output
                    if (first <= last) {
                        const uint32_t  count        = last - first + 1;
                        const uint32_t* bufferIdList =                                   &buffer_attribute_id                    [first];
                        const GLintptr* offsetList   = reinterpret_cast<const GLintptr*>(&buffer_attribute_offset                [first]);
                        const GLsizei*  strideList   = reinterpret_cast<const GLsizei* >(&attributeLayoutStates.bufferIndexStride[first]);
                        threadContextGroup_->functions.glBindVertexBuffers(first, count, bufferIdList, offsetList, strideList);

                        for (int i = first; i <= last; ++i) {
                            threadContext_->buffer_attribute_id    [i] = buffer_attribute_id    [i];
                            threadContext_->buffer_attribute_offset[i] = buffer_attribute_offset[i];
                        }
                    }
                } else {
                    for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                        if (threadContext_->buffer_attribute_id                    [i] != buffer_attribute_id                    [i]
                        ||  threadContext_->buffer_attribute_offset                [i] != buffer_attribute_offset                [i]
                        ||  threadContext_->attributeLayoutStates.bufferIndexStride[i] != attributeLayoutStates.bufferIndexStride[i]) {
                            threadContextGroup_->functions.glBindVertexBuffer(i, buffer_attribute_id[i], buffer_attribute_offset[i], attributeLayoutStates.bufferIndexStride[i]);
                            threadContext_->buffer_attribute_id                    [i] = buffer_attribute_id                     [i];
                            threadContext_->buffer_attribute_offset                [i] = buffer_attribute_offset                 [i];
                            threadContext_->attributeLayoutStates.bufferIndexStride[i] = attributeLayoutStates.bufferIndexStride [i];
                        }
                    }
                }
                buffer_attribute_changedSlotMin = config::MAX_ATTRIBUTES;
                buffer_attribute_changedSlotMax = -1;
            }
        } else {
            //This path gets used when there is no GL_ARB_vertex_attrib_binding (There is also no use of GL_ARB_multi_bind here)
            int8_t changedSlotMin = buffer_attribute_changedSlotMin;
            int8_t changedSlotMax = buffer_attribute_changedSlotMax;

            //ignore states of buffer index if not used by shader layout
            changedSlotMax = std::min(changedSlotMax, attributeLayoutStates.uppermostActiveBufferIndex);

            if (threadContext_->attributeLayoutChanged) {
              //int uppermostActiveLocation = max(max(attributeLayoutStates.uppermostActiveLocation, attributeLayoutStates.uppermostActiveLocation), changedSlotMax);
                int uppermostActiveLocation = attributeLayoutStates.uppermostActiveLocation;
                LOOPI(uppermostActiveLocation + 1) {
                          auto& currentLoc = threadContext_->attributeLayoutStates.location[i];
                    const auto& pendingLoc =                 attributeLayoutStates.location[i];

                    if (pendingLoc.usage == AttributeLayoutStates::Usage::enabled) {
                        if (currentLoc.usage != AttributeLayoutStates::Usage::enabled) {
                            threadContextGroup_->functions.glEnableVertexAttribArray(i);
                            currentLoc.usage = AttributeLayoutStates::Usage::enabled;
                        }
                        const uint32_t pendingBufferId =                 buffer_attribute_id                    [pendingLoc.bufferIndex];
                        const intptr_t pendingOffset   =                 buffer_attribute_offset                [pendingLoc.bufferIndex] + pendingLoc.offset;
                        const uint32_t pendingStride   =                 attributeLayoutStates.bufferIndexStride[pendingLoc.bufferIndex];
                        const uint32_t currentBufferId = threadContext_->buffer_attribute_id                    [currentLoc.bufferIndex];
                        const intptr_t currentOffset   = threadContext_->buffer_attribute_offset                [currentLoc.bufferIndex] + currentLoc.offset;
                        const uint32_t currentStride   = threadContext_->attributeLayoutStates.bufferIndexStride[currentLoc.bufferIndex];
                        if (currentLoc.attributeFormat != pendingLoc.attributeFormat
                        ||  currentLoc.gpuType         != pendingLoc.gpuType
                        ||  currentBufferId            != pendingBufferId
                        ||  currentOffset              != pendingOffset
                        ||  currentStride              != pendingStride)
                        {
                            const auto& af = pendingLoc.attributeFormat;
                            threadContext_->cachedBindArrayBuffer(pendingBufferId);
                            switch (pendingLoc.gpuType) {
                                case AttributeLayoutStates::GpuType::f32: threadContextGroup_->functions.glVertexAttribPointer (i, af->componentsCountOrBGRA, af->componentsType, af->normalized, pendingStride, reinterpret_cast<const void*>(pendingOffset)); break;
                                case AttributeLayoutStates::GpuType::i32: threadContextGroup_->functions.glVertexAttribIPointer(i, af->componentsCountOrBGRA, af->componentsType,                 pendingStride, reinterpret_cast<const void*>(pendingOffset)); break;
                                case AttributeLayoutStates::GpuType::f64: threadContextGroup_->functions.glVertexAttribLPointer(i, af->componentsCountOrBGRA, af->componentsType,                 pendingStride, reinterpret_cast<const void*>(pendingOffset)); break;
                                case AttributeLayoutStates::GpuType::unknown: break;
                            }
                            currentLoc = pendingLoc;
                            threadContext_->buffer_attribute_id                    [currentLoc.bufferIndex] =                       buffer_attribute_id    [pendingLoc.bufferIndex];
                            threadContext_->buffer_attribute_offset                [currentLoc.bufferIndex] =                       buffer_attribute_offset[pendingLoc.bufferIndex];
                            threadContext_->attributeLayoutStates.bufferIndexStride[currentLoc.bufferIndex] = attributeLayoutStates.bufferIndexStride      [pendingLoc.bufferIndex];
                        }
                              auto& currentInstancing = threadContext_->attributeLayoutStates.instancing[i];
                        const auto& pendingInstancing =                 attributeLayoutStates.instancing[i];
                        if (currentInstancing != pendingInstancing) {
                            threadContextGroup_->functions.glVertexAttribDivisor(i, pendingInstancing);
                            currentInstancing = pendingInstancing;
                        }
                    } else if (pendingLoc.usage == AttributeLayoutStates::Usage::disabled) {
                        if (currentLoc.usage != AttributeLayoutStates::Usage::disabled) {
                            threadContextGroup_->functions.glDisableVertexAttribArray(i);
                            currentLoc.usage = AttributeLayoutStates::Usage::disabled;
                        }
                    } else {
                        //indifferent attribute state because shader does not use attribute location at all. So we do not need to change anything!
                    }
                }
                buffer_attribute_changedSlotMin = config::MAX_ATTRIBUTES;
                buffer_attribute_changedSlotMax = -1;
                threadContext_->attributeLayoutChanged = false;
            } else if (changedSlotMax >= changedSlotMin) {
                LOOPI(attributeLayoutStates.uppermostActiveLocation + 1) {
                    const auto& currentLoc = threadContext_->attributeLayoutStates.location[i];
                    const auto& pendingLoc =                 attributeLayoutStates.location[i];

                    if (pendingLoc.usage == AttributeLayoutStates::Usage::indifferent) continue;

                    const GLintptr pendingOffset   =                 buffer_attribute_offset                [currentLoc.bufferIndex] + currentLoc.offset;
                    const GLintptr currentOffset   = threadContext_->buffer_attribute_offset                [currentLoc.bufferIndex] + currentLoc.offset;
                    const GLsizei  currentStride   = threadContext_->attributeLayoutStates.bufferIndexStride[currentLoc.bufferIndex];

                    const uint32_t currentBufferId = threadContext_->buffer_attribute_id                    [currentLoc.bufferIndex];
                    const uint32_t pendingBufferId =                 buffer_attribute_id                    [currentLoc.bufferIndex];

                    if ((currentLoc.bufferIndex >= changedSlotMin && currentLoc.bufferIndex <= changedSlotMax)
                        ||  currentBufferId != pendingBufferId
                        ||  currentOffset   != pendingOffset)
                    {
                        const auto& af = pendingLoc.attributeFormat;
                        threadContext_->cachedBindArrayBuffer(pendingBufferId);
                        switch (currentLoc.gpuType) {
                            case AttributeLayoutStates::GpuType::f32: threadContextGroup_->functions.glVertexAttribPointer (i, af->componentsCountOrBGRA, af->componentsType, af->normalized, currentStride, reinterpret_cast<const void*>(pendingOffset)); break;
                            case AttributeLayoutStates::GpuType::i32: threadContextGroup_->functions.glVertexAttribIPointer(i, af->componentsCountOrBGRA, af->componentsType,                 currentStride, reinterpret_cast<const void*>(pendingOffset)); break;
                            case AttributeLayoutStates::GpuType::f64: threadContextGroup_->functions.glVertexAttribLPointer(i, af->componentsCountOrBGRA, af->componentsType,                 currentStride, reinterpret_cast<const void*>(pendingOffset)); break;
                            case AttributeLayoutStates::GpuType::unknown: break;
                        }
                    }
                }
                for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                    threadContext_->buffer_attribute_id    [i] = buffer_attribute_id    [i];
                    threadContext_->buffer_attribute_offset[i] = buffer_attribute_offset[i];
                }
                buffer_attribute_changedSlotMin = config::MAX_ATTRIBUTES;
                buffer_attribute_changedSlotMax = -1;
            }
        }
    }

    /*
        This returns a multi-line string that identificates/describes a shader/pipeline object for warning or error messages.
    */
    string PipelineRasterization::getPipelineIdentificationString() {
        //typeid.name() does not return perfectly identifiable class names, but close enough for now
        string s = string("")
            + "typeid.name: \"" + typeid(*this).name() + "\" based on glCompact::PipelineRasterization\n";
        if (loadedFromFiles) {
            s += " loaded from this files:\n";
            LOOPI(5) if (hasShader[i]) s += "  " + shaderTypeStringSameLenght[i] + ": \"" + fileName[i] + "\"\n";
        } else {
            s += " loaded from strings\n";
            //TODO: Output whole shader here? No real other way to identificate and be clear about what caused error/warning?
            //Could be an issue with spamming to much on screen/log. Or make full output a one-time event for the application runtime?
            //Also would need clear documentation for users that may not want to output glsl to the world.
        }
        return s;
    }

    string PipelineRasterization::getPipelineInformationQueryString() {
        string s;
        if (attributeLayoutStates.uppermostActiveLocation > -1) {
            s += " attributes:\n";
            LOOPI(attributeLayoutStates.uppermostActiveLocation + 1) {
                if (attributeLocationInfo[i].type) {
                    s += "  (location = " + to_string(i) + ") " + gl::typeToGlslAndCNameString(attributeLocationInfo[i].type) + " " + attributeLocationInfo[i].name + "\n";
                }
            }
        }
        return(s + PipelineInterface::getPipelineInformationQueryString());
    }

    const std::string PipelineRasterization::shaderTypeString[] = {
        "VERTEX",
        "TESS CONTROL",
        "TESS EVALUATION",
        "GEOMETRY",
        "FRAGMENT"
    };

    const std::string PipelineRasterization::shaderTypeStringSameLenght[] = {
        "VERTEX         ",
        "TESS CONTROL   ",
        "TESS EVALUATION",
        "GEOMETRY       ",
        "FRAGMENT       "
    };
}
