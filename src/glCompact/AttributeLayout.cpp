#include "glCompact/AttributeLayout.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/threadContextGroup_.hpp"

#include <algorithm> //win for std::max / min

#include "glCompact/Tools_.hpp"
#include "glCompact/GlTools.hpp"

#include <stdexcept>

/*
    Instancing is Core since 3.1, and before that also available via extension like GL_ARB_draw_instanced


    TODO: include other gpu types like
    ARB_vertex_attrib_64bit (core since 4.1)


    GL_MAX_VERTEX_ATTRIBS (Min. 16?)
    GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET (Min. 2047?)

    No GL_FIXED for now!

    TODO: warn for non alignen data. eg first 3bytes and then a float (bad performance for the non aligned float!)
    http://www.opengl.org/wiki/Common_Mistakes:_Deprecated  does this still matter?
*/

/**
    (Core in 2.0)
    GL_MAX_VERTEX_ATTRIBS

    GL_MAX_ELEMENTS_VERTICES is just a performance hint! Buffer will work with more vertices. On modern hardware not relevant at all! GL ES 1.1 even removed it completly!

    GL_ARB_vertex_attrib_64bit needs to be pressend to use 64bit (double) type on gpu side (can always be source format?)

    with EXT_direct_state_access_memory pressent:
    void VertexArrayVertexAttribLOffsetEXT(uint vaobj, uint buffer, uint index, int size, enum type, sizei stride, intptr offset);
*/

/**
ARB_vertex_attrib_binding (Core since 4.3)
    void BindVertexBuffer(uint bindingindex, uint buffer, intptr offset, sizei stride);
    void VertexAttribFormat(uint attribindex, int size, enum type, boolean normalized, uint relativeoffset);
    void VertexAttribIFormat(uint attribindex, int size, enum type, uint relativeoffset);
    void VertexAttribLFormat(uint attribindex, int size, enum type, uint relativeoffset);
    void VertexAttribBinding(uint attribindex, uint bindingindex);
    void VertexBindingDivisor(uint bindingindex, uint divisor);
*/

/**
    ARB_draw_elements_base_vertex (core in GL3.2)
    GL_MAX_VARYING_COMPONENTS (min. 60)
    Quote: "The number of available locations is implementation-defined, and it cannot be queried. However, it will be at least one-fourth of GL_MAX_VARYING_COMPONENTS​." so always a minimum of 15 layout locations?
    GL_MAX_VARYING_VECTORS (min. 15 (I guess the "hidden" one is vec4 gl_Position))
    GL_MAX_VERTEX_ATTRIBS (min. 16)
*/

/*
    glVertexAttribPointer exist since GL 2.0
    in 3.0 it was depricated to use pointers to client side memory.

    If we ever want to run on lower then GL3.0 hardware/drivers. We could use glCompact::Buffer that use client side memory, as vertex source transparently.
*/

/*
    set that is bound to specific shader.

    also may need extra function for different mat. Like mat3x3, dmat4x4, etc...
    See: https://www.khronos.org/opengl/wiki/Vertex_Specification#Matrix_attributes


    What is with Packed Vertex Data Formats?
     INT_2_10_10_10_REV
     UNSIGNED_INT_2_10_10_10_REV
     UNSIGNED_INT_10F_11F_11F_REV

    What is with BGRA reordering option for vec4? (Swizzeling could just be done in shader!)

    Mat type | using X attribute locations
    Mat2xX     2
    Mat3xX     3
    Mat4xX     4
*/

using namespace std;

namespace glCompact {
    /**
     * \ingroup API
     * \class glCompact::AttributeLayout
     * \brief Attribute data input layout class to be used with PipelineRasterization
     * \details bla...
     */

    AttributeLayout::AttributeLayout() {
    }

    AttributeLayout::~AttributeLayout() {
    }

    void AttributeLayout::addBufferIndex() {
        addBufferIndex_(false);
    }

    /**
     *
     */
    void AttributeLayout::addBufferIndexWithInstancing() {
        addBufferIndex_(true);
    }

    void AttributeLayout::addBufferIndex_(
        bool instancing
    ) {
        UNLIKELY_IF (uppermostActiveBufferIndex >= config::MAX_ATTRIBUTES)
            throw std::runtime_error("Trying to define buffer index beyond config::MAX_ATTRIBUTES (" + to_string(config::MAX_ATTRIBUTES) + ")");
        auto currentBufferIndex = ++uppermostActiveBufferIndex;
        bufferIndex[currentBufferIndex].instancing = instancing;
    }

    void AttributeLayout::addLocation(
        uint8_t         location,
        AttributeFormat attributeFormat
    ) {
        auto  currentBufferIndex  = uppermostActiveBufferIndex;
        auto& currentBufferOffset = bufferIndex[currentBufferIndex].stride;
        UNLIKELY_IF (currentBufferIndex == -1)
            throw runtime_error("Need to add a buffer index before any vertex location can be defined!");
        UNLIKELY_IF (this->location[location].attributeFormat != AttributeFormat::NONE)
            throw runtime_error("Trying to define already defined attribute location (" + to_string(location) + ")");
        UNLIKELY_IF (location >  config::MAX_ATTRIBUTES)
            throw runtime_error("Trying to define location (" + to_string(location) + ") beyond config::MAX_ATTRIBUTES (" + to_string(config::MAX_ATTRIBUTES) + ")");
        UNLIKELY_IF (location >= threadContextGroup_->values.GL_MAX_VERTEX_ATTRIBS)
            throw runtime_error("Trying to set AttributeLayout that has attribute location (" + to_string(location) + ") that goes beyond implementation limit. GL_MAX_VERTEX_ATTRIBS(" + to_string(threadContextGroup_->values.GL_MAX_VERTEX_ATTRIBS) + " = 0.." + to_string(threadContextGroup_->values.GL_MAX_VERTEX_ATTRIBS - 1));

        Location &loc           =  this->location[location];
        loc.attributeFormat     =  attributeFormat;
        loc.offset              =  currentBufferOffset;
        loc.bufferIndex         =  currentBufferIndex;
        uppermostActiveLocation =  std::max<int32_t>(uppermostActiveLocation, location);
        currentBufferOffset     += attributeFormat->byteSize;
    }

    void AttributeLayout::addSpacing(
        uint16_t byteSize
    ) {
        UNLIKELY_IF (uppermostActiveBufferIndex < 0)
            throw runtime_error("Need to add a buffer index before any spacing can be added!");
        auto  currentBufferIndex  = uppermostActiveBufferIndex;
        auto& currentBufferOffset = bufferIndex[currentBufferIndex].stride;
        currentBufferOffset += byteSize;
    }

    void AttributeLayout::addSpacing(
        AttributeFormat attributeFormat
    ) {
        addSpacing(attributeFormat->byteSize);
    }

    void AttributeLayout::reset() {
        uppermostActiveLocation    = -1;
        uppermostActiveBufferIndex = -1;
        LOOPI(config::MAX_ATTRIBUTES) {
            bufferIndex[i].stride     = 0;
            bufferIndex[i].instancing = 0;
        }
        LOOPI(config::MAX_ATTRIBUTES) {
            location[i].attributeFormat = AttributeFormat::NONE;
            location[i].offset          = 0;
            location[i].bufferIndex     = 0;
        }
    }

    /*void checkOnceOnFirstUse() {
        if (bufferIndexCount >= threadContext->value.GL_MAX_VERTEX_ATTRIB_BINDINGS) throw std::runtime_error("To many attribute buffer bindings (GL_MAX_VERTEX_ATTRIB_BINDINGS = " + std::toString(threadContext->value.GL_MAX_VERTEX_ATTRIB_BINDINGS) + ")");
        if (location >= threadContext->value.GL_MAX_ATTRIBUTES)                     throw std::runtime_error("Trying to define location (" + std::toString(location) + ") bigger then supported by this implementation. GL_MAX_ATTRIBUTES (" + std::toString(threadContext->value.GL_MAX_ATTRIBUTES) + ")");

      //if ((shaderType == GL_DOUBLE) && !threadContext->extension.GL_ARB_vertex_attrib_64bit) throw std::runtime_error("64 bit floating point is not supported by this system");
      //if (B10G11R11_UFLOAT) if (!threadContext->version.equalOrGreater(4,4)) throw std::runtime_error("B10G11R11_UFLOAT (GL_UNSIGNED_INT_10F_11F_11F_REV) is only supported with OpenGL 4.4 or higher");
    }*/
}
