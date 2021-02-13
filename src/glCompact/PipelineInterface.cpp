#include "glCompact/PipelineInterface.hpp"
#include "glCompact/config.hpp"
#include "glCompact/Debug.hpp"
#include "glCompact/Context_.hpp"
#include "glCompact/threadContext_.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/threadContextGroup_.hpp"
#include "glCompact/TextureInterface.hpp"
#include "glCompact/Sampler.hpp"
#include "glCompact/BufferGpu.hpp"
#include "glCompact/Tools_.hpp"
#include "glCompact/gl/Helper.hpp"
#include "glCompact/SurfaceFormatDetail.hpp"
#include "glCompact/multiMalloc.h"
#include "glCompact/minimumMaximum.hpp"

#include <glm/glm.hpp>

#include <regex>
#include <algorithm>

///\cond HIDDEN_FROM_DOXYGEN
/*
    Before ARB_shading_language_420pack (Core since OpenGL 4.2), the OpenGL binding slots for texture-sampler/images could only be set as a uniform after loading the shader.
    glCompact makes the binding part of the name in the shader in form of: NAME_bindingNUMBER
    This is also the only way in glCompact to set the binding! It is not possible to set this values as uniform!
    - It is an error if any of this uniforms do not folow this naming standard!
    - It is an error if more then one of each uniform types uses a specific binding number!

    From OpenGL docs:
        It is not allowed to have uniform variables of different sampler types
        pointing to the same texture image unit within a program object.

    Q: make shaderSeperate class? (ARB_separate_shader_objects, glProgramUniform)
    A: NO D3D anti patern!

    glGetIntegerv
        GL_MAX_UNIFORM_LOCATIONS
        GL_MAX_VERTEX_UNIFORM_COMPONENTS

    Uniform Buffer
        Needs GL_ARB_uniform_buffer_object (Core since 3.1)

        Minimum guaranteed size is 16 KiB. Some harware comes with 32 KiB and current hardware 64 KiB.

        GL_MAX_UNIFORM_BUFFER_BINDINGS (max. slots), we will limit it to max used slots anyway. Maybe print a single warning if anything not used by shader gets set?

        GL_UNIFORM_BLOCK_DATA_SIZE (Minimum size a buffer has to be when it gets bound as UBO, if the buffer is smaller it caused a OpenGL error or may be undetected and cause undefined behaviour or application termination)

        INTERN:
        void UniformBlockBinding( uint Shader, uint uniformBlockIndex, uint uniformBlockBinding );
        BindBufferRange

    Shader Storage Buffer
        Needs GL_ARB_shader_storage_buffer_object (Core since 4.3)

        MAX_SHADER_STORAGE_BUFFER_BINDINGS        (Min. 8)
        MAX_COMBINED_SHADER_OUTPUT_RESOURCES      (Min. 8)

        MAX_SHADER_STORAGE_BLOCK_SIZE             (Min. 2^24 = 16 MiB; Since Core 4.5: Min. 2^27 = 128 MiB)

        MAX_VERTEX_SHADER_STORAGE_BLOCKS          (Min. 0)
        MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS    (Min. 0)
        MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS (Min. 0)
        MAX_GEOMETRY_SHADER_STORAGE_BLOCKS        (Min. 0)
        MAX_FRAGMENT_SHADER_STORAGE_BLOCKS        (Min. 8)
        MAX_COMPUTE_SHADER_STORAGE_BLOCKS         (Min. 8)

        MAX_COMBINED_SHADER_STORAGE_BLOCKS        (Min. 8)

        SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT (Min. 256; is the maximum allowed, not the minimum. ?!)

        internal:
        void ShaderStorageBlockBinding( uint Shader, uint storageBlockIndex, uint storageBlockBinding );




    For usage binding of:
        GL_ATOMIC_COUNTER_BUFFER
        GL_TRANSFORM_FEEDBACK_BUFFER
        GL_UNIFORM_BUFFER
        GL_SHADER_STORAGE_BUFFER

        void glBindBufferBase  (GLenum target, GLuint index, GLuint buffer);
        void glBindBufferRange (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);

    ARB_multi_bind:
        void glBindBuffersBase (GLenum target, GLuint first, GLsizei count, const GLuint *buffers);
        void glBindBuffersRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
*/
///\endcond

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    PipelineInterface::~PipelineInterface() {
        //if (!SDL_GL_GetCurrentContext())
        //    cout << "WARNING: glCompact::PipelineInterface destructor called but no active OpenGL context in this thread to delete it! Leaking OpenGL object!" << endl;
        if (id) {
            if (threadContext_) {
                detachFromThreadContext();
            }
            if (threadContextGroup_) {
                threadContextGroup_->functions.glDeleteProgram(id);
            }
            id = 0;
        }
        free(multiMallocPtr);
    }

    void PipelineInterface::setTexture(
        uint32_t                slot,
        const TextureInterface& texture
    ) {
        UNLIKELY_IF (!texture.id)
            throw std::runtime_error("empty texture not accepted!");
        UNLIKELY_IF (int32_t(slot) >= sampler_count) return; //TODO: would break if anyone uses slot value over 0x8FFFFFFFF

        texture_id    [slot] = texture.id;
        texture_target[slot] = texture.target;
        texture_markSlotChange(slot);
    }

    void PipelineInterface::setTexture(
        uint32_t slot
    ) {
        UNLIKELY_IF (int32_t(slot) >= sampler_count) return;

        texture_id    [slot] = 0;
        texture_target[slot] = 0;
        texture_markSlotChange(slot);
    }

    void PipelineInterface::setTexture() {
        for (int32_t i = 0; i < sampler_count; ++i) {
            texture_id    [i] = 0;
            texture_target[i] = 0;
        }
        texture_changedSlotMin = 0;
        texture_changedSlotMax = sampler_count - 1;
    }

    void PipelineInterface::setSampler(
        uint32_t       slot,
        const Sampler& sampler
    ) {
        UNLIKELY_IF (int32_t(slot) >= sampler_count) return;

        sampler_id[slot] = sampler.id;
        sampler_markSlotChange(slot);
    }

    void PipelineInterface::setSampler(
        uint32_t slot
    ) {
        UNLIKELY_IF (int32_t(slot) >= sampler_count) return;

        sampler_id[slot] = 0;
        sampler_markSlotChange(slot);
    }

    void PipelineInterface::setSampler() {
        for (int32_t i = 0; i < sampler_count; ++i) sampler_id[i] = 0;
        sampler_changedSlotMin = 0;
        sampler_changedSlotMax = sampler_count - 1;
    }

    void PipelineInterface::setUniformBuffer(
        uint32_t               slot,
        const BufferInterface& buffer,
        uintptr_t              offset
    ) {
        UNLIKELY_IF (!buffer.id)
            throw std::runtime_error("does not take empty Buffer!");
        UNLIKELY_IF (int32_t(slot) >= buffer_uniform_count) return;

        buffer_uniform_id    [slot] = buffer.id;
        buffer_uniform_offset[slot] = offset;
        buffer_uniform_size  [slot] = 0;
        buffer_uniform_markSlotChange(slot);
    }

    void PipelineInterface::setUniformBuffer(
        uint32_t               slot,
        const BufferInterface& buffer,
        uintptr_t              offset,
        uintptr_t              size
    ) {
        UNLIKELY_IF (!buffer.id)
            throw std::runtime_error("does not take empty Buffer!");
        UNLIKELY_IF (int32_t(slot) >= buffer_uniform_count) return;

        buffer_uniform_id    [slot] = buffer.id;
        buffer_uniform_offset[slot] = offset;
        buffer_uniform_size  [slot] = size;
        buffer_uniform_markSlotChange(slot);
    }

    void PipelineInterface::setUniformBuffer(
        uint32_t slot
    ) {
        UNLIKELY_IF (int32_t(slot) >= buffer_uniform_count) return;

        buffer_uniform_id    [slot] = 0;
        buffer_uniform_offset[slot] = 0;
        buffer_uniform_size  [slot] = 0;
        buffer_uniform_markSlotChange(slot);
    }

    void PipelineInterface::setUniformBuffer() {
        for (int32_t i = 0; i < buffer_uniform_count; ++i) {
            buffer_uniform_id    [i] = 0;
            buffer_uniform_offset[i] = 0;
            buffer_uniform_size  [i] = 0;
        }
        buffer_uniform_changedSlotMin = 0;
        buffer_uniform_changedSlotMax = buffer_uniform_count - 1;
    }

    void PipelineInterface::setImage(
        uint32_t        slot,
        TextureSelector textureSelector
    ){
        setImage(slot, textureSelector, textureSelector.texture->surfaceFormat);
    }

    void PipelineInterface::setImage(
        uint32_t        slot,
        TextureSelector textureSelector,
        SurfaceFormat   surfaceFormat
    ){
        UNLIKELY_IF (int32_t(slot) >= image_count) return;

        image_id         [slot] = textureSelector.texture->id;
        image_format     [slot] = surfaceFormat->sizedFormat;
        image_mipmapLevel[slot] = textureSelector.mipmapLevel;
        image_layer      [slot] = textureSelector.layer;
        image_markSlotChange(slot);
    }

    void PipelineInterface::setImage(
        uint32_t slot
    ) {
        UNLIKELY_IF (int32_t(slot) >= image_count) return;

        image_id[slot] = 0;
        image_markSlotChange(slot);
    }

    void PipelineInterface::setImage() {
        for (int32_t i = 0; i < image_count; ++i) image_id[i] = 0;
        image_changedSlotMin = 0;
        image_changedSlotMax = image_count - 1;
    }

    void PipelineInterface::setAtomicCounterBuffer(
        uint32_t               slot,
        const BufferInterface& buffer,
        uintptr_t              offset
    ) {
        setAtomicCounterBuffer(slot, buffer, offset, buffer.getSize());
    }

    void PipelineInterface::setAtomicCounterBuffer(
        uint32_t               slot,
        const BufferInterface& buffer,
        uintptr_t              offset,
        uintptr_t              size
    ) {
        UNLIKELY_IF (!buffer.id)
            throw std::runtime_error("does not take empty Buffer!");
        UNLIKELY_IF (int32_t(slot) >= buffer_atomicCounter_count) return;
        UNLIKELY_IF (buffer.getSize() - offset < atomicCounterBindingList[slot].dataSize)
            throw std::runtime_error(string("")
            +   "Buffer does not have enough space after offset for all atomic counters!\n"
            +   "size(" + to_string(buffer.getSize()) + ") - offset(" + to_string(offset) + ") = " + to_string(buffer.getSize() - offset) + " but needed size for all actomic counters is "
            +   to_string(atomicCounterBindingList[slot].dataSize) + "!"
            );

        buffer_atomicCounter_id    [slot] = buffer.id;
        buffer_atomicCounter_offset[slot] = offset;
        buffer_atomicCounter_size  [slot] = size;
        buffer_atomicCounter_markSlotChange(slot);
    }

    void PipelineInterface::setAtomicCounterBuffer(
        uint32_t slot
    ) {
        UNLIKELY_IF (int32_t(slot) >= buffer_atomicCounter_count) return;

        buffer_atomicCounter_id    [slot] = 0;
        buffer_atomicCounter_offset[slot] = 0;
        buffer_atomicCounter_size  [slot] = 0;
        buffer_atomicCounter_markSlotChange(slot);
    }

    void PipelineInterface::setAtomicCounterBuffer() {
        for (int32_t i = 0; i < buffer_atomicCounter_count; ++i) {
            buffer_atomicCounter_id    [i] = 0;
            buffer_atomicCounter_offset[i] = 0;
            buffer_atomicCounter_size  [i] = 0;
        }
        buffer_atomicCounter_changedSlotMin = 0;
        buffer_atomicCounter_changedSlotMax = buffer_atomicCounter_count - 1;
    }

    /*
        offset must be aligned to GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT (max. 256)
    */
    void PipelineInterface::setShaderStorageBuffer(
        uint32_t         slot,
        BufferInterface& buffer,
        uintptr_t        offset
    ) {
        UNLIKELY_IF (!buffer.id)
            throw std::runtime_error("does not take empty Buffer!");
        UNLIKELY_IF (int32_t(slot) >= buffer_shaderStorage_count) return;

        setShaderStorageBuffer(slot, buffer, offset, buffer.size);
    }

    void PipelineInterface::setShaderStorageBuffer(
        uint32_t         slot,
        BufferInterface& buffer,
        uintptr_t        offset,
        uintptr_t        size
    ) {
        UNLIKELY_IF (!buffer.id)
            throw std::runtime_error("does not take empty Buffer!");
        UNLIKELY_IF (int32_t(slot) >= buffer_shaderStorage_count) return;

        buffer_shaderStorage_id    [slot] = buffer.id;
        buffer_shaderStorage_offset[slot] = offset;
        buffer_shaderStorage_size  [slot] = size;
        buffer_shaderStorage_markSlotChange(slot);
    }

    void PipelineInterface::setShaderStorageBuffer(
        uint32_t slot
    ) {
        UNLIKELY_IF (int32_t(slot) >= buffer_shaderStorage_count) return;

        buffer_shaderStorage_id    [slot] = 0;
        buffer_shaderStorage_offset[slot] = 0;
        buffer_shaderStorage_size  [slot] = 0;
        buffer_shaderStorage_markSlotChange(slot);
    }

    void PipelineInterface::setShaderStorageBuffer() {
        for (int i = 0; i < buffer_shaderStorage_count; ++i) {
            buffer_shaderStorage_id    [i] = 0;
            buffer_shaderStorage_offset[i] = 0;
            buffer_shaderStorage_size  [i] = 0;
        }
        buffer_shaderStorage_changedSlotMin = 0;
        buffer_shaderStorage_changedSlotMax = buffer_shaderStorage_count - 1;
    }

    void PipelineInterface::detachFromThreadContext() {
        if (threadContext_) {
            if (threadContext_->pipeline == this) threadContext_->pipeline = 0;
            checkedThatThreadContextBindingArraysAreBigEnough = false;
        }
    }

    std::string PipelineInterface::getShaderInfoLog(
        uint32_t objId
    ) {
        uint32_t infologLength = 0;
         int32_t maxLengthReturn;
        uint32_t maxLength;
        std::vector<char> bytes;

        threadContextGroup_->functions.glGetShaderiv(objId, GL_INFO_LOG_LENGTH, &maxLengthReturn);
        maxLength = maxLengthReturn;
        bytes.resize(maxLength + 1);
        threadContextGroup_->functions.glGetShaderInfoLog(objId, maxLength, &infologLength, &bytes[0]);

        if (infologLength > 0)
            return std::string(&bytes[0], infologLength);
        else
            return "";
    }

    std::string PipelineInterface::getProgramInfoLog(
        uint32_t objId
    ) {
        uint32_t infologLength = 0;
         int32_t maxLengthReturn;
        uint32_t maxLength;
        std::vector<char> bytes;

        threadContextGroup_->functions.glGetProgramiv(objId, GL_INFO_LOG_LENGTH, &maxLengthReturn);
        maxLength = maxLengthReturn;
        bytes.resize(maxLength + 1);
        threadContextGroup_->functions.glGetProgramInfoLog(objId, maxLength, &infologLength, &bytes[0]);

        if (infologLength > 0)
            return std::string(&bytes[0], infologLength);
        else
            return "";
    }

    /*bool PipelineInterface::copyBinaryToMemory(
        GLenum& binaryFormat,
        GLsizei bufSize,
        void*   mem
    ) const {
        if (!threadContext->extensions.GL_ARB_get_program_binary) return false;
        if (!binarySize || bufSize < binarySize) return false;
        GLsizei bufSizeReturn;
        threadContextGroup_->functions.glGetProgramBinary(id, binarySize, &bufSizeReturn, &binaryFormat, mem);
        return true;
    }

    bool PipelineInterface::copyBinaryFromMemory(
        GLenum      binaryFormat,
        GLsizei     bufSize,
        const void* mem
    ) {
        free();
        if (!threadContext->extensions.GL_ARB_get_program_binary) return false;
        id_ = threadContextGroup_->functions.glCreateProgram();
        threadContextGroup_->functions.glProgramBinary(id, binaryFormat, mem, bufSize);
        GLint linkStatus = 0;
        threadContextGroup_->functions.glGetProgramiv(id, GL_LINK_STATUS, &linkStatus);
        if (!linkStatus) {
            free();
            return false;
        }
        return true;
    }

    bool PipelineInterface::copyBinaryToFile(
        const std::string& fileName
    ) const {
        if (!binarySize) return false;
        GLenum binaryFormat;
        char* data = new char[binarySize];

        bool result = copyBinaryToMemory(binaryFormat, binarySize, data);
        if (result) {
            std::ofstream fileStream(fileName.c_str(), std::ios::out | std::ios::binary);
            if (fileStream.is_open()) {
                uint64_t binaryFormat64 = binaryFormat;
                fileStream.write(reinterpret_cast<const char*>(&binaryFormat64), sizeof(uint64_t));
                fileStream.write(reinterpret_cast<const char*>(data), binarySize);
            } else {
                result = false;
            }
        }

        delete[] data;
        return result;
    }

    bool PipelineInterface::copyBinaryFromFile(
        const std::string& fileName
    ) {
        free();
        bool result = false;
        std::ifstream fileStream(fileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
        if (fileStream.is_open()) {
            std::ifstream::pos_type fileSize = fileStream.tellg();
            fileStream.seekg(0, std::ios::beg);
            int binarySizeNew = int(fileSize) - sizeof(uint64_t);
            if (binarySizeNew <= 0) return false;
            GLenum binaryFormatNew;
            char* data = new char[binarySizeNew];
            uint64_t binaryFormat64;
            fileStream.read(reinterpret_cast<char*>(&binaryFormat64), sizeof(uint64_t));
            fileStream.read(reinterpret_cast<char*>(data), binarySizeNew);
            binaryFormatNew = static_cast<GLenum>(binaryFormat64);
            result = copyBinaryFromMemory(binaryFormatNew, binarySizeNew, data);
            delete[] data;
        }
        return result;
    }*/

    string PipelineInterface::getPipelineInformationQueryString() {
        string s;
        if (uniformList.size()) {
            s += " uniforms:\n";
            for (auto uniform : uniformList) {
                s += "  " + gl::typeToGlslAndCNameString(uniform.type) + " " + uniform.name
                + (uniform.arraySize ? ("[" + to_string(uniform.arraySize) + "]") : "")
                + "\n";
            }
        }
        if (samplerList.size()) {
            s += " samplers:\n";
            for (auto sampler : samplerList) {
                s += "  " + gl::typeToGlslAndCNameString(sampler.type) + " " + sampler.name + "\n";
            }
        }
        if (imageList.size()) {
            s += " images:\n";
            for (auto image : imageList) {
                s += "  " + gl::typeToGlslAndCNameString(image.type) + " " + image.name + "\n";
            }
        }
        if (uniformBlockList.size()) {
            s += " uniform blocks:\n";
            for (auto uniformBlock : uniformBlockList) {
                s += "  " + uniformBlock.name + " (size: " + to_string(uniformBlock.blockSize) + " byte)\n";
                for (auto uniform : uniformBlock.uniform) {
                    s += "   " + gl::typeToGlslAndCNameString(uniform.type) + " " + uniform.name
                    + (uniform.arraySize ? ("[" + to_string(uniform.arraySize) + "]") : "")
                    + "\n";
                }
            }
        }
        if (atomicCounterBindingList.size()) {
            s += " atomic counter buffer bindings:\n";
            LOOPI(atomicCounterBindingList.size()) {
                if (atomicCounterBindingList[i].uniformList.size()) {
                    s += "  binding " + to_string(i) + " (size: " + to_string(atomicCounterBindingList[i].dataSize) + " byte)\n";
                    for (auto ac : atomicCounterBindingList[i].uniformList) {
                        s += "   offset:" + to_string(ac.offset) + " " + ac.name + "\n";
                    }
                }
            }
        }
        return s;
    }

    void PipelineInterface::warning(
        const string& message
    ) {
        string s;
        if (threadContext_->pipelineThatCausedLastWarning != this) {
            threadContext_->pipelineThatCausedLastWarning = this;
            s += "Warning in " + getPipelineIdentificationString();
        }
        s += " " + message;
        Debug::warningFunc(s);
    }

    void PipelineInterface::error(
        const string& message
    ) {
        throw runtime_error("Error in " + getPipelineIdentificationString() + message);
    }

    //if the uniform name does not exist, this will return -1
    //Uniforms can be removed during optimization by the GLSL compiler.
    int32_t PipelineInterface::getUniformLocation(
        const std::string& uniformName
    ) {
        //return threadContextGroup_->functions.glGetUniformLocation(id, uniformName.c_str());
        for (auto& uniform : uniformList)
            if (uniform.name == uniformName) return uniform.location;
        return -1;
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const GLfloat& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform1f(shaderId , uniformLocation, value);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform1f(uniformLocation, value);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec2& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform2f(shaderId , uniformLocation, value[0], value[1]);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform2f(uniformLocation, value[0], value[1]);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec3& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform3f(shaderId , uniformLocation, value[0], value[1], value[2]);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform3f(uniformLocation, value[0], value[1], value[2]);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec4& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform4f(shaderId , uniformLocation, value[0], value[1], value[2], value[3]);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform4f(uniformLocation, value[0], value[1], value[2], value[3]);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const GLdouble& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform1d(shaderId , uniformLocation, value);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform1d(uniformLocation, value);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec2& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform2d(shaderId , uniformLocation, value[0], value[1]);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform2d(uniformLocation, value[0], value[1]);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec3& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform3d(shaderId , uniformLocation, value[0], value[1], value[2]);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform3d(uniformLocation, value[0], value[1], value[2]);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec4& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform4d(shaderId , uniformLocation, value[0], value[1], value[2], value[3]);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform4d(uniformLocation, value[0], value[1], value[2], value[3]);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const int32_t& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform1i(shaderId , uniformLocation, value);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform1i(uniformLocation, value);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec2& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform2i(shaderId , uniformLocation, value[0], value[1]);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform2i(uniformLocation, value[0], value[1]);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec3& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform3i(shaderId , uniformLocation, value[0], value[1], value[2]);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform3i(uniformLocation, value[0], value[1], value[2]);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec4& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform4i(shaderId , uniformLocation, value[0], value[1], value[2], value[3]);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform4i(uniformLocation, value[0], value[1], value[2], value[3]);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const GLuint& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform1ui(shaderId , uniformLocation, value);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform1ui(uniformLocation, value);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec2& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform2ui(shaderId , uniformLocation, value[0], value[1]);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform2ui(uniformLocation, value[0], value[1]);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec3& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform3ui(shaderId , uniformLocation, value[0], value[1], value[2]);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform3ui(uniformLocation, value[0], value[1], value[2]);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec4& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects) {
            threadContextGroup_->functions.glProgramUniform4ui(shaderId , uniformLocation, value[0], value[1], value[2], value[3]);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform4ui(uniformLocation, value[0], value[1], value[2], value[3]);
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const GLfloat& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform1fv(shaderId, uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform1fv(uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec2& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform2fv(shaderId, uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform2fv(uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec3& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform3fv(shaderId, uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform3fv(uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec4& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform4fv(shaderId, uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform4fv(uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    //NOTE: There are no EXT DSA functions for setting double uniforms
    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const GLdouble& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform1dv(shaderId, uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform1dv(uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec2& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform2dv(shaderId, uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform2dv(uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec3 &value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform3dv(shaderId, uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform3dv(uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec4& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform4dv(shaderId, uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform4dv(uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const int32_t& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform1iv(shaderId, uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform1iv(uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec2& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform2iv(shaderId, uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform2iv(uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec3& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform3iv(shaderId, uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform3iv(uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec4& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform4iv(shaderId, uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform4iv(uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const GLuint& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform1uiv(shaderId, uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform1uiv(uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec2& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform2uiv(shaderId, uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform2uiv(uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec3& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform3uiv(shaderId, uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform3uiv(uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec4& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniform4uiv(shaderId, uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniform4uiv(uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        }
    }

    //NOTE: transpose can be done with glm, therefor we ignore the transpose value for the gl function
    //TODO: row-major order. or column-major the default?
    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat2x2& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix2fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix2fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat2x3& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix2x3fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix2x3fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat2x4& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix2x4fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix2x4fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat3x2& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix3x2fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix3x2fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat3x3& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix3fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix3fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat3x4& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix3x4fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix3x4fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat4x2& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix4x2fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix4x2fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat4x3& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix4x3fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix4x3fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat4x4& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix4fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix4fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    //NOTE: There are no EXT DSA functions for setting double uniforms
    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat2x2& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix2dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix2dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat2x3& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix2x3dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix2x3dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat2x4& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix2x4dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix2x4dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat3x2& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix3x2dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix3x2dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat3x3& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix3dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix3dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat3x4& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix3x4dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix3x4dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat4x2& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix4x2dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix4x2dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat4x3& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix4x3dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix4x3dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat4x4& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup_->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup_->functions.glProgramUniformMatrix4dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformMatrix4dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    //GL_ARB_bindless_texture (not core)
    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const uint64_t& value) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS) {
            threadContextGroup_->functions.glProgramUniformHandleui64ARB(shaderId, uniformLocation, value);
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformHandleui64ARB(uniformLocation, value);
        }
    }

    //GL_ARB_bindless_texture (not core)
    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const uint64_t& value, int count) {
        if (config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS) {
            threadContextGroup_->functions.glProgramUniformHandleui64vARB(shaderId, uniformLocation, count, reinterpret_cast<const GLuint64*>(&value));
        } else {
            threadContext_->cachedBindShader(shaderId);
            threadContextGroup_->functions.glUniformHandleui64vARB(uniformLocation, count, reinterpret_cast<const GLuint64*>(&value));
        }
    }

    void PipelineInterface::buffer_uniform_markSlotChange(
        int32_t slot
    ) {
        if (slot >= buffer_uniform_count) return;
        buffer_uniform_changedSlotMin = minimum(buffer_uniform_changedSlotMin, slot);
        buffer_uniform_changedSlotMax = maximum(buffer_uniform_changedSlotMax, slot);
    }

    void PipelineInterface::buffer_atomicCounter_markSlotChange(
        int32_t slot
    ) {
        if (slot >= buffer_atomicCounter_count) return;
        buffer_atomicCounter_changedSlotMin = minimum(buffer_atomicCounter_changedSlotMin, slot);
        buffer_atomicCounter_changedSlotMax = maximum(buffer_atomicCounter_changedSlotMax, slot);
    }

    void PipelineInterface::buffer_shaderStorage_markSlotChange(
        int32_t slot
    ) {
        if (slot >= buffer_shaderStorage_count) return;
        buffer_shaderStorage_changedSlotMin = minimum(buffer_shaderStorage_changedSlotMin, slot);
        buffer_shaderStorage_changedSlotMax = maximum(buffer_shaderStorage_changedSlotMax, slot);
    }

    void PipelineInterface::texture_markSlotChange(
        int32_t slot
    ) {
        if (slot >= sampler_count) return;
        texture_changedSlotMin = minimum(texture_changedSlotMin, slot);
        texture_changedSlotMax = maximum(texture_changedSlotMax, slot);
    }

    void PipelineInterface::sampler_markSlotChange(
        int32_t slot
    ) {
        if (slot >= sampler_count) return;
        sampler_changedSlotMin = minimum(sampler_changedSlotMin, slot);
        sampler_changedSlotMax = maximum(sampler_changedSlotMax, slot);
    }

    void PipelineInterface::image_markSlotChange(
        int32_t slot
    ) {
        if (slot >= image_count) return;
        image_changedSlotMin = minimum(image_changedSlotMin, slot);
        image_changedSlotMax = maximum(image_changedSlotMax, slot);
    }

    /*
        Functions to read values of uniforms

        (Core since 2.0)
        void glGetUniformfv(    GLuint program, GLint location, GLfloat *params);
        void glGetUniformiv(    GLuint program, GLint location, GLint *params);

        (Core since 3.0)
        void glGetUniformuiv(   GLuint program, GLint location, GLuint *params);

        (Core since 4.0)
        void glGetUniformdv(    GLuint program, GLint location, GLdouble *params);

        (Core since 4.5)
        void glGetnUniformfv(   GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
        void glGetnUniformiv(   GLuint program, GLint location, GLsizei bufSize, GLint *params);
        void glGetnUniformuiv(  GLuint program, GLint location, GLsizei bufSize, GLuint *params);
        void glGetnUniformdv(   GLuint program, GLint location, GLsizei bufSize, GLdouble *params);

        GL_KHR_robustness (Core since 4.5)
        void glGetnUniformfv(uint program, int location, sizei bufSize, float *params);
        void glGetnUniformiv(uint program, int location, sizei bufSize, int *params);
        void glGetnUniformuiv(uint program, int location, sizei bufSize, uint *params);

        GL_ARB_robustness (Not Core)
        void GetUniformfv(     uint program, int location, float *params);
        void GetUniformiv(     uint program, int location, int *params);
        void GetUniformuiv(    uint program, int location, uint *params);
        void GetUniformdv(     uint program, int location, double *params);
        void GetnUniformfvARB( uint program, int location, sizei bufSize, float *params);
        void GetnUniformivARB( uint program, int location, sizei bufSize, int *params);
        void GetnUniformuivARB(uint program, int location, sizei bufSize, uint *params);
        void GetnUniformdvARB( uint program, int location, sizei bufSize, double *params);
    */

    void PipelineInterface::getUniform(uint32_t shaderId, int32_t uniformLocation, float& value) {
        threadContextGroup_->functions.glGetUniformfv(shaderId, uniformLocation, &value);
    }

    void PipelineInterface::getUniform(uint32_t shaderId, int32_t uniformLocation, int32_t& value) {
        threadContextGroup_->functions.glGetUniformiv(shaderId, uniformLocation, &value);
    }

    void PipelineInterface::getUniform(uint32_t shaderId, int32_t uniformLocation, uint32_t& value) {
        threadContextGroup_->functions.glGetUniformuiv(shaderId, uniformLocation, &value);
    }

    void PipelineInterface::getUniform(uint32_t shaderId, int32_t uniformLocation, double& value) {
        threadContextGroup_->functions.glGetUniformdv(shaderId, uniformLocation, &value);
    }

    void PipelineInterface::getUniform(uint32_t shaderId, int32_t uniformLocation, float& value, int count) {
        threadContextGroup_->functions.glGetnUniformfv(shaderId, uniformLocation, count, &value);
    }

    void PipelineInterface::getUniform(uint32_t shaderId, int32_t uniformLocation, int32_t& value, int count) {
        threadContextGroup_->functions.glGetnUniformiv(shaderId, uniformLocation, count, &value);
    }

    void PipelineInterface::getUniform(uint32_t shaderId, int32_t uniformLocation, uint32_t& value, int count) {
        threadContextGroup_->functions.glGetnUniformuiv(shaderId, uniformLocation, count, &value);
    }

    void PipelineInterface::getUniform(uint32_t shaderId, int32_t uniformLocation, double& value, int count) {
        threadContextGroup_->functions.glGetnUniformdv(shaderId, uniformLocation, count, &value);
    }

    /*
        Shader inquiry functions

        Core since like forever (2.0; or even lower?):
            void     glGetProgramiv                   (uint32_t program, GLenum pname, int32_t *params);
                GL_ACTIVE_UNIFORMS
                GL_ACTIVE_UNIFORM_MAX_LENGTH
            void     glGetActiveUniform               (uint32_t program, uint32_t index, uint32_t bufSize, uint32_t *length, int32_t *size, GLenum *type, char *name);
            int32_t  glGetUniformLocation             (uint32_t program, const char *name);

            void     glGetActiveAttrib                (uint32_t program, uint32_t index, uint32_t bufSize, uint32_t *length, int32_t *size, enum *type, char *name);

        Core since 3.0
             int32_t glGetFragDataLocation(uint32_t program, const char * name);
            void     glBindFragDataLocation(uint32_t program, uint32_t colorNumber, const char * name);

        GL_ARB_uniform_buffer_object (Core since 3.1)
            void     glGetUniformIndices              (uint32_t program, uint32_t uniformCount, const char* const * uniformNames, uint32_t* uniformIndices);
            void     glGetActiveUniformsiv            (uint32_t program, uint32_t uniformCount, const uint32_t* uniformIndices, GLenum pname, int32_t* params);
            void     glGetActiveUniformName           (uint32_t program, uint32_t uniformIndex, uint32_t bufSize, uint32_t* length, char* uniformName);
            uint32_t glGetUniformBlockIndex           (uint32_t program, const char* uniformBlockName);
            void     glGetActiveUniformBlockiv        (uint32_t program, uint32_t uniformBlockIndex, GLenum pname, int32_t* params);
            void     glGetActiveUniformBlockName      (uint32_t program, uint32_t uniformBlockIndex, uint32_t bufSize, uint32_t* length, char* uniformBlockName);
            void     glBindBufferRange                (GLenum target, uint32_t index, uint32_t buffer, intptr offset, sizeiptr size);
            void     glBindBufferBase                 (GLenum target, uint32_t index, uint32_t buffer);
            void     glGetIntegeri_v                  (GLenum target, uint32_t index, int32_t* data);
            void     glUniformBlockBinding            (uint32_t program, uint32_t uniformBlockIndex, uint32_t uniformBlockBinding);

            glGetProgramiv:
                GL_ACTIVE_UNIFORM_BLOCKS
                GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH

        Core since 3.2
            int32_t glGetFragDataIndex(uint32_t program, const char * name);

        GL_ARB_shader_subroutine (Core since 4.0)
             int32_t glGetSubroutineUniformLocation  (uint32_t program, enum shadertype,  const char *name);
            uint32_t glGetSubroutineIndex            (uint32_t program, GLenum shadertype, const char *name);
            void     glGetActiveSubroutineUniformiv  (uint32_t program, GLenum shadertype, uint32_t index, GLenum pname, int32_t *values);
            void     glGetActiveSubroutineUniformName(uint32_t program, GLenum shadertype, uint32_t index, uint32_t bufsize, uint32_t *length, char *name);
            void     glGetActiveSubroutineName       (uint32_t program, GLenum shadertype, uint32_t index, uint32_t bufsize, uint32_t *length, char *name);
            void     glUniformSubroutinesuiv         (enum shadertype, uint32_t count, const uint32_t *indices);
            void     glGetUniformSubroutineuiv       (enum shadertype, int32_t location, uint32_t *params);
            void     glGetProgramStageiv             (uint32_t program, GLenum shadertype, GLenum pname, int32_t *values);

        GL_ARB_shader_atomic_counters (Core since 4.2)
            void glGetActiveAtomicCounterBufferiv(uint32_t program, uint32_t bufferIndex, enum pname, int32_t *params );

        GL_ARB_program_interface_query (Core since 4.3)
            void     glGetProgramInterfaceiv          (uint32_t program, GLenum programInterface, GLenum pname, int32_t *params);
            uint32_t glGetProgramResourceIndex        (uint32_t program, GLenum programInterface, const char *name);
            void     glGetProgramResourceName         (uint32_t program, GLenum programInterface, uint32_t index, uint32_t bufSize, uint32_t *length, char *name);
            void     glGetProgramResourceiv           (uint32_t program, GLenum programInterface, uint32_t index, uint32_t propCount, const GLenum *props, uint32_t bufSize, uint32_t *length, int32_t *params);
            int32_t  glGetProgramResourceLocation     (uint32_t program, GLenum programInterface, const char *name);
            int32_t  glGetProgramResourceLocationIndex(uint32_t program, GLenum programInterface, const char *name);

        programInterface:
            GL_UNIFORM
            GL_UNIFORM_BLOCK
            GL_ATOMIC_COUNTER_BUFFER
            GL_PROGRAM_INPUT
            GL_PROGRAM_OUTPUT

            GL_VERTEX_SUBROUTINE
            GL_TESS_CONTROL_SUBROUTINE
            GL_TESS_EVALUATION_SUBROUTINE
            GL_GEOMETRY_SUBROUTINE
            GL_FRAGMENT_SUBROUTINE
            GL_COMPUTE_SUBROUTINE

            GL_VERTEX_SUBROUTINE_UNIFORM
            GL_TESS_CONTROL_SUBROUTINE_UNIFORM
            GL_TESS_EVALUATION_SUBROUTINE_UNIFORM
            GL_GEOMETRY_SUBROUTINE_UNIFORM
            GL_FRAGMENT_SUBROUTINE_UNIFORM
            GL_COMPUTE_SUBROUTINE_UNIFORM

            GL_TRANSFORM_FEEDBACK_VARYING
            GL_BUFFER_VARIABLE
            GL_SHADER_STORAGE_BLOCK
            GL_TRANSFORM_FEEDBACK_BUFFER
        pname:
            GL_ACTIVE_RESOURCES
            GL_MAX_NAME_LENGTH
            GL_MAX_NUM_ACTIVE_VARIABLES

    */

    int getBindingFromString(
        const std::string& s
    ) {
        //using raw string literal to not polute regex escape characters
        //R"(  )";
        //R"##(  )##";

        //TODO: move into static object, maybe even use one of this fancy static regex libs...
      //static regex  expression(R"##(^binding([[:digit:]]+)_+)##"); //binding0_texturename
        static regex  expression(R"##(_binding([[:digit:]]+)$)##");  //texturename_binding0
        smatch match;
        if (regex_search(s, match, expression)) {
            return stoi(match.str(1));
        } else {
            //throw std::runtime_error("glCompact needs sampler/image names to start with binding<BINDINGNUMBER>_. But name is \"" + s + "\"");
            throw std::runtime_error("glCompact needs sampler/image/uniformBlock names in shaders to end with _binding<BINDINGNUMBER>. But name is \"" + s + "\"");
        }
    }

    /*
        This function queries several shader information like uniforms (including sampler/image/atomic_counter binding setters), UBO, SSBO
        It also contains glCompacts own bind-by-name-parameter (NAME_bindingX) and associated checks and warnings.

        NOTE: size of max name length, for all sub objects, always includes the 0-terminator. Name getter functions return sizes do NOT contain the 0-terminator.
    */
    void PipelineInterface::collectInformation() {
        vector<char> nameBuffer;

        //get infos about all attributes (Core since 2.0)
        int32_t activeAttributeCount;
        int32_t activeAttributeNameLengthMax;
        threadContextGroup_->functions.glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES,           &activeAttributeCount);
        threadContextGroup_->functions.glGetProgramiv(id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &activeAttributeNameLengthMax);
        nameBuffer.resize(activeAttributeNameLengthMax);
        LOOPI(activeAttributeCount) {
            Attribute attribute;
            uint32_t stringLenght = 0;
            threadContextGroup_->functions.glGetActiveAttrib(id, i, activeAttributeNameLengthMax, &stringLenght, &attribute.arraySize, &attribute.type, &nameBuffer[0]);
            attribute.name = string(&nameBuffer[0], stringLenght);
        }

        //get infos about all uniforms. This includes uniforms from UBOs and SSBOs. Whose uniform location are -1.
        struct RawUniform {
            string  name;
            int32_t location;
            int32_t type;
            int32_t arraySize;
            int32_t arrayStride;
        };
        vector<RawUniform> rawUniformList;

        int32_t activeUniformCount         = 0;
        int32_t activeUniformNameLengthMax = 0;
        threadContextGroup_->functions.glGetProgramiv(id, GL_ACTIVE_UNIFORMS,           &activeUniformCount);
        threadContextGroup_->functions.glGetProgramiv(id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &activeUniformNameLengthMax);
        rawUniformList.resize(activeUniformCount);
        nameBuffer.resize(activeUniformNameLengthMax);
        LOOPI(activeUniformCount) {
            auto& ru = rawUniformList[i];
            uint32_t stringLenght = 0;
            threadContextGroup_->functions.glGetActiveUniform(id, uint32_t(i), activeUniformNameLengthMax, &stringLenght, &ru.arraySize, &ru.type, &nameBuffer[0]);
            string name(&nameBuffer[0], stringLenght);
            //Remove any trailing "[0]"
            if ((name.size() >= 3) && (name.substr(name.size() - 3, 3) == "[0]")) name = name.substr(0, name.size() - 3);
            int32_t location0 = threadContextGroup_->functions.glGetUniformLocation(id, &nameBuffer[0]);
            int32_t location1 = threadContextGroup_->functions.glGetUniformLocation(id, (name + "[1]").c_str());
            ru.name        = name;
            ru.location    = location0;
            ru.arrayStride = (location0 != -1 && location1 != -1) ? (location1 - location0) : 0;

            if (gl::typeIsSampler(ru.type)) {
                int32_t layoutQualifierBinding;
                getUniform(id, ru.location, layoutQualifierBinding);
                int32_t binding = getBindingFromString(ru.name);
                //cout << "DEBUG OUTPUT: " << typeToGlslAndCNameString(ru.type) << " " << ru.name << " layoutQualifierBinding:" << ru.layoutQualifierBinding << endl;
                if (layoutQualifierBinding && layoutQualifierBinding != binding)
                    error(string("")
                        + "Sampler uses different GLSL layout qualifier binding value then glCompact NAME_bindingX binding! But must be the same or layout qualifier must be removed!\n"
                        + " -> layout(binding = " + to_string(layoutQualifierBinding) + ") " + ru.name
                    );
                //if (binding >= config::MAX_SAMPLER_BINDINGS)
                //    error(" Trying to use sampler binding that is outside the range of config::MAX_SAMPLER_BINDINGS(" + to_string(config::MAX_SAMPLER_BINDINGS) + " = 0.." + to_string(config::MAX_SAMPLER_BINDINGS-1) + ")\n"
                //    + " -> " + ru.name);
                if (samplerList.size() < uint32_t(binding + 1))
                    samplerList.resize(binding + 1);
                if (samplerList[binding].type != 0)
                    error(string("")
                        + " Trying to use sampler binding more then once!\n"
                        + " -> " + samplerList[binding].name + "\n" +
                        + " -> " + ru.name
                    );
                setUniform(id, ru.location, binding);
                samplerList[binding].name = ru.name;
                samplerList[binding].type = ru.type;


            } else if (gl::typeIsImage(ru.type)) {
                int32_t layoutQualifierBinding;
                getUniform(id, ru.location, layoutQualifierBinding);
                int32_t binding = getBindingFromString(ru.name);
                if (layoutQualifierBinding && layoutQualifierBinding != binding)
                    error(string("")
                        + "Image uses different GLSL layout qualifier binding value then glCompact NAME_bindingX binding! But must be the same or layout qualifier must be removed!\n"
                        + " -> layout(binding = " + to_string(layoutQualifierBinding) + ") " + ru.name
                    );
                //if (binding >= config::MAX_IMAGE_BINDINGS)
                //    error(" Trying to use sampler binding that is outside the range of config::MAX_IMAGE_BINDINGS(" + to_string(config::MAX_IMAGE_BINDINGS) + " = 0.." + to_string(config::MAX_IMAGE_BINDINGS-1) + ")\n"
                //    + " -> " + ru.name);
                if (imageList.size() < uint32_t(binding + 1))
                    imageList.resize(binding + 1);
                if (imageList[binding].type != 0)
                    error(string("")
                        + " Trying to use image binding more then once!\n"
                        + " -> " + imageList[binding].name + "\n" +
                        + " -> " + ru.name
                    );
                setUniform(id, ru.location, binding);
                imageList[binding].name = ru.name;
                imageList[binding].type = ru.type;

            //} else if (ru.type == GL_UNSIGNED_INT_ATOMIC_COUNTER) {
            } else if (ru.location != -1) {
                Uniform u;
                u.name        = ru.name;
                u.location    = ru.location;
                u.type        = ru.type;
                u.arraySize   = ru.arraySize;
                u.arrayStride = ru.arrayStride;
                uniformList.push_back(u);
            }
        }

        //Uniform Buffer Object (UBO) (Core since 3.1)
        //Note: explicitly allowes different uniform blocks to use the same binding!
        if (threadContextGroup_->extensions.GL_ARB_uniform_buffer_object) {
            int32_t activeUniformBlockCount         = 0;
            int32_t activeUniformBlockNameLengthMax = 0;
            threadContextGroup_->functions.glGetProgramiv(id, GL_ACTIVE_UNIFORM_BLOCKS,                &activeUniformBlockCount);
            threadContextGroup_->functions.glGetProgramiv(id, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &activeUniformBlockNameLengthMax);
            nameBuffer.resize(activeUniformBlockNameLengthMax);
            uniformBlockList.resize(activeUniformBlockCount);

            LOOPI(activeUniformBlockCount) {
                auto& uniformBlock = uniformBlockList[i];
                uint32_t stringLenght = 0;
                int32_t uniformCount;
                threadContextGroup_->functions.glGetActiveUniformBlockName(id, uint32_t(i), activeUniformBlockNameLengthMax, &stringLenght, &nameBuffer[0]);
                uniformBlock.name = string(&nameBuffer[0], stringLenght);
                int32_t binding = getBindingFromString(uniformBlock.name);
                //if (binding >= config::MAX_UNIFORM_BUFFER_BINDINGS)
                //    error(string("")
                //        + "Trying to use uniform block binding outisde of valid range of config::MAX_UNIFORM_BUFFER_BINDINGS(" + to_string(config::MAX_UNIFORM_BUFFER_BINDINGS) + " = 0.." + to_string(config::MAX_UNIFORM_BUFFER_BINDINGS - 1) + ")\n"
                //        + " -> " + uniformBlock.name
                //    );
                int32_t blockIndex = threadContextGroup_->functions.glGetUniformBlockIndex(id, &nameBuffer[0]);
                int32_t layoutQualifierBinding;
                threadContextGroup_->functions.glGetActiveUniformBlockiv(id, blockIndex, GL_UNIFORM_BLOCK_BINDING, &layoutQualifierBinding);
                if (layoutQualifierBinding && layoutQualifierBinding != binding)
                    error(string("")
                        + "Uniform block uses different GLSL layout qualifier binding values then glCompact NAME_bindingX binding! But must be the same!"
                        + " -> layout(binding = " + to_string(layoutQualifierBinding) + ") " + uniformBlock.name
                    );
                threadContextGroup_->functions.glUniformBlockBinding(id, blockIndex, binding);
                threadContextGroup_->functions.glGetActiveUniformBlockiv(id, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &uniformBlock.blockSize);
                threadContextGroup_->functions.glGetActiveUniformBlockiv(id, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &uniformCount);
                vector<uint32_t> uniformIndexList(uniformCount); //index into raw uniform list
                threadContextGroup_->functions.glGetActiveUniformBlockiv(id, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, reinterpret_cast< int32_t*>(&uniformIndexList[0]));

                vector< int32_t> uniformOffsetList          (uniformCount);
                vector< int32_t> uniformArrayStrideList     (uniformCount); //?
                vector< int32_t> uniformMatrixStrideList    (uniformCount); //?
                vector< int32_t> uniformMatricIsRowMajorList(uniformCount); // 0 = column-major matrix, 1 = row-major matrix
                threadContextGroup_->functions.glGetActiveUniformsiv(id, uniformCount, &uniformIndexList[0], GL_UNIFORM_OFFSET,          &uniformOffsetList          [0]);
                threadContextGroup_->functions.glGetActiveUniformsiv(id, uniformCount, &uniformIndexList[0], GL_UNIFORM_ARRAY_STRIDE,    &uniformArrayStrideList     [0]);
                threadContextGroup_->functions.glGetActiveUniformsiv(id, uniformCount, &uniformIndexList[0], GL_UNIFORM_MATRIX_STRIDE,   &uniformMatrixStrideList    [0]);
                threadContextGroup_->functions.glGetActiveUniformsiv(id, uniformCount, &uniformIndexList[0], GL_UNIFORM_IS_ROW_MAJOR,    &uniformMatricIsRowMajorList[0]);

                uniformBlock.uniform.resize(uniformCount);
                LOOPJ(uniformCount) {
                    auto& uniform = uniformBlock.uniform[j];
                    uniform.name           = rawUniformList[uniformIndexList[j]].name;
                    uniform.type           = rawUniformList[uniformIndexList[j]].type;
                    uniform.arraySize      = rawUniformList[uniformIndexList[j]].arraySize;
                    uniform.offset         = uniformOffsetList              [j];
                    uniform.arrayStride    = uniformArrayStrideList         [j];
                    uniform.matrixStride   = uniformMatrixStrideList        [j];
                    uniform.matrixRowMajor = uniformMatricIsRowMajorList    [j] == 1;
                }
            }
        }

        //Atomic Counter Buffer Bindings - GL_ARB_shader_atomic_counters (Core since 4.2)
        if (threadContextGroup_->extensions.GL_ARB_shader_atomic_counters) {
            //glGetActiveUniformsiv UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX
            //glGetActiveAtomicCounterBufferiv( GLuint program, GLuint bufferIndex, GLenum pname, GLint *params);
            int32_t activeAtomicCounter = 0;
            threadContextGroup_->functions.glGetProgramiv(id, GL_ACTIVE_ATOMIC_COUNTER_BUFFERS, &activeAtomicCounter);
            atomicCounterBindingList.resize(activeAtomicCounter);
            LOOPI(activeAtomicCounter) {
                int32_t binding;
                threadContextGroup_->functions.glGetActiveAtomicCounterBufferiv(id, i, GL_ATOMIC_COUNTER_BUFFER_BINDING, &binding);
                if (atomicCounterBindingList.size() < uint32_t(binding) + 1) atomicCounterBindingList.resize(binding + 1);
                auto& acbb = atomicCounterBindingList[binding];

                threadContextGroup_->functions.glGetActiveAtomicCounterBufferiv(id, i, GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE, reinterpret_cast<int32_t*>(&acbb.dataSize));
                int32_t atomicCountersCount = 0;
                threadContextGroup_->functions.glGetActiveAtomicCounterBufferiv(id, i, GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS, &atomicCountersCount);
                vector<int32_t> atomicCounterIndexList(atomicCountersCount);
                vector<int32_t> uniformOffsetList     (atomicCountersCount);
                threadContextGroup_->functions.glGetActiveAtomicCounterBufferiv(id, i, GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES, &atomicCounterIndexList[0]);
                threadContextGroup_->functions.glGetActiveUniformsiv(id, atomicCountersCount, reinterpret_cast<uint32_t*>(&atomicCounterIndexList[0]), GL_UNIFORM_OFFSET, &uniformOffsetList[0]);
                acbb.uniformList.resize(atomicCountersCount);
                LOOPJ(atomicCountersCount) {
                    auto& ac = acbb.uniformList[j];
                    ac.name   = rawUniformList[atomicCounterIndexList[j]].name;
                    ac.offset = uniformOffsetList                    [j];
                }
            }
        }

        //Shader Storage Buffer Object (SSBO) (Core since 4.3)
        //
        //SSBOs presupposes GL_ARB_program_interface_query being supported! Because it is the only way to query information about the shader bindings!
        //
        //NOTE: The index parameter of glGetProgramResourceiv:
        //The base range of 0.. is used for getting information about each kind of interface array (Uniforms, UniformBlocks, ShaderStorageBlocks, ...)
        //Among other things this is used to query arrays of sub element indices. Who then can be used again as an index parameter for glGetProgramResourceiv...
        struct RawStorageBlockVariable {
            string  name;
            int32_t type;
            int32_t arraySize;
            int32_t offset;
            int32_t arrayStride;
            int32_t matrixStride;
            int32_t matrixRowMajor;

            int32_t topLevelArraySize;
            int32_t topLevelArrayStride;
        };
        struct RawStorageBlock {
            string name;
            int32_t index;
            std::vector<RawStorageBlockVariable> variable;
        };
        vector<RawStorageBlock> rawStorageBlockList;
        if (threadContextGroup_->extensions.GL_ARB_shader_storage_buffer_object) {
            int32_t activeSSBCount;
            int32_t activeSSBMaxNameLength;
            threadContextGroup_->functions.glGetProgramInterfaceiv(id, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &activeSSBCount);
            threadContextGroup_->functions.glGetProgramInterfaceiv(id, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH,  &activeSSBMaxNameLength);
            nameBuffer.resize(activeSSBMaxNameLength);
            //rawStorageBlockList.resize(activeSSBCount);
            //if (activeSSBCount) cout << " RAW SSB (" << to_string(activeSSBCount) << "):" << endl;

            LOOPI(activeSSBCount) {
                StorageBlock sb;
                uint32_t nameLength = 0;
                threadContextGroup_->functions.glGetProgramResourceName(id, GL_SHADER_STORAGE_BLOCK, i, activeSSBMaxNameLength, &nameLength, &nameBuffer[0]);
                string name(&nameBuffer[0], nameLength);
                sb.name = name;

                struct {
                    int32_t activeVariableCount = 0;
                    int32_t binding;
                } query;
                //int32_t activeVariableCount = 0;
                int32_t activeVariableCountQuery[] = {
                    GL_NUM_ACTIVE_VARIABLES,
                    GL_BUFFER_BINDING
                };
                int32_t queryCount = sizeof(activeVariableCountQuery) / 4;
                threadContextGroup_->functions.glGetProgramResourceiv(id, GL_SHADER_STORAGE_BLOCK, i, queryCount, activeVariableCountQuery, queryCount, nullptr, reinterpret_cast<int32_t*>(&query));
                sb.binding = query.binding;
                sb.variable.resize(query.activeVariableCount);
                vector<int32_t> activeVariableIndexList(query.activeVariableCount);
                int32_t activeVariableIndexListQuery[] = {GL_ACTIVE_VARIABLES};
                threadContextGroup_->functions.glGetProgramResourceiv(id, GL_SHADER_STORAGE_BLOCK, i, 1, activeVariableIndexListQuery, query.activeVariableCount, nullptr, &activeVariableIndexList[0]);
                LOOPJ(query.activeVariableCount) {
                    auto& variable = sb.variable[j];

                    int32_t bufferVariableNameLengthWithNullTerminator = 0;
                    int32_t nameLengthQuery[] = {GL_NAME_LENGTH};
                    threadContextGroup_->functions.glGetProgramResourceiv(id, GL_BUFFER_VARIABLE, activeVariableIndexList[j], 1, nameLengthQuery, 1, nullptr, &bufferVariableNameLengthWithNullTerminator);
                    if (nameBuffer.size() < uint32_t(bufferVariableNameLengthWithNullTerminator)) nameBuffer.resize(bufferVariableNameLengthWithNullTerminator);
                    threadContextGroup_->functions.glGetProgramResourceName(id, GL_BUFFER_VARIABLE, activeVariableIndexList[j], bufferVariableNameLengthWithNullTerminator, nullptr, &nameBuffer[0]);
                    variable.name = string(&nameBuffer[0], bufferVariableNameLengthWithNullTerminator - 1);

                    struct {
                        int32_t type;
                        int32_t arraySize;
                        int32_t offset;
                        int32_t arrayStride;
                        int32_t matrixStride;
                        int32_t matrixRowMajor;
                        int32_t topLevelArraySize;
                        int32_t topLevelArrayStride;
                    } queryData;
                    int32_t bufferVariableDataQuery[] = {GL_TYPE, GL_ARRAY_SIZE, GL_OFFSET, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR, GL_TOP_LEVEL_ARRAY_SIZE, GL_TOP_LEVEL_ARRAY_STRIDE};
                    int32_t bufferVariableDataQueryCount = sizeof(bufferVariableDataQuery) / 4;
                    threadContextGroup_->functions.glGetProgramResourceiv(id, GL_BUFFER_VARIABLE, activeVariableIndexList[j], bufferVariableDataQueryCount, bufferVariableDataQuery, bufferVariableDataQueryCount, nullptr, reinterpret_cast<int32_t*>(&queryData));

                    variable.type                = queryData.type;
                    variable.arraySize           = queryData.arraySize;
                    variable.offset              = queryData.offset;
                    variable.arrayStride         = queryData.arrayStride;
                    variable.matrixStride        = queryData.matrixStride;
                    variable.matrixRowMajor      = queryData.matrixRowMajor == 1;
                    variable.topLevelArraySize   = queryData.topLevelArraySize;
                    variable.topLevelArrayStride = queryData.topLevelArrayStride;
                }
                storageBlockList.push_back(sb);
            }
        }
        sampler_count               = samplerList.size();
        for (const auto& ub : uniformBlockList) buffer_uniform_count = max<size_t>(buffer_uniform_count, ub.binding + 1);
        image_count                 = imageList.size();
        buffer_atomicCounter_count  = atomicCounterBindingList.size();
        for (const auto& sb : storageBlockList) buffer_shaderStorage_count = max<size_t>(buffer_shaderStorage_count, sb.binding + 1);
    }

    void PipelineInterface::allocateMemory() {
        multiMallocDescriptor md[] = {
            {&buffer_uniform_id,            &buffer_uniform_count,          buffer_uniform_count},
            {&buffer_uniform_offset,        &buffer_uniform_count,          buffer_uniform_count},
            {&buffer_uniform_size,          &buffer_uniform_count,          buffer_uniform_count},
            {&buffer_atomicCounter_id,      &buffer_atomicCounter_count,    buffer_atomicCounter_count},
            {&buffer_atomicCounter_offset,  &buffer_atomicCounter_count,    buffer_atomicCounter_count},
            {&buffer_atomicCounter_size,    &buffer_atomicCounter_count,    buffer_atomicCounter_count},
            {&buffer_shaderStorage_id,      &buffer_shaderStorage_count,    buffer_shaderStorage_count},
            {&buffer_shaderStorage_offset,  &buffer_shaderStorage_count,    buffer_shaderStorage_count},
            {&buffer_shaderStorage_size,    &buffer_shaderStorage_count,    buffer_shaderStorage_count},
            {&texture_id,                   &sampler_count,                 sampler_count},
            {&texture_target,               &sampler_count,                 sampler_count},
            {&sampler_id,                   &sampler_count,                 sampler_count},
            {&image_id,                     &image_count,                   image_count},
            {&image_format,                 &image_count,                   image_count},
            {&image_mipmapLevel,            &image_count,                   image_count},
            {&image_layer,                  &image_count,                   image_count}
        };
        multiMallocPtr = multiMalloc(md, sizeof(md));
    }

    void PipelineInterface::processPendingChanges() {
        processPendingChangesBuffersUniform();
        processPendingChangesBuffersAtomicCounter();
        processPendingChangesBuffersShaderStorage();
        processPendingChangesTextures();
        processPendingChangesSamplers();
        processPendingChangesImages();
        threadContext_->processPendingChangesMemoryBarriers();
    }


    void PipelineInterface::processPendingChangesPipeline() {
        if (!checkedThatThreadContextBindingArraysAreBigEnough) {
            multiMallocDescriptor md[] = {
                {&threadContext_->buffer_uniform_id,            &threadContext_->buffer_uniform_count,          buffer_uniform_count},
                {&threadContext_->buffer_uniform_offset,        &threadContext_->buffer_uniform_count,          buffer_uniform_count},
                {&threadContext_->buffer_uniform_size,          &threadContext_->buffer_uniform_count,          buffer_uniform_count},
                {&threadContext_->buffer_atomicCounter_id,      &threadContext_->buffer_atomicCounter_count,    buffer_atomicCounter_count},
                {&threadContext_->buffer_atomicCounter_offset,  &threadContext_->buffer_atomicCounter_count,    buffer_atomicCounter_count},
                {&threadContext_->buffer_atomicCounter_size,    &threadContext_->buffer_atomicCounter_count,    buffer_atomicCounter_count},
                {&threadContext_->buffer_shaderStorage_id,      &threadContext_->buffer_shaderStorage_count,    buffer_shaderStorage_count},
                {&threadContext_->buffer_shaderStorage_offset,  &threadContext_->buffer_shaderStorage_count,    buffer_shaderStorage_count},
                {&threadContext_->buffer_shaderStorage_size,    &threadContext_->buffer_shaderStorage_count,    buffer_shaderStorage_count},
                {&threadContext_->texture_id,                   &threadContext_->sampler_count,                 sampler_count},
                {&threadContext_->texture_target,               &threadContext_->sampler_count,                 sampler_count},
                {&threadContext_->sampler_id,                   &threadContext_->sampler_count,                 sampler_count},
                {&threadContext_->image_id,                     &threadContext_->image_count,                   image_count},
                {&threadContext_->image_format,                 &threadContext_->image_count,                   image_count},
                {&threadContext_->image_mipmapLevel,            &threadContext_->image_count,                   image_count},
                {&threadContext_->image_layer,                  &threadContext_->image_count,                   image_count},
            };
            threadContext_->multiMallocPtr = multiReMallocGrowOnly(threadContext_->multiMallocPtr, md, sizeof(md));
            checkedThatThreadContextBindingArraysAreBigEnough = true;
        }
    }

    void PipelineInterface::processPendingChangesBuffersUniform() {
        auto changedSlotMin = buffer_uniform_changedSlotMin;
        auto changedSlotMax = buffer_uniform_changedSlotMax;

        if (changedSlotMin <= changedSlotMax) {
            if (threadContextGroup_->extensions.GL_ARB_multi_bind) {
                //Filter out unchanged slots at the beginning and end of the list
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext_->buffer_uniform_id    [changedSlotMin] != buffer_uniform_id    [changedSlotMin]
                    ||  threadContext_->buffer_uniform_offset[changedSlotMin] != buffer_uniform_offset[changedSlotMin]
                    ||  threadContext_->buffer_uniform_size  [changedSlotMin] != buffer_uniform_size  [changedSlotMin]) break;
                    changedSlotMin++;
                }
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext_->buffer_uniform_id    [changedSlotMax] != buffer_uniform_id    [changedSlotMax]
                    ||  threadContext_->buffer_uniform_offset[changedSlotMax] != buffer_uniform_offset[changedSlotMax]
                    ||  threadContext_->buffer_uniform_size  [changedSlotMax] != buffer_uniform_size  [changedSlotMax]) break;
                    changedSlotMax--;
                }

                if (changedSlotMin <= changedSlotMax) {
                    const uint32_t    count        = changedSlotMax - changedSlotMin + 1;
                    const uint32_t*   bufferIdList =                                     &buffer_uniform_id    [changedSlotMin];
                    const GLintptr*   offsetList   = reinterpret_cast<const GLintptr*>  (&buffer_uniform_offset[changedSlotMin]);
                    const GLsizeiptr* sizeList     = reinterpret_cast<const GLsizeiptr*>(&buffer_uniform_size  [changedSlotMin]);
                    threadContextGroup_->functions.glBindBuffersRange(GL_UNIFORM_BUFFER, changedSlotMin, count, bufferIdList, offsetList, sizeList);
                }
            } else {
                for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                    if (threadContext_->buffer_uniform_id    [i] != buffer_uniform_id    [i]
                    ||  threadContext_->buffer_uniform_offset[i] != buffer_uniform_offset[i]
                    ||  threadContext_->buffer_uniform_size  [i] != buffer_uniform_size  [i])
                        threadContextGroup_->functions.glBindBufferRange(GL_UNIFORM_BUFFER, i, buffer_uniform_id[i], buffer_uniform_offset[i], buffer_uniform_size[i]);
                }
            }
            for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                threadContext_->buffer_uniform_id    [i] = buffer_uniform_id    [i];
                threadContext_->buffer_uniform_offset[i] = buffer_uniform_offset[i];
                threadContext_->buffer_uniform_size  [i] = buffer_uniform_size  [i];
            }
            buffer_uniform_changedSlotMin = std::numeric_limits<decltype(buffer_uniform_changedSlotMin)>::max();
            buffer_uniform_changedSlotMax = -1;
        }
    }

    void PipelineInterface::processPendingChangesBuffersAtomicCounter() {
        auto changedSlotMin = buffer_atomicCounter_changedSlotMin;
        auto changedSlotMax = buffer_atomicCounter_changedSlotMax;

        if (changedSlotMin <= changedSlotMax) {
            if (threadContextGroup_->extensions.GL_ARB_multi_bind) {
                //Filter out unchanged slots at the beginning and end of the list
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext_->buffer_atomicCounter_id    [changedSlotMin] != buffer_atomicCounter_id    [changedSlotMin]
                    ||  threadContext_->buffer_atomicCounter_offset[changedSlotMin] != buffer_atomicCounter_offset[changedSlotMin]
                    ||  threadContext_->buffer_atomicCounter_size  [changedSlotMin] != buffer_atomicCounter_size  [changedSlotMin]) break;
                    changedSlotMin++;
                }
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext_->buffer_atomicCounter_id    [changedSlotMax] != buffer_atomicCounter_id    [changedSlotMax]
                    ||  threadContext_->buffer_atomicCounter_offset[changedSlotMax] != buffer_atomicCounter_offset[changedSlotMax]
                    ||  threadContext_->buffer_atomicCounter_size  [changedSlotMax] != buffer_atomicCounter_size  [changedSlotMax]) break;
                    changedSlotMax--;
                }

                if (changedSlotMin <= changedSlotMax) {
                    const uint32_t    count        = changedSlotMax - changedSlotMin + 1;
                    const uint32_t*   bufferIdList =                                     &buffer_atomicCounter_id    [changedSlotMin];
                    const GLintptr*   offsetList   = reinterpret_cast<const GLintptr*>  (&buffer_atomicCounter_offset[changedSlotMin]);
                    const GLsizeiptr* sizeList     = reinterpret_cast<const GLsizeiptr*>(&buffer_atomicCounter_size  [changedSlotMin]);
                    threadContextGroup_->functions.glBindBuffersRange(GL_ATOMIC_COUNTER_BUFFER, changedSlotMin, count, bufferIdList, offsetList, sizeList);
                }
            } else {
                for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                    if (threadContext_->buffer_atomicCounter_id    [i] != buffer_atomicCounter_id    [i]
                    ||  threadContext_->buffer_atomicCounter_offset[i] != buffer_atomicCounter_offset[i]
                    ||  threadContext_->buffer_atomicCounter_size  [i] != buffer_atomicCounter_size  [i])
                        threadContextGroup_->functions.glBindBufferRange(GL_ATOMIC_COUNTER_BUFFER, i, buffer_atomicCounter_id[i], buffer_atomicCounter_offset[i], buffer_atomicCounter_size[i]);
                }
            }
            for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                threadContext_->buffer_atomicCounter_id    [i] = buffer_atomicCounter_id    [i];
                threadContext_->buffer_atomicCounter_offset[i] = buffer_atomicCounter_offset[i];
                threadContext_->buffer_atomicCounter_size  [i] = buffer_atomicCounter_size  [i];
            }
            buffer_atomicCounter_changedSlotMin = std::numeric_limits<decltype(buffer_atomicCounter_changedSlotMin)>::max();
            buffer_atomicCounter_changedSlotMax = -1;
        }
    }

    /*
        We don't need glBindBufferBase/glBindBuffersBase here

            void glBindBufferBase(GLenum target, GLuint index, GLuint buffer);
            void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);

        GL_ARB_multi_bind (Core since 4.4):
            void glBindBuffersBase(GLenum target, GLuint first, GLsizei count, const GLuint *buffers);
            void glBindBuffersRange(GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLintptr *sizes);
    */
    void PipelineInterface::processPendingChangesBuffersShaderStorage() {
        auto changedSlotMin = buffer_shaderStorage_changedSlotMin;
        auto changedSlotMax = buffer_shaderStorage_changedSlotMax;

        if (changedSlotMin <= changedSlotMax) {
            if (threadContextGroup_->extensions.GL_ARB_multi_bind) {
                const uint32_t    count        = changedSlotMax - changedSlotMin + 1;
                const uint32_t*   bufferIdList =                                     &buffer_shaderStorage_id    [changedSlotMin];
                const GLintptr*   offsetList   = reinterpret_cast<const GLintptr*>  (&buffer_shaderStorage_offset[changedSlotMin]);
                const GLsizeiptr* sizeList     = reinterpret_cast<const GLsizeiptr*>(&buffer_shaderStorage_size  [changedSlotMin]);
                threadContextGroup_->functions.glBindBuffersRange(GL_SHADER_STORAGE_BUFFER, changedSlotMin, count, bufferIdList, offsetList, sizeList);
            } else {
                for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                    if (threadContext_->buffer_shaderStorage_id    [i] != buffer_shaderStorage_id    [i]
                    ||  threadContext_->buffer_shaderStorage_offset[i] != buffer_shaderStorage_offset[i]
                    ||  threadContext_->buffer_shaderStorage_size  [i] != buffer_shaderStorage_size  [i])
                        threadContextGroup_->functions.glBindBufferRange(GL_SHADER_STORAGE_BUFFER, i, buffer_shaderStorage_id[i], buffer_shaderStorage_offset[i], buffer_shaderStorage_size[i]);
                }
            }
            for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                threadContext_->buffer_shaderStorage_id    [i] = buffer_shaderStorage_id    [i];
                threadContext_->buffer_shaderStorage_offset[i] = buffer_shaderStorage_offset[i];
                threadContext_->buffer_shaderStorage_size  [i] = buffer_shaderStorage_size  [i];
            }
            buffer_shaderStorage_changedSlotMin = std::numeric_limits<decltype(buffer_shaderStorage_changedSlotMin)>::max();
            buffer_shaderStorage_changedSlotMax = -1;
        }
    }

    /*
     * glBindTextureUnit (Core since 4.5) is not used because we already have GL_ARB_multi_bind (Core since 4.4)
     */
    void PipelineInterface::processPendingChangesTextures() {
        auto changedSlotMin = texture_changedSlotMin;
        auto changedSlotMax = texture_changedSlotMax;

        if (changedSlotMin <= changedSlotMax) {
            if (threadContextGroup_->extensions.GL_ARB_multi_bind) {
                //Filter out unchanged slots at the beginning and end of the list
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext_->texture_id[changedSlotMin] != texture_id[changedSlotMin]) break;
                    changedSlotMin++;
                }
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext_->texture_id[changedSlotMax] != texture_id[changedSlotMax]) break;
                    changedSlotMax--;
                }

                if (changedSlotMin <= changedSlotMax) {
                          uint32_t  count       = changedSlotMax - changedSlotMin + 1;
                    const uint32_t* textureList = &texture_id[changedSlotMin];
                    threadContextGroup_->functions.glBindTextures(changedSlotMin, count, textureList);
                    for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                        threadContext_->texture_id[i] = texture_id[i];
                    }
                }
            } else {
                for (int i = changedSlotMin; i <= changedSlotMax; ++i)
                    threadContext_->cachedBindTextureCompatibleOrFirstTime(i, texture_target[i], texture_id[i]);
            }
            texture_changedSlotMin = std::numeric_limits<decltype(texture_changedSlotMin)>::max();
            texture_changedSlotMax = -1;
        }
    }

    void PipelineInterface::processPendingChangesSamplers() {
        auto changedSlotMin = sampler_changedSlotMin;
        auto changedSlotMax = sampler_changedSlotMax;

        if (changedSlotMin <= changedSlotMax) {
            if (threadContextGroup_->extensions.GL_ARB_multi_bind) {
                //Filter out unchanged slots at the beginning and end of the list
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext_->sampler_id[changedSlotMin] != sampler_id[changedSlotMin]) break;
                    changedSlotMin++;
                }
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext_->sampler_id[changedSlotMax] != sampler_id[changedSlotMax]) break;
                    changedSlotMax--;
                }

                if (changedSlotMin <= changedSlotMax) {
                    GLsizei count = changedSlotMax - changedSlotMin + 1;
                    const uint32_t* samplerList = &sampler_id[changedSlotMin];
                    threadContextGroup_->functions.glBindSamplers(changedSlotMin, count, samplerList);
                    for (int i = changedSlotMin; i <= changedSlotMax; ++i)
                        threadContext_->sampler_id[i] = sampler_id[i];
                }
            } else {
                for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                    if (threadContext_->sampler_id[i] != sampler_id[i]) {
                        threadContextGroup_->functions.glBindSampler(i, sampler_id[i]);
                        threadContext_->sampler_id[i] = sampler_id[i];
                    }
                }
            }
            sampler_changedSlotMin = std::numeric_limits<decltype(sampler_changedSlotMin)>::max();
            sampler_changedSlotMax = -1;
        }
    }

    /*
        The multi-bind version of glBindImageTexture (glBindImageTextures) is very limited in parameters and therefore depends on textureViews.
        I want support for the lowest version possible. So not depend on textureViews. (I prefer my direct set function with optional format parameter anyway!)

        Creating textureViews in the background, to maybe get some more performance for unchanging pipeline bindings, won't happen!
        They would have the side effect of keeping objects alive that the user already deleted.

        glBindImageTexture  (Core since 4.2)
        glTextureView       (Core since 4.3)
        glBindImageTextures (Core since 4.4)
    */
    void PipelineInterface::processPendingChangesImages() {
        auto changedSlotMin = image_changedSlotMin;
        auto changedSlotMax = image_changedSlotMax;

        if (changedSlotMin <= changedSlotMax) {
            for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                if (threadContext_->image_id         [i] != image_id         [i]
                ||  threadContext_->image_format     [i] != image_format     [i]
                ||  threadContext_->image_mipmapLevel[i] != image_mipmapLevel[i]
                ||  threadContext_->image_layer      [i] != image_layer      [i]) {
                    if (image_id[i]) {
                        if (image_layer[i] == -1) {
                            threadContextGroup_->functions.glBindImageTexture(i, image_id[i], image_mipmapLevel[i], 0,              0, GL_READ_WRITE, image_format[i]);
                        } else {
                            threadContextGroup_->functions.glBindImageTexture(i, image_id[i], image_mipmapLevel[i], 1, image_layer[i], GL_READ_WRITE, image_format[i]);
                        }
                    } else {
                        //Mesa does not like the format to be 0 even when the texture is 0, so we use GL_R8!
                        threadContextGroup_->functions.glBindImageTexture(i, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
                    }
                    threadContext_->image_id         [i] = image_id         [i];
                    threadContext_->image_format     [i] = image_format     [i];
                    threadContext_->image_mipmapLevel[i] = image_mipmapLevel[i];
                    threadContext_->image_layer      [i] = image_layer      [i];
                }
            }
            image_changedSlotMin = std::numeric_limits<decltype(image_changedSlotMin)>::max();
            image_changedSlotMax = -1;
        }
    }

    string PipelineInterface::glTypeToGlslName(int32_t type) {
        return gl::typeToGlslAndCNameString(type);
    }

    string PipelineInterface::glTypeToCppName(int32_t type) {
        return gl::typeToCppTypeNameString(type);
    }
}
