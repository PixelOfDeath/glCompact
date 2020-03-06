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
#include "glCompact/PipelineInterface.hpp"
#include "glCompact/Config.hpp"
#include "glCompact/Debug.hpp"
#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"
#include "glCompact/TextureInterface.hpp"
#include "glCompact/Sampler.hpp"
#include "glCompact/Buffer.hpp"
#include "glCompact/ToolsInternal.hpp"
#include "glCompact/gl/Helper.hpp"
#include "glCompact/SurfaceFormatDetail.hpp"

#include <glm/glm.hpp>

#include <regex>

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
            if (threadContext) {
                detachFromThreadContext();
            }
            if (threadContextGroup) {
                threadContextGroup->functions.glDeleteProgram(id);
            }
            id = 0;
        }
    }

    void PipelineInterface::setTexture(
        uint32_t                slot,
        const TextureInterface& texture
    ) {
        UNLIKELY_IF (!texture.id)
            throw std::runtime_error("empty texture not accepted!");
        UNLIKELY_IF (int32_t(slot) > sampler_highestActiveBinding) return; //TODO: would break if anyone uses slot value over 0x8FFFFFFFF

        texture_id    [slot] = texture.id;
        texture_target[slot] = texture.target;

        if (threadContext->shader == this) threadContext->texture_markSlotChange(slot);
    }

    void PipelineInterface::setTexture(
        uint32_t slot
    ) {
        if (int32_t(slot) > sampler_highestActiveBinding) return;

        texture_id    [slot] = 0;
        texture_target[slot] = 0;
        if (threadContext->shader == this) threadContext->texture_markSlotChange(slot);
    }

    void PipelineInterface::setTexture() {
        for (int32_t i = 0; i <= sampler_highestActiveBinding; ++i) {
            texture_id    [i] = 0;
            texture_target[i] = 0;
        }
        if (threadContext->shader == this) {
            threadContext->texture_markSlotChange(0);
            threadContext->texture_markSlotChange(sampler_highestActiveBinding);
        }
    }

    void PipelineInterface::setSampler(
        uint32_t       slot,
        const Sampler& sampler
    ) {
        if (int32_t(slot) > sampler_highestActiveBinding) return;

        sampler_id[slot] = sampler.id;
        if (threadContext->shader == this) threadContext->sampler_markSlotChange(slot);
    }

    void PipelineInterface::setSampler(
        uint32_t slot
    ) {
        if (int32_t(slot) > sampler_highestActiveBinding) return;

        sampler_id[slot] = 0;
        if (threadContext->shader == this) threadContext->sampler_markSlotChange(slot);
    }

    void PipelineInterface::setSampler() {
        for (int32_t i = 0; i <= sampler_highestActiveBinding; ++i) sampler_id[i] = 0;
        if (threadContext->shader == this) {
            threadContext->sampler_markSlotChange(0);
        }
    }

    void PipelineInterface::setUniformBuffer(
        uint32_t               slot,
        const BufferInterface& buffer,
        uintptr_t              offset
    ) {
        UNLIKELY_IF (!buffer.id)
            throw std::runtime_error("does not take empty Buffer!");
        UNLIKELY_IF (int32_t(slot) > buffer_uniform_highestActiveBinding) return;

        buffer_uniform_id    [slot] = buffer.id;
        buffer_uniform_offset[slot] = offset;
        buffer_uniform_size  [slot] = 0;
        if (threadContext->shader == this) threadContext->buffer_uniform_markSlotChange(slot);
    }

    void PipelineInterface::setUniformBuffer(
        uint32_t               slot,
        const BufferInterface& buffer,
        uintptr_t              offset,
        uintptr_t              size
    ) {
        UNLIKELY_IF (!buffer.id)
            throw std::runtime_error("does not take empty Buffer!");
        UNLIKELY_IF (int32_t(slot) > buffer_uniform_highestActiveBinding) return;

        buffer_uniform_id    [slot] = buffer.id;
        buffer_uniform_offset[slot] = offset;
        buffer_uniform_size  [slot] = size;
        if (threadContext->shader == this) threadContext->buffer_uniform_markSlotChange(slot);
    }

    void PipelineInterface::setUniformBuffer(
        uint32_t slot
    ) {
        if (int32_t(slot) > buffer_uniform_highestActiveBinding) return;

        buffer_uniform_id[slot] = 0;
        if (threadContext->shader == this) threadContext->buffer_uniform_markSlotChange(slot);
    }

    void PipelineInterface::setUniformBuffer() {
        for (int32_t i = 0; i <= buffer_uniform_highestActiveBinding; ++i) buffer_uniform_id[i] = 0;
        if (threadContext->shader == this) {
            threadContext->buffer_uniform_markSlotChange(0);
            threadContext->buffer_uniform_markSlotChange(buffer_uniform_highestActiveBinding);
        }
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
        if (int32_t(slot) > image_highestActiveBinding) return;

        image_id         [slot] = textureSelector.texture->id;
        image_format     [slot] = surfaceFormat->sizedFormat;
        image_mipmapLevel[slot] = textureSelector.mipmapLevel;
        image_layer      [slot] = textureSelector.layer;
        if (threadContext->shader == this) threadContext->image_markSlotChange(slot);
    }

    void PipelineInterface::setImage(
        uint32_t slot
    ) {
        if (int32_t(slot) > image_highestActiveBinding) return;

        image_id[slot] = 0;
        if (threadContext->shader == this) threadContext->image_markSlotChange(slot);
    }

    void PipelineInterface::setImage() {
        for (int32_t i = 0; i <= image_highestActiveBinding; ++i) image_id[i] = 0;

        if (threadContext->shader == this) {
            threadContext->image_changedSlotMin = 0;
            threadContext->image_changedSlotMax = max(threadContext->image_changedSlotMax, image_highestActiveBinding);
        }
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
        UNLIKELY_IF (int32_t(slot) > buffer_atomicCounter_highestActiveBinding) return;
        UNLIKELY_IF (buffer.getSize() - offset < atomicCounterBindingList[slot].dataSize)
            throw std::runtime_error(string("")
            +   "Buffer does not have enough space after offset for all atomic counters!\n"
            +   "size(" + to_string(buffer.getSize()) + ") - offset(" + to_string(offset) + ") = " + to_string(buffer.getSize() - offset) + " but needed size for all actomic counters is "
            +   to_string(atomicCounterBindingList[slot].dataSize) + "!"
            );

        buffer_atomicCounter_id    [slot] = buffer.id;
        buffer_atomicCounter_offset[slot] = offset;
        buffer_atomicCounter_size  [slot] = size;
        if (threadContext->shader == this) threadContext->buffer_atomicCounter_markSlotChange(slot);
    }

    void PipelineInterface::setAtomicCounterBuffer(
        uint32_t slot
    ) {
        if (int32_t(slot) > buffer_atomicCounter_highestActiveBinding) return;

        buffer_atomicCounter_id[slot] = 0;
        if (threadContext->shader == this) threadContext->buffer_atomicCounter_markSlotChange(slot);
    }

    void PipelineInterface::setAtomicCounterBuffer() {
        for (int32_t i = 0; i <= buffer_atomicCounter_highestActiveBinding; ++i) buffer_uniform_id[i] = 0;
        if (threadContext->shader == this) {
            threadContext->buffer_atomicCounter_markSlotChange(0);
            threadContext->buffer_atomicCounter_markSlotChange(buffer_atomicCounter_highestActiveBinding);
        }
    }

    void PipelineInterface::setShaderStorageBuffer(
        uint32_t         slot,
        BufferInterface& buffer,
        uintptr_t        offset
    ) {
        UNLIKELY_IF (!buffer.id)
            throw std::runtime_error("does not take empty Buffer!");
        UNLIKELY_IF (int32_t(slot) > buffer_shaderStorage_highestActiveBinding) return;

        setShaderStorageBuffer(slot, buffer, offset, buffer.size_);
    }

    void PipelineInterface::setShaderStorageBuffer(
        uint32_t         slot,
        BufferInterface& buffer,
        uintptr_t        offset,
        uintptr_t        size
    ) {
        UNLIKELY_IF (!buffer.id)
            throw std::runtime_error("does not take empty Buffer!");
        UNLIKELY_IF (int32_t(slot) > buffer_shaderStorage_highestActiveBinding) return;

        buffer_shaderStorage_id    [slot] = buffer.id;
        buffer_shaderStorage_offset[slot] = offset;
        buffer_shaderStorage_size  [slot] = size;
        if (threadContext->shader == this) threadContext->buffer_shaderStorage_markSlotChange(slot);
    }

    void PipelineInterface::setShaderStorageBuffer(
        uint32_t slot
    ) {
        if (int32_t(slot) > buffer_shaderStorage_highestActiveBinding) return;

        buffer_shaderStorage_id    [slot] = 0;
        buffer_shaderStorage_offset[slot] = 0;
        buffer_shaderStorage_size  [slot] = 0;
        if (threadContext->shader == this) threadContext->buffer_shaderStorage_markSlotChange(slot);
    }

    void PipelineInterface::setShaderStorageBuffer() {
        for (int i = 0; i <= buffer_shaderStorage_highestActiveBinding; ++i) {
            buffer_shaderStorage_id    [i] = 0;
            buffer_shaderStorage_offset[i] = 0;
            buffer_shaderStorage_size  [i] = 0;
        }
        if (threadContext->shader == this) {
            threadContext->buffer_shaderStorage_markSlotChange(0);
            threadContext->buffer_shaderStorage_markSlotChange(buffer_shaderStorage_highestActiveBinding);
        }
    }

    void PipelineInterface::detachFromThreadContext() {
        if (threadContext) {
            //TODO: also remove VAO that buffers vertex layout, because they only exist in the creator context
            if (threadContext->shader == this) threadContext->shader = 0;
        }
    }

    std::string PipelineInterface::getShaderInfoLog(
        uint32_t objId
    ) {
        uint32_t infologLength = 0;
         int32_t maxLengthReturn;
        uint32_t maxLength;
        std::vector<char> bytes;

        threadContextGroup->functions.glGetShaderiv(objId, GL_INFO_LOG_LENGTH, &maxLengthReturn);
        maxLength = maxLengthReturn;
        bytes.resize(maxLength + 1);
        threadContextGroup->functions.glGetShaderInfoLog(objId, maxLength, &infologLength, &bytes[0]);

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

        threadContextGroup->functions.glGetProgramiv(objId, GL_INFO_LOG_LENGTH, &maxLengthReturn);
        maxLength = maxLengthReturn;
        bytes.resize(maxLength + 1);
        threadContextGroup->functions.glGetProgramInfoLog(objId, maxLength, &infologLength, &bytes[0]);

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
        threadContextGroup->functions.glGetProgramBinary(id, binarySize, &bufSizeReturn, &binaryFormat, mem);
        return true;
    }

    bool PipelineInterface::copyBinaryFromMemory(
        GLenum      binaryFormat,
        GLsizei     bufSize,
        const void* mem
    ) {
        free();
        if (!threadContext->extensions.GL_ARB_get_program_binary) return false;
        id_ = threadContextGroup->functions.glCreateProgram();
        threadContextGroup->functions.glProgramBinary(id, binaryFormat, mem, bufSize);
        GLint linkStatus = 0;
        threadContextGroup->functions.glGetProgramiv(id, GL_LINK_STATUS, &linkStatus);
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
        if (threadContext->pipelineThatCausedLastWarning != this) {
            threadContext->pipelineThatCausedLastWarning = this;
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
        //return threadContextGroup->functions.glGetUniformLocation(id, uniformName.c_str());
        for (auto& uniform : uniformList)
            if (uniform.name == uniformName) return uniform.location;
        return -1;
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const GLfloat& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform1f(uniformLocation, value);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec2& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform2f(uniformLocation, value[0], value[1]);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec3& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform3f(uniformLocation, value[0], value[1], value[2]);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec4& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform4f(uniformLocation, value[0], value[1], value[2], value[3]);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const GLdouble& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform1d(uniformLocation, value);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec2& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform2d(uniformLocation, value[0], value[1]);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec3& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform3d(uniformLocation, value[0], value[1], value[2]);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec4& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform4d(uniformLocation, value[0], value[1], value[2], value[3]);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const int32_t& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform1i(uniformLocation, value);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec2& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform2i(uniformLocation, value[0], value[1]);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec3& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform3i(uniformLocation, value[0], value[1], value[2]);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec4& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform4i(uniformLocation, value[0], value[1], value[2], value[3]);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const GLuint& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform1ui(uniformLocation, value);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec2& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform2ui(uniformLocation, value[0], value[1]);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec3& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform3ui(uniformLocation, value[0], value[1], value[2]);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec4& value) {
        threadContext->cachedBindShader(shaderId);
        threadContextGroup->functions.glUniform4ui(uniformLocation, value[0], value[1], value[2], value[3]);
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const GLfloat& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform1fv(shaderId, uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniform1fvEXT(shaderId, uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform1fv(uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec2& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform2fv(shaderId, uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniform2fvEXT(shaderId, uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform2fv(uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec3& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform3fv(shaderId, uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniform3fvEXT(shaderId, uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform3fv(uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec4& value, int count) {
        if (threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform4fv(shaderId, uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        else if (threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniform4fvEXT(shaderId, uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform4fv(uniformLocation, count, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    //NOTE: There are no EXT DSA functions for setting double uniforms
    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const GLdouble& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform1dv(shaderId, uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform1dv(uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec2& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform2dv(shaderId, uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform2dv(uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec3 &value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform3dv(shaderId, uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform3dv(uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec4& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform4dv(shaderId, uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform4dv(uniformLocation, count, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const int32_t& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform1iv(shaderId, uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniform1ivEXT(shaderId, uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform1iv(uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec2& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform2iv(shaderId, uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniform2ivEXT(shaderId, uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform2iv(uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec3& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform3iv(shaderId, uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniform3ivEXT(shaderId, uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform3iv(uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec4& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform4iv(shaderId, uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniform4ivEXT(shaderId, uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform4iv(uniformLocation, count, reinterpret_cast<const GLint*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const GLuint& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform1uiv(shaderId, uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniform1uivEXT(shaderId, uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform1uiv(uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec2& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform2uiv(shaderId, uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniform2uivEXT(shaderId, uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform2uiv(uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec3& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform3uiv(shaderId, uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniform3uivEXT(shaderId, uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform3uiv(uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec4& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniform4uiv(shaderId, uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniform4uivEXT(shaderId, uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniform4uiv(uniformLocation, count, reinterpret_cast<const GLuint*>(&value));
        }
    }

    //NOTE: transpose can be done with glm, therefor we ignore the transpose value for the gl function
    //TODO: row-major order. or column-major the default?
    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat2x2& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix2fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniformMatrix2fvEXT(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix2fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat2x3& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix2x3fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniformMatrix2x3fvEXT(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix2x3fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat2x4& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix2x4fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniformMatrix2x4fvEXT(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix2x4fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat3x2& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix3x2fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniformMatrix3x2fvEXT(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix3x2fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat3x3& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix3fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniformMatrix3fvEXT(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix3fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat3x4& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix3x4fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniformMatrix3x4fvEXT(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix3x4fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat4x2& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix4x2fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniformMatrix4x2fvEXT(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix4x2fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat4x3& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix4x3fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniformMatrix4x3fvEXT(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix4x3fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat4x4& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix4fv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_EXT_direct_state_access)
            threadContextGroup->functions.glProgramUniformMatrix4fvEXT(shaderId, uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix4fv(uniformLocation, count, false, reinterpret_cast<const GLfloat*>(&value));
        }
    }

    //NOTE: There are no EXT DSA functions for setting double uniforms
    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat2x2& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix2dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix2dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat2x3& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix2x3dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix2x3dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat2x4& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix2x4dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix2x4dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat3x2& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix3x2dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix3x2dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat3x3& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix3dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix3dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat3x4& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix3x4dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix3x4dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat4x2& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix4x2dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix4x2dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat4x3& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix4x3dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix4x3dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat4x4& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS && threadContextGroup->extensions.GL_ARB_separate_shader_objects)
            threadContextGroup->functions.glProgramUniformMatrix4dv(shaderId, uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformMatrix4dv(uniformLocation, count, false, reinterpret_cast<const GLdouble*>(&value));
        }
    }

    //GL_ARB_bindless_texture (not core)
    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const uint64_t& value) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS) {
            threadContextGroup->functions.glProgramUniformHandleui64ARB(shaderId, uniformLocation, value);
        } else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformHandleui64ARB(uniformLocation, value);
        }
    }

    //GL_ARB_bindless_texture (not core)
    void PipelineInterface::setUniform(uint32_t shaderId, int32_t uniformLocation, const uint64_t& value, int count) {
        if (Config::ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS) {
            threadContextGroup->functions.glProgramUniformHandleui64vARB(shaderId, uniformLocation, count, reinterpret_cast<const GLuint64*>(&value));
        } else {
            threadContext->cachedBindShader(shaderId);
            threadContextGroup->functions.glUniformHandleui64vARB(uniformLocation, count, reinterpret_cast<const GLuint64*>(&value));
        }
    }

    void PipelineInterface::activate() {
        threadContext->attributeLayoutChanged = 1;

        threadContext->texture_changedSlotMin = 0;
        threadContext->texture_changedSlotMax = max(threadContext->texture_getHighestNonNull(), sampler_highestActiveBinding);
        threadContext->sampler_changedSlotMin = 0;
        threadContext->sampler_changedSlotMax = max(threadContext->sampler_getHighestNonNull(), sampler_highestActiveBinding);
        threadContext->buffer_uniform_changedSlotMin = 0;
        threadContext->buffer_uniform_changedSlotMax = max(threadContext->buffer_uniform_getHighestNonNull(), buffer_uniform_highestActiveBinding);


        threadContext->image_changedSlotMin   = 0;
        threadContext->image_changedSlotMax   = max(threadContext->image_getHighestNonNull(),   image_highestActiveBinding);
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
        threadContextGroup->functions.glGetUniformfv(shaderId, uniformLocation, &value);
    }

    void PipelineInterface::getUniform(uint32_t shaderId, int32_t uniformLocation, int32_t& value) {
        threadContextGroup->functions.glGetUniformiv(shaderId, uniformLocation, &value);
    }

    void PipelineInterface::getUniform(uint32_t shaderId, int32_t uniformLocation, uint32_t& value) {
        threadContextGroup->functions.glGetUniformuiv(shaderId, uniformLocation, &value);
    }

    void PipelineInterface::getUniform(uint32_t shaderId, int32_t uniformLocation, double& value) {
        threadContextGroup->functions.glGetUniformdv(shaderId, uniformLocation, &value);
    }

    void PipelineInterface::getUniform(uint32_t shaderId, int32_t uniformLocation, float& value, int count) {
        threadContextGroup->functions.glGetnUniformfv(shaderId, uniformLocation, count, &value);
    }

    void PipelineInterface::getUniform(uint32_t shaderId, int32_t uniformLocation, int32_t& value, int count) {
        threadContextGroup->functions.glGetnUniformiv(shaderId, uniformLocation, count, &value);
    }

    void PipelineInterface::getUniform(uint32_t shaderId, int32_t uniformLocation, uint32_t& value, int count) {
        threadContextGroup->functions.glGetnUniformuiv(shaderId, uniformLocation, count, &value);
    }

    void PipelineInterface::getUniform(uint32_t shaderId, int32_t uniformLocation, double& value, int count) {
        threadContextGroup->functions.glGetnUniformdv(shaderId, uniformLocation, count, &value);
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

    //if the uniform name ends in [0] it is an array. (this may not be the case on some "shaky" GLES drivers...)
    //TODO: move the regex into static object, maybe even use one of this fancy static regex libs...
    static bool ifStringHasArrayBracketsRemoveThemAndReturnTrue(
        string& name
    ) {
        static regex expression(R"""(^(.+)\[0\]$)""");
        smatch match;
        if (regex_search(name, match, expression)) {
            name = match.str(1);
            return true;
        } else {
            return false;
        }
    }

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

        //get infos about all uniforms. This includes uniforms from UBOs and SSBOs. Whose uniform location are -1.
        struct RawUniform {
            string  name;
            bool    nameIsArray;
            int32_t location;
            int32_t type;
            int32_t arraySize;
        };
        vector<RawUniform> rawUniformList;

        int32_t activeUniformCount         = 0;
        int32_t activeUniformNameLengthMax = 0;
        threadContextGroup->functions.glGetProgramiv(id, GL_ACTIVE_UNIFORMS,           &activeUniformCount);
        threadContextGroup->functions.glGetProgramiv(id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &activeUniformNameLengthMax);
        rawUniformList.resize(activeUniformCount);
        nameBuffer.resize(activeUniformNameLengthMax);
        LOOPI(activeUniformCount) {
            auto& ru = rawUniformList[i];
            uint32_t stringLenght = 0;
            threadContextGroup->functions.glGetActiveUniform(id, uint32_t(i), activeUniformNameLengthMax, &stringLenght, &ru.arraySize, &ru.type, &nameBuffer[0]);
            ru.location = threadContextGroup->functions.glGetUniformLocation(id, &nameBuffer[0]);
            string name(&nameBuffer[0], stringLenght);
            ru.nameIsArray = ifStringHasArrayBracketsRemoveThemAndReturnTrue(name);
            ru.name = name;
            if (ru.arraySize == 1 && !ru.nameIsArray) ru.arraySize = 0;

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
                if (binding >= Config::MAX_SAMPLER_BINDINGS)
                    error(" Trying to use sampler binding that is outside the range of Config::MAX_SAMPLER_BINDINGS(" + to_string(Config::MAX_SAMPLER_BINDINGS) + " = 0.." + to_string(Config::MAX_SAMPLER_BINDINGS-1) + ")\n"
                    + " -> " + ru.name);
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
                if (binding >= Config::MAX_IMAGE_BINDINGS)
                    error(" Trying to use sampler binding that is outside the range of Config::MAX_IMAGE_BINDINGS(" + to_string(Config::MAX_IMAGE_BINDINGS) + " = 0.." + to_string(Config::MAX_IMAGE_BINDINGS-1) + ")\n"
                    + " -> " + ru.name);
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
                u.name      = ru.name;
                u.location  = ru.location;
                u.type      = ru.type;
                u.arraySize = ru.arraySize;
                uniformList.push_back(u);
            }
        }

        //Uniform Buffer Object (UBO) (Core since 3.1)
        //Note: explicitly allowes different uniform blocks to use the same binding!
        if (threadContextGroup->extensions.GL_ARB_uniform_buffer_object) {
            int32_t activeUniformBlockCount         = 0;
            int32_t activeUniformBlockNameLengthMax = 0;
            threadContextGroup->functions.glGetProgramiv(id, GL_ACTIVE_UNIFORM_BLOCKS,                &activeUniformBlockCount);
            threadContextGroup->functions.glGetProgramiv(id, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &activeUniformBlockNameLengthMax);
            nameBuffer.resize(activeUniformBlockNameLengthMax);
            uniformBlockList.resize(activeUniformBlockCount);

            LOOPI(activeUniformBlockCount) {
                auto& uniformBlock = uniformBlockList[i];
                uint32_t stringLenght = 0;
                int32_t uniformCount;
                threadContextGroup->functions.glGetActiveUniformBlockName(id, uint32_t(i), activeUniformBlockNameLengthMax, &stringLenght, &nameBuffer[0]);
                uniformBlock.name = string(&nameBuffer[0], stringLenght);
                int32_t binding = getBindingFromString(uniformBlock.name);
                if (binding >= Config::MAX_UNIFORM_BUFFER_BINDINGS)
                    error(string("")
                        + "Trying to use uniform block binding outisde of valid range of Config::MAX_UNIFORM_BUFFER_BINDINGS(" + to_string(Config::MAX_UNIFORM_BUFFER_BINDINGS) + " = 0.." + to_string(Config::MAX_UNIFORM_BUFFER_BINDINGS - 1) + ")\n"
                        + " -> " + uniformBlock.name
                    );
                int32_t blockIndex = threadContextGroup->functions.glGetUniformBlockIndex(id, &nameBuffer[0]);
                int32_t layoutQualifierBinding;
                threadContextGroup->functions.glGetActiveUniformBlockiv(id, blockIndex, GL_UNIFORM_BLOCK_BINDING, &layoutQualifierBinding);
                if (layoutQualifierBinding && layoutQualifierBinding != binding)
                    error(string("")
                        + "Uniform block uses different GLSL layout qualifier binding values then glCompact NAME_bindingX binding! But must be the same!"
                        + " -> layout(binding = " + to_string(layoutQualifierBinding) + ") " + uniformBlock.name
                    );
                threadContextGroup->functions.glUniformBlockBinding(id, blockIndex, binding);
                threadContextGroup->functions.glGetActiveUniformBlockiv(id, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &uniformBlock.blockSize);
                threadContextGroup->functions.glGetActiveUniformBlockiv(id, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &uniformCount);
                vector<uint32_t> uniformIndexList(uniformCount); //index into raw uniform list
                threadContextGroup->functions.glGetActiveUniformBlockiv(id, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, reinterpret_cast< int32_t*>(&uniformIndexList[0]));

                vector< int32_t> uniformOffsetList          (uniformCount);
                vector< int32_t> uniformArrayStrideList     (uniformCount); //?
                vector< int32_t> uniformMatrixStrideList    (uniformCount); //?
                vector< int32_t> uniformMatricIsRowMajorList(uniformCount); // 0 = column-major matrix, 1 = row-major matrix
                threadContextGroup->functions.glGetActiveUniformsiv(id, uniformCount, &uniformIndexList[0], GL_UNIFORM_OFFSET,          &uniformOffsetList          [0]);
                threadContextGroup->functions.glGetActiveUniformsiv(id, uniformCount, &uniformIndexList[0], GL_UNIFORM_ARRAY_STRIDE,    &uniformArrayStrideList     [0]);
                threadContextGroup->functions.glGetActiveUniformsiv(id, uniformCount, &uniformIndexList[0], GL_UNIFORM_MATRIX_STRIDE,   &uniformMatrixStrideList    [0]);
                threadContextGroup->functions.glGetActiveUniformsiv(id, uniformCount, &uniformIndexList[0], GL_UNIFORM_IS_ROW_MAJOR,    &uniformMatricIsRowMajorList[0]);

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
        if (threadContextGroup->extensions.GL_ARB_shader_atomic_counters) {
            //glGetActiveUniformsiv UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX
            //glGetActiveAtomicCounterBufferiv( GLuint program, GLuint bufferIndex, GLenum pname, GLint *params);
            int32_t activeAtomicCounter = 0;
            threadContextGroup->functions.glGetProgramiv(id, GL_ACTIVE_ATOMIC_COUNTER_BUFFERS, &activeAtomicCounter);
            atomicCounterBindingList.resize(activeAtomicCounter);
            LOOPI(activeAtomicCounter) {
                int32_t binding;
                threadContextGroup->functions.glGetActiveAtomicCounterBufferiv(id, i, GL_ATOMIC_COUNTER_BUFFER_BINDING, &binding);
                if (atomicCounterBindingList.size() < uint32_t(binding) + 1) atomicCounterBindingList.resize(binding + 1);
                auto& acbb = atomicCounterBindingList[binding];

                threadContextGroup->functions.glGetActiveAtomicCounterBufferiv(id, i, GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE, reinterpret_cast<int32_t*>(&acbb.dataSize));
                int32_t atomicCountersCount = 0;
                threadContextGroup->functions.glGetActiveAtomicCounterBufferiv(id, i, GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS, &atomicCountersCount);
                vector<int32_t> atomicCounterIndexList(atomicCountersCount);
                vector<int32_t> uniformOffsetList     (atomicCountersCount);
                threadContextGroup->functions.glGetActiveAtomicCounterBufferiv(id, i, GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES, &atomicCounterIndexList[0]);
                threadContextGroup->functions.glGetActiveUniformsiv(id, atomicCountersCount, reinterpret_cast<uint32_t*>(&atomicCounterIndexList[0]), GL_UNIFORM_OFFSET, &uniformOffsetList[0]);
                acbb.uniformList.resize(atomicCountersCount);
                LOOPI(atomicCountersCount) {
                    auto& ac = acbb.uniformList[i];
                    ac.name   = rawUniformList[atomicCounterIndexList[i]].name;
                    ac.offset = uniformOffsetList                    [i];
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
        if (threadContextGroup->extensions.GL_ARB_shader_storage_buffer_object) {
            int32_t activeSSBCount;
            int32_t activeSSBMaxNameLength;
            threadContextGroup->functions.glGetProgramInterfaceiv(id, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &activeSSBCount);
            threadContextGroup->functions.glGetProgramInterfaceiv(id, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH,  &activeSSBMaxNameLength);
            nameBuffer.resize(activeSSBMaxNameLength);
            //rawStorageBlockList.resize(activeSSBCount);
            //if (activeSSBCount) cout << " RAW SSB (" << to_string(activeSSBCount) << "):" << endl;

            LOOPI(activeSSBCount) {
                StorageBlock sb;
                uint32_t nameLength = 0;
                threadContextGroup->functions.glGetProgramResourceName(id, GL_SHADER_STORAGE_BLOCK, i, activeSSBMaxNameLength, &nameLength, &nameBuffer[0]);
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
                threadContextGroup->functions.glGetProgramResourceiv(id, GL_SHADER_STORAGE_BLOCK, i, queryCount, activeVariableCountQuery, queryCount, nullptr, reinterpret_cast<int32_t*>(&query));
                sb.variable.resize(query.activeVariableCount);
                vector<int32_t> activeVariableIndexList(query.activeVariableCount);
                int32_t activeVariableIndexListQuery[] = {GL_ACTIVE_VARIABLES};
                threadContextGroup->functions.glGetProgramResourceiv(id, GL_SHADER_STORAGE_BLOCK, i, 1, activeVariableIndexListQuery, query.activeVariableCount, nullptr, &activeVariableIndexList[0]);
                LOOPJ(query.activeVariableCount) {
                    auto& variable = sb.variable[j];

                    int32_t bufferVariableNameLengthWithNullTerminator = 0;
                    int32_t nameLengthQuery[] = {GL_NAME_LENGTH};
                    threadContextGroup->functions.glGetProgramResourceiv(id, GL_BUFFER_VARIABLE, activeVariableIndexList[j], 1, nameLengthQuery, 1, nullptr, &bufferVariableNameLengthWithNullTerminator);
                    if (nameBuffer.size() < uint32_t(bufferVariableNameLengthWithNullTerminator)) nameBuffer.resize(bufferVariableNameLengthWithNullTerminator);
                    threadContextGroup->functions.glGetProgramResourceName(id, GL_BUFFER_VARIABLE, activeVariableIndexList[j], bufferVariableNameLengthWithNullTerminator, nullptr, &nameBuffer[0]);
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
                    int32_t queryCount = sizeof(bufferVariableDataQuery) / 4;
                    threadContextGroup->functions.glGetProgramResourceiv(id, GL_BUFFER_VARIABLE, activeVariableIndexList[j], queryCount, bufferVariableDataQuery, queryCount, nullptr, reinterpret_cast<int32_t*>(&queryData));

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
        sampler_highestActiveBinding               = samplerList.size();
        for (const auto& ub : uniformBlockList) buffer_uniform_highestActiveBinding = max(buffer_uniform_highestActiveBinding, ub.binding);
        image_highestActiveBinding                 = imageList.size();
        buffer_atomicCounter_highestActiveBinding  = atomicCounterBindingList.size();
        for (const auto& sb : storageBlockList) buffer_shaderStorage_highestActiveBinding = max(buffer_shaderStorage_highestActiveBinding, sb.binding);
    }

    void PipelineInterface::processPendingChanges() {
        processPendingChangesBuffersUniform();
        processPendingChangesBuffersAtomicCounter();
        processPendingChangesBuffersShaderstorage();
        processPendingChangesTextures();
        processPendingChangesSamplers();
        processPendingChangesImages();

        threadContext->processPendingChangesBarriers();
    }

    void PipelineInterface::processPendingChangesBuffersUniform() {
        auto changedSlotMin = threadContext->buffer_uniform_changedSlotMin;
        auto changedSlotMax = threadContext->buffer_uniform_changedSlotMax;

        if (changedSlotMin <= changedSlotMax) {
            if (threadContextGroup->extensions.GL_ARB_multi_bind) {
                //Filter out unchanged slots at the beginning and end of the list
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext->buffer_uniform_id    [changedSlotMin] != buffer_uniform_id    [changedSlotMin]
                    ||  threadContext->buffer_uniform_offset[changedSlotMin] != buffer_uniform_offset[changedSlotMin]
                    ||  threadContext->buffer_uniform_size  [changedSlotMin] != buffer_uniform_size  [changedSlotMin]) break;
                    changedSlotMin++;
                }
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext->buffer_uniform_id    [changedSlotMax] != buffer_uniform_id    [changedSlotMax]
                    ||  threadContext->buffer_uniform_offset[changedSlotMax] != buffer_uniform_offset[changedSlotMax]
                    ||  threadContext->buffer_uniform_size  [changedSlotMax] != buffer_uniform_size  [changedSlotMax]) break;
                    changedSlotMax--;
                }

                if (changedSlotMin <= changedSlotMax) {
                    const uint32_t    count        = changedSlotMax - changedSlotMin + 1;
                    const uint32_t*   bufferIdList =                                     &buffer_uniform_id    [changedSlotMin];
                    const GLintptr*   offsetList   = reinterpret_cast<const GLintptr*>  (&buffer_uniform_offset[changedSlotMin]);
                    const GLsizeiptr* sizeList     = reinterpret_cast<const GLsizeiptr*>(&buffer_uniform_size  [changedSlotMin]);
                    threadContextGroup->functions.glBindBuffersRange(GL_UNIFORM_BUFFER, changedSlotMin, count, bufferIdList, offsetList, sizeList);
                }
            } else {
                for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                    if (threadContext->buffer_uniform_id    [i] != buffer_uniform_id    [i]
                    ||  threadContext->buffer_uniform_offset[i] != buffer_uniform_offset[i]
                    ||  threadContext->buffer_uniform_size  [i] != buffer_uniform_size  [i])
                        threadContextGroup->functions.glBindBufferRange(GL_UNIFORM_BUFFER, i, buffer_uniform_id[i], buffer_uniform_offset[i], buffer_uniform_size[i]);
                }
            }
            for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                threadContext->buffer_uniform_id    [i] = buffer_uniform_id    [i];
                threadContext->buffer_uniform_offset[i] = buffer_uniform_offset[i];
                threadContext->buffer_uniform_size  [i] = buffer_uniform_size  [i];
            }
            threadContext->buffer_uniform_changedSlotMin = Config::MAX_UNIFORM_BUFFER_BINDINGS;
            threadContext->buffer_uniform_changedSlotMax = -1;
        }
    }

    void PipelineInterface::processPendingChangesBuffersAtomicCounter() {
        auto changedSlotMin = threadContext->buffer_atomicCounter_changedSlotMin;
        auto changedSlotMax = threadContext->buffer_atomicCounter_changedSlotMax;

        if (changedSlotMin <= changedSlotMax) {
            if (threadContextGroup->extensions.GL_ARB_multi_bind) {
                //Filter out unchanged slots at the beginning and end of the list
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext->buffer_atomicCounter_id    [changedSlotMin] != buffer_atomicCounter_id    [changedSlotMin]
                    ||  threadContext->buffer_atomicCounter_offset[changedSlotMin] != buffer_atomicCounter_offset[changedSlotMin]
                    ||  threadContext->buffer_atomicCounter_size  [changedSlotMin] != buffer_atomicCounter_size  [changedSlotMin]) break;
                    changedSlotMin++;
                }
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext->buffer_atomicCounter_id    [changedSlotMax] != buffer_atomicCounter_id    [changedSlotMax]
                    ||  threadContext->buffer_atomicCounter_offset[changedSlotMax] != buffer_atomicCounter_offset[changedSlotMax]
                    ||  threadContext->buffer_atomicCounter_size  [changedSlotMax] != buffer_atomicCounter_size  [changedSlotMax]) break;
                    changedSlotMax--;
                }

                if (changedSlotMin <= changedSlotMax) {
                    const uint32_t    count        = changedSlotMax - changedSlotMin + 1;
                    const uint32_t*   bufferIdList =                                     &buffer_atomicCounter_id    [changedSlotMin];
                    const GLintptr*   offsetList   = reinterpret_cast<const GLintptr*>  (&buffer_atomicCounter_offset[changedSlotMin]);
                    const GLsizeiptr* sizeList     = reinterpret_cast<const GLsizeiptr*>(&buffer_atomicCounter_size  [changedSlotMin]);
                    threadContextGroup->functions.glBindBuffersRange(GL_ATOMIC_COUNTER_BUFFER, changedSlotMin, count, bufferIdList, offsetList, sizeList);
                }
            } else {
                for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                    if (threadContext->buffer_atomicCounter_id    [i] != buffer_atomicCounter_id    [i]
                    ||  threadContext->buffer_atomicCounter_offset[i] != buffer_atomicCounter_offset[i]
                    ||  threadContext->buffer_atomicCounter_size  [i] != buffer_atomicCounter_size  [i])
                        threadContextGroup->functions.glBindBufferRange(GL_ATOMIC_COUNTER_BUFFER, i, buffer_atomicCounter_id[i], buffer_atomicCounter_offset[i], buffer_atomicCounter_size[i]);
                }
            }
            for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                threadContext->buffer_atomicCounter_id    [i] = buffer_atomicCounter_id    [i];
                threadContext->buffer_atomicCounter_offset[i] = buffer_atomicCounter_offset[i];
                threadContext->buffer_atomicCounter_size  [i] = buffer_atomicCounter_size  [i];
            }
            threadContext->buffer_atomicCounter_changedSlotMin = Config::MAX_ATOMIC_COUNTER_BUFFER_BINDINGS;
            threadContext->buffer_atomicCounter_changedSlotMax = -1;
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
    void PipelineInterface::processPendingChangesBuffersShaderstorage() {
        auto changedSlotMin = threadContext->buffer_shaderStorage_changedSlotMin;
        auto changedSlotMax = threadContext->buffer_shaderStorage_changedSlotMax;

        if (changedSlotMin <= changedSlotMax) {
            if (threadContextGroup->extensions.GL_ARB_multi_bind) {
                const uint32_t    count        = changedSlotMax - changedSlotMin + 1;
                const uint32_t*   bufferIdList =                                     &buffer_shaderStorage_id    [changedSlotMin];
                const GLintptr*   offsetList   = reinterpret_cast<const GLintptr*>  (&buffer_shaderStorage_offset[changedSlotMin]);
                const GLsizeiptr* sizeList     = reinterpret_cast<const GLsizeiptr*>(&buffer_shaderStorage_size  [changedSlotMin]);
                threadContextGroup->functions.glBindBuffersRange(GL_SHADER_STORAGE_BUFFER, changedSlotMin, count, bufferIdList, offsetList, sizeList);
            } else {
                for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                    if (threadContext->buffer_shaderStorage_id    [i] != buffer_shaderStorage_id    [i]
                    ||  threadContext->buffer_shaderStorage_offset[i] != buffer_shaderStorage_offset[i]
                    ||  threadContext->buffer_shaderStorage_size  [i] != buffer_shaderStorage_size  [i])
                        threadContextGroup->functions.glBindBufferRange(GL_SHADER_STORAGE_BUFFER, i, buffer_shaderStorage_id[i], buffer_shaderStorage_offset[i], buffer_shaderStorage_size[i]);
                }
            }
            for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                threadContext->buffer_shaderStorage_id    [i] = buffer_shaderStorage_id    [i];
                threadContext->buffer_shaderStorage_offset[i] = buffer_shaderStorage_offset[i];
                threadContext->buffer_shaderStorage_size  [i] = buffer_shaderStorage_size  [i];
            }
            threadContext->buffer_shaderStorage_changedSlotMin = Config::MAX_SHADERSTORAGE_BUFFER_BINDINGS;
            threadContext->buffer_shaderStorage_changedSlotMax = -1;
        }
    }

    /*
     * glBindTextureUnit (Core since 4.5) is not used because we already have GL_ARB_multi_bind (Core since 4.4)
     */
    void PipelineInterface::processPendingChangesTextures() {
        auto changedSlotMin = threadContext->texture_changedSlotMin;
        auto changedSlotMax = threadContext->texture_changedSlotMax;

        if (changedSlotMin <= changedSlotMax) {
            if (threadContextGroup->extensions.GL_ARB_multi_bind) {
                //Filter out unchanged slots at the beginning and end of the list
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext->texture_id[changedSlotMin] != texture_id[changedSlotMin]) break;
                    changedSlotMin++;
                }
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext->texture_id[changedSlotMax] != texture_id[changedSlotMax]) break;
                    changedSlotMax--;
                }

                if (changedSlotMin <= changedSlotMax) {
                          uint32_t  count       = changedSlotMax - changedSlotMin + 1;
                    const uint32_t* textureList = &texture_id[changedSlotMin];
                    threadContextGroup->functions.glBindTextures(changedSlotMin, count, textureList);
                    for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                        threadContext->texture_id[i] = texture_id[i];
                    }
                }
            } else {
                //Classical texture binding in OpenGL makes it possible to bind multiple texture types (e.g. TEXTURE_2D and TEXTURE_3D) at the same slot at the same time.
                //This works fine if you only want to edit the object states, but for rendering OpenGL only accepts one active type at each texture slot.
                //Therefore if we bind a texture type we must make sure that other texture type bindings at the same slot are set to 0!
                //In glCompact there is never more then one texture type set in a single texture slot!

                //This does not include the case if the same texture ID changes target.
                //But in glCompact the old style of redefining the texture type of an texture ID is not possible!
                //And the texture deleting function will always remove the texture from the context state tracker.

                for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                    bool targetChange    = threadContext->texture_target[i] != texture_target[i];
                    bool textureChange   = threadContext->texture_id    [i] != texture_id    [i];
                    bool unbindOldTarget = targetChange  &&     threadContext->texture_id    [i];
                    bool bindNewTexture  = textureChange &&                            texture_id    [i];
                    if (unbindOldTarget || bindNewTexture) threadContext->cachedSetActiveTexture(i);
                    if (unbindOldTarget)                   threadContextGroup->functions.glBindTexture(threadContext->texture_target[i], 0);
                    if (bindNewTexture)                    threadContextGroup->functions.glBindTexture(                       texture_target[i], texture_id[i]);
                    if (targetChange)                      threadContext->texture_target[i] = texture_target[i];
                    if (textureChange)                     threadContext->texture_id    [i] = texture_id    [i];
                }
            }
            threadContext->texture_changedSlotMin = Config::MAX_SAMPLER_BINDINGS;
            threadContext->texture_changedSlotMax = -1;
        }
    }

    void PipelineInterface::processPendingChangesSamplers() {
        auto changedSlotMin = threadContext->sampler_changedSlotMin;
        auto changedSlotMax = threadContext->sampler_changedSlotMax;

        if (changedSlotMin <= changedSlotMax) {
            if (threadContextGroup->extensions.GL_ARB_multi_bind) {
                //Filter out unchanged slots at the beginning and end of the list
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext->sampler_id[changedSlotMin] != sampler_id[changedSlotMin]) break;
                    changedSlotMin++;
                }
                while (changedSlotMin <= changedSlotMax) {
                    if (threadContext->sampler_id[changedSlotMax] != sampler_id[changedSlotMax]) break;
                    changedSlotMax--;
                }

                if (changedSlotMin <= changedSlotMax) {
                    GLsizei count = changedSlotMax - changedSlotMin + 1;
                    const uint32_t* samplerList = &sampler_id[changedSlotMin];
                    threadContextGroup->functions.glBindSamplers(changedSlotMin, count, samplerList);
                    for (int i = changedSlotMin; i <= changedSlotMax; ++i)
                        threadContext->sampler_id[i] = sampler_id[i];
                }
            } else {
                for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                    if (threadContext->sampler_id[i] != sampler_id[i]) {
                        threadContextGroup->functions.glBindSampler(i, sampler_id[i]);
                        threadContext->sampler_id[i] = sampler_id[i];
                    }
                }
            }
            threadContext->sampler_changedSlotMin = Config::MAX_SAMPLER_BINDINGS;
            threadContext->sampler_changedSlotMax = -1;
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
        auto changedSlotMin = threadContext->image_changedSlotMin;
        auto changedSlotMax = threadContext->image_changedSlotMax;

        if (changedSlotMin <= changedSlotMax) {
            for (int i = changedSlotMin; i <= changedSlotMax; ++i) {
                if (threadContext->image_id         [i] != image_id         [i]
                ||  threadContext->image_format     [i] != image_format     [i]
                ||  threadContext->image_mipmapLevel[i] != image_mipmapLevel[i]
                ||  threadContext->image_layer      [i] != image_layer      [i]) {
                    if (image_id[i]) {
                        if (image_layer[i] == -1) {
                            threadContextGroup->functions.glBindImageTexture(i, image_id[i], image_mipmapLevel[i], 0,              0, GL_READ_WRITE, image_format[i]);
                        } else {
                            threadContextGroup->functions.glBindImageTexture(i, image_id[i], image_mipmapLevel[i], 1, image_layer[i], GL_READ_WRITE, image_format[i]);
                        }
                    } else {
                        threadContextGroup->functions.glBindImageTexture(i, 0, 0, GL_FALSE, 0, GL_READ_ONLY, 0); //GL_R8);
                    }
                    threadContext->image_id         [i] = image_id         [i];
                    threadContext->image_format     [i] = image_format     [i];
                    threadContext->image_mipmapLevel[i] = image_mipmapLevel[i];
                    threadContext->image_layer      [i] = image_layer      [i];
                }
            }
            threadContext->image_changedSlotMin = Config::MAX_IMAGE_BINDINGS;
            threadContext->image_changedSlotMax = -1;
        }
    }
}
