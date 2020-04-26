#pragma once
#include "glCompact/BufferInterface.hpp"
#include "glCompact/TextureInterface.hpp"
#include "glCompact/TextureSelector.hpp"
#include "glCompact/SurfaceFormat.hpp"
#include "glCompact/Sampler.hpp"

#include <glm/fwd.hpp>

#include <string>
#include <vector>
#include <initializer_list> //We need to use initializer_list, because GCC has broken template arrays for unsigned int types until something like GCC 8.3
#include <limits>

namespace glCompact {
    class PipelineInterface {
            friend class Context;
            friend class Sampler;
        public:
            void setTexture              (uint32_t slot, const TextureInterface& texture);
            void setTexture              (uint32_t slot);
            void setTexture              ();

            void setSampler              (uint32_t slot, const Sampler& sampler);
            void setSampler              (uint32_t slot);
            void setSampler              ();

            void setUniformBuffer        (uint32_t slot, const BufferInterface& buffer, uintptr_t offset = 0);
            void setUniformBuffer        (uint32_t slot, const BufferInterface& buffer, uintptr_t offset, uintptr_t size);
            void setUniformBuffer        (uint32_t slot);
            void setUniformBuffer        ();

            void setImage                (uint32_t slot, TextureSelector textureSelector);
            void setImage                (uint32_t slot, TextureSelector textureSelector, SurfaceFormat surfaceFormat);
            void setImage                (uint32_t slot);
            void setImage                ();

            void setAtomicCounterBuffer  (uint32_t slot, const BufferInterface& buffer, uintptr_t offset = 0);
            void setAtomicCounterBuffer  (uint32_t slot, const BufferInterface& buffer, uintptr_t offset, uintptr_t size);
            void setAtomicCounterBuffer  (uint32_t slot);
            void setAtomicCounterBuffer  ();

            void setShaderStorageBuffer  (uint32_t slot,       BufferInterface& buffer, uintptr_t offset = 0);
            void setShaderStorageBuffer  (uint32_t slot,       BufferInterface& buffer, uintptr_t offset, uintptr_t size);
            void setShaderStorageBuffer  (uint32_t slot);
            void setShaderStorageBuffer  ();

          //void unbindAll();

            const std::string getInfoLog(){return infoLog_;}

            void detachFromThreadContext();

            //This needs GL_ARB_get_program_binary (Core since 4.1)
            //on amd cards getting the binary works only if binaryRetrievableHint is set to true (not tested on nvidia, intel)
            //Mesa drivers fake support for GL_ARB_get_program_binary by always rejecting to load the shader binary, to "fulfill" the minimum requirement for GL 4.1
            //Modern drivers do this shader caching by them self (and more efficient!) in the background anyway. So it makes no sense to include this in the API.
            //The only use I can come up with is injecting self made bytecode for specific GPUs
            /*bool copyBinaryToMemory  (GLenum& binaryFormat, GLsizei bufSize, void* mem) const;
            bool copyBinaryFromMemory(GLenum binaryFormat, GLsizei bufSize, const void* mem);
            bool copyBinaryToFile    (const std::string& fileName) const;
            bool copyBinaryFromFile  (const std::string& fileName);*/
        protected:
            /** UniformSetter
                \brief class template to create an interface for a shader uniform or uniform array
                \tparam T type must fit to the type used in the shader

                \details Example usage pattern:
                <pre>
                class MyPipelineRasterization : public glCompact::PipelineRasterization {
                    public:
                        UniformSetter<float>     myUniform1{this, "myUniformName1"};
                        UniformSetter<glm::vec2> myUniform2{this, "myUniformName2"};

                        //Initalizing the first value
                        UniformSetter<float>     myUniform3{this, "myUniformName3", 0.5f};
                        UniformSetter<glm::vec2> myUniform4{this, "myUniformName4", {3.0f, 4.0f}};

                        //Initalizing several values
                        UniformSetter<float>     myUniform5{this, "myUniformName5", {0.5f, 0.6f, 0.7f}};
                        UniformSetter<glm::vec2> myUniform6{this, "myUniformName6", {{3.0f, 4.0f}, {5.0f, 6.0f}, {7.0f, 8.0f}}};
                };

                MyPipelineRasterization myPipelineRasterization(vertexShaderString, "", "", "", fragmentShaderString);

                //set the first value
                myPipelineRasterization.myUniform1    = 2.0f;

                //set the second value (Index starts at 0)
                myPipelineRasterization.myUniform1[1] = 2.0f;

                //set several values
                myPipelineRasterization.myUniform1    = {2.0f, 3.0f};

                //set several values at index
                myPipelineRasterization.myUniform1[2] = {2.0f, 3.0f};

                //set several values of type with constructor that itself takes several values e.g. vec2
                myPipelineRasterization.myUniform2    = {{2.0f, 3.0f}, {2.1f, 3.1f}, {2.2f, 3.2f}};
                </pre>
            */
            template<typename T>
            class UniformSetter {
                public:
                    UniformSetter(PipelineInterface* const pParent, const std::string& uniformName) {
                        constexpr int32_t cppType =
                            std::is_same<T, float       >::value ? 0x1406 : //GL_FLOAT
                            std::is_same<T, glm::vec2   >::value ? 0x8B50 : //GL_FLOAT_VEC2
                            std::is_same<T, glm::vec3   >::value ? 0x8B51 : //GL_FLOAT_VEC3
                            std::is_same<T, glm::vec4   >::value ? 0x8B52 : //GL_FLOAT_VEC4
                            std::is_same<T, double      >::value ? 0x140A : //GL_DOUBLE
                            std::is_same<T, glm::dvec2  >::value ? 0x8FFC : //GL_DOUBLE_VEC2
                            std::is_same<T, glm::dvec3  >::value ? 0x8FFD : //GL_DOUBLE_VEC3
                            std::is_same<T, glm::dvec4  >::value ? 0x8FFE : //GL_DOUBLE_VEC4
                            std::is_same<T, int32_t     >::value ? 0x1404 : //GL_INT
                            std::is_same<T, glm::ivec2  >::value ? 0x8B53 : //GL_INT_VEC2
                            std::is_same<T, glm::ivec3  >::value ? 0x8B54 : //GL_INT_VEC3
                            std::is_same<T, glm::ivec4  >::value ? 0x8B55 : //GL_INT_VEC4
                            std::is_same<T, uint32_t    >::value ? 0x1405 : //GL_UNSIGNED_INT
                            std::is_same<T, glm::uvec2  >::value ? 0x8DC6 : //GL_UNSIGNED_INT_VEC2
                            std::is_same<T, glm::uvec3  >::value ? 0x8DC7 : //GL_UNSIGNED_INT_VEC3
                            std::is_same<T, glm::uvec4  >::value ? 0x8DC8 : //GL_UNSIGNED_INT_VEC4
                            std::is_same<T, bool        >::value ? 0x8B56 : //GL_BOOL
                            std::is_same<T, glm::bvec2  >::value ? 0x8B57 : //GL_BOOL_VEC2
                            std::is_same<T, glm::bvec3  >::value ? 0x8B58 : //GL_BOOL_VEC3
                            std::is_same<T, glm::bvec4  >::value ? 0x8B59 : //GL_BOOL_VEC4
                            std::is_same<T, glm::mat2x2 >::value ? 0x8B5A : //GL_FLOAT_MAT2
                            std::is_same<T, glm::mat2x3 >::value ? 0x8B65 : //GL_FLOAT_MAT2x3
                            std::is_same<T, glm::mat2x4 >::value ? 0x8B66 : //GL_FLOAT_MAT2x4
                            std::is_same<T, glm::mat3x2 >::value ? 0x8B67 : //GL_FLOAT_MAT3x2
                            std::is_same<T, glm::mat3x3 >::value ? 0x8B5B : //GL_FLOAT_MAT3
                            std::is_same<T, glm::mat3x4 >::value ? 0x8B68 : //GL_FLOAT_MAT3x4
                            std::is_same<T, glm::mat4x2 >::value ? 0x8B69 : //GL_FLOAT_MAT4x2
                            std::is_same<T, glm::mat4x3 >::value ? 0x8B6A : //GL_FLOAT_MAT4x3
                            std::is_same<T, glm::mat4x4 >::value ? 0x8B5C : //GL_FLOAT_MAT4
                            std::is_same<T, glm::dmat2x2>::value ? 0x8F46 : //GL_DOUBLE_MAT2
                            std::is_same<T, glm::dmat2x3>::value ? 0x8F49 : //GL_DOUBLE_MAT2x3
                            std::is_same<T, glm::dmat2x4>::value ? 0x8F4A : //GL_DOUBLE_MAT2x4
                            std::is_same<T, glm::dmat3x2>::value ? 0x8F4B : //GL_DOUBLE_MAT3x2
                            std::is_same<T, glm::dmat3x3>::value ? 0x8F47 : //GL_DOUBLE_MAT3
                            std::is_same<T, glm::dmat3x4>::value ? 0x8F4C : //GL_DOUBLE_MAT3x4
                            std::is_same<T, glm::dmat4x2>::value ? 0x8F4D : //GL_DOUBLE_MAT4x2
                            std::is_same<T, glm::dmat4x3>::value ? 0x8F4E : //GL_DOUBLE_MAT4x3
                            std::is_same<T, glm::dmat4x4>::value ? 0x8F48 : //GL_DOUBLE_MAT4
                            0;
                        static_assert(cppType != 0, "Not a valid template type for UniformSetter");

                        shaderId = pParent->id;
                        int32_t type;
                        for (auto& uniform : pParent->uniformList)
                            if (uniform.name == uniformName) {
                                location = uniform.location;
                                type     = uniform.type;
                                count    = uniform.arraySize;
                                stride   = uniform.arrayStride;
                                break;
                            }
                        if (location == -1) {
                            pParent->warning("UniformSetter did not find uniform with the name \"" + uniformName + "\"\n");
                        } else if (cppType != type) {
                            pParent->warning("UniformSetter<" + glTypeToCppName(cppType) + "> mismatches GLSL type. Expected UniformSetter<" + glTypeToCppName(type)  + "> for: uniform " + glTypeToGlslName(type) + " " + uniformName);
                        }
                    }
                    UniformSetter(PipelineInterface* const pParent, const std::string& uniformName, const T& initValue):
                        UniformSetter(pParent, uniformName)
                    {
                        setUniform(shaderId, location, initValue);
                    }
                    UniformSetter(PipelineInterface* const pParent, const std::string& uniformName, std::initializer_list<T> valueList):
                        UniformSetter(pParent, uniformName)
                    {
                        setUniform(shaderId, location, *valueList.begin(), std::min<int>(valueList.size(), count));
                    }
                    const T& operator=(const T& value) {
                        setUniform(shaderId, location, value);
                        return value;
                    }
                    std::initializer_list<T> operator=(std::initializer_list<T> valueList) {
                        setUniform(shaderId, location, *valueList.begin(), std::min<int>(valueList.size(), count));
                        return valueList;
                    }
                    UniformSetter<T> operator[](uint32_t i) {
                        if (i < count)
                            return UniformSetter<T>(shaderId, location + (stride * i), count - i, stride);
                        return UniformSetter<T>(shaderId, -1, 0, 0);
                    }
                private:
                    uint32_t shaderId;
                     int32_t location = -1;
                    uint16_t count    =  0; //uniforms and therefore locations should be limited to 64KiB?!
                    uint16_t stride   =  0;
                    constexpr UniformSetter(uint32_t shaderId, int32_t location, uint16_t count, uint16_t stride): shaderId(shaderId), location(location), count(count), stride(stride){}
            };

            //TODO: need setter for uniform structures
        protected:
            PipelineInterface() = default;
            ~PipelineInterface();
        protected:
            template<typename T>
            inline void setUniformByName(uint32_t shaderId, const std::string& uniformName, const T& value) {
                setUniform(shaderId, getUniformLocation(uniformName), value);
            }

            template<typename T>
            inline void setUniformByName(uint32_t shaderId, const std::string& uniformName, const T& value, int count) {
                setUniform(shaderId, getUniformLocation(uniformName), value, count);
            }

            int32_t getUniformLocation(const std::string &uniformName);

            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const float&        value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec2&    value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec3&    value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec4&    value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const int32_t&      value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec2&   value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec3&   value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec4&   value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const uint32_t&     value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec2&   value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec3&   value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec4&   value);

            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const float&        value, int count);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec2&    value, int count);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec3&    value, int count);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::vec4&    value, int count);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const int32_t&      value, int count);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec2&   value, int count);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec3&   value, int count);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::ivec4&   value, int count);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const uint32_t&     value, int count);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec2&   value, int count);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec3&   value, int count);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::uvec4&   value, int count);

            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat2x2&  value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat2x3&  value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat2x4&  value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat3x2&  value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat3x3&  value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat3x4&  value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat4x2&  value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat4x3&  value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::mat4x4&  value, int count = 1);

            //ARB_gpu_shader_fp64 (Core since 4.0)
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const double&       value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec2&   value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec3&   value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec4&   value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const double&       value, int count);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec2&   value, int count);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec3&   value, int count);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dvec4&   value, int count);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat2x2& value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat2x3& value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat2x4& value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat3x2& value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat3x3& value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat3x4& value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat4x2& value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat4x3& value, int count = 1);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const glm::dmat4x4& value, int count = 1);

            //part of GL_ARB_bindless_texture (Not part of Core)
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const uint64_t& value);
            static void setUniform(uint32_t shaderId, int32_t uniformLocation, const uint64_t& value, int count);

            uint32_t id = 0;
            std::string infoLog_;

            int32_t textureUniformLocation[config::MAX_SAMPLER_BINDINGS];

            std::string getShaderInfoLog(uint32_t objId);
            std::string getProgramInfoLog(uint32_t objId);
        protected:
            virtual std::string getPipelineIdentificationString() = 0;
            std::string getPipelineInformationQueryString();
            void warning(const std::string& message);
            void error  (const std::string& message);

            //Keep all the get uniform functions private? Only useful for initializing and debugging. Otherwise probably destroies performance like all GL server -> GL Client interactions.
            //TODO: -1 location = ?   if error, should we filter it out? Let get-variables be untouched or zero them?
            //All the array get functions came into OpenGL in a very late version!
            template<typename T>
            inline void getUniformByName(const std::string& uniformName, T& value, int count = 1) {
                getUniform(getUniformLocation(uniformName), value, count);
            }

            static void getUniform(uint32_t shaderId, int32_t uniformLocation, float&    value);
            static void getUniform(uint32_t shaderId, int32_t uniformLocation, int32_t&  value);

            static void getUniform(uint32_t shaderId, int32_t uniformLocation, uint32_t& value);

            static void getUniform(uint32_t shaderId, int32_t uniformLocation, float&    value, int count);
            static void getUniform(uint32_t shaderId, int32_t uniformLocation, int32_t&  value, int count);
            static void getUniform(uint32_t shaderId, int32_t uniformLocation, uint32_t& value, int count);

            //ARB_gpu_shader_fp64 (Core since 4.0)
            static void getUniform(uint32_t shaderId, int32_t uniformLocation, double&   value);
            static void getUniform(uint32_t shaderId, int32_t uniformLocation, double&   value, int count);

            /*void getUniform(GLint uniformLocation, GLfloat&      value, int count = 1);
            void getUniform(GLint uniformLocation, glm::vec2&    value, int count = 1);
            void getUniform(GLint uniformLocation, glm::vec3&    value, int count = 1);
            void getUniform(GLint uniformLocation, glm::vec4&    value, int count = 1);
            void getUniform(GLint uniformLocation, GLdouble&     value, int count = 1);
            void getUniform(GLint uniformLocation, glm::dvec2&   value, int count = 1);
            void getUniform(GLint uniformLocation, glm::dvec3&   value, int count = 1);
            void getUniform(GLint uniformLocation, glm::dvec4&   value, int count = 1);
            void getUniform(GLint uniformLocation, GLint&        value, int count = 1);
            void getUniform(GLint uniformLocation, glm::ivec2&   value, int count = 1);
            void getUniform(GLint uniformLocation, glm::ivec3&   value, int count = 1);
            void getUniform(GLint uniformLocation, glm::ivec4&   value, int count = 1);
            void getUniform(GLint uniformLocation, GLuint&       value, int count = 1);
            void getUniform(GLint uniformLocation, glm::uvec2&   value, int count = 1);
            void getUniform(GLint uniformLocation, glm::uvec3&   value, int count = 1);
            void getUniform(GLint uniformLocation, glm::uvec4&   value, int count = 1);
            void getUniform(GLint uniformLocation, glm::mat2x2&  value, int count = 1);
            void getUniform(GLint uniformLocation, glm::mat2x3&  value, int count = 1);
            void getUniform(GLint uniformLocation, glm::mat2x4&  value, int count = 1);
            void getUniform(GLint uniformLocation, glm::mat3x2&  value, int count = 1);
            void getUniform(GLint uniformLocation, glm::mat3x3&  value, int count = 1);
            void getUniform(GLint uniformLocation, glm::mat3x4&  value, int count = 1);
            void getUniform(GLint uniformLocation, glm::mat4x2&  value, int count = 1);
            void getUniform(GLint uniformLocation, glm::mat4x3&  value, int count = 1);
            void getUniform(GLint uniformLocation, glm::mat4x4&  value, int count = 1);
            void getUniform(GLint uniformLocation, glm::dmat2x2& value, int count = 1);
            void getUniform(GLint uniformLocation, glm::dmat2x3& value, int count = 1);
            void getUniform(GLint uniformLocation, glm::dmat2x4& value, int count = 1);
            void getUniform(GLint uniformLocation, glm::dmat3x2& value, int count = 1);
            void getUniform(GLint uniformLocation, glm::dmat3x3& value, int count = 1);
            void getUniform(GLint uniformLocation, glm::dmat3x4& value, int count = 1);
            void getUniform(GLint uniformLocation, glm::dmat4x2& value, int count = 1);
            void getUniform(GLint uniformLocation, glm::dmat4x3& value, int count = 1);
            void getUniform(GLint uniformLocation, glm::dmat4x4& value, int count = 1);*/

            //BUFFER ATTRIBUTE
            //attributeLayoutStates.uppermostActiveBufferIndex for highestActiveBinding
            void       buffer_attribute_markSlotChange(int32_t slot);
             int32_t   buffer_attribute_changedSlotMin = std::numeric_limits<decltype(buffer_attribute_changedSlotMin)>::max();
             int32_t   buffer_attribute_changedSlotMax = -1;

            //BUFFER UNIFORM
             int32_t   buffer_uniform_highestActiveBinding = -1;
            void       buffer_uniform_markSlotChange(int32_t slot);
             int32_t   buffer_uniform_changedSlotMin = std::numeric_limits<decltype(buffer_uniform_changedSlotMin)>::max();
             int32_t   buffer_uniform_changedSlotMax = -1;
            uint32_t*  buffer_uniform_id;
            uintptr_t* buffer_uniform_offset;
            uintptr_t* buffer_uniform_size;

            //BUFFER ATOMIC COUNTER
             int32_t   buffer_atomicCounter_highestActiveBinding = -1;
            void       buffer_atomicCounter_markSlotChange(int32_t slot);
             int32_t   buffer_atomicCounter_changedSlotMin = std::numeric_limits<decltype(buffer_atomicCounter_changedSlotMin)>::max();
             int32_t   buffer_atomicCounter_changedSlotMax = -1;
            uint32_t*  buffer_atomicCounter_id;
            uintptr_t* buffer_atomicCounter_offset;
            uintptr_t* buffer_atomicCounter_size;

            //BUFFER SHADER STORAGE
             int32_t   buffer_shaderStorage_highestActiveBinding = -1;
            void       buffer_shaderStorage_markSlotChange(int32_t slot);
             int32_t   buffer_shaderStorage_changedSlotMin = std::numeric_limits<decltype(buffer_shaderStorage_changedSlotMin)>::max();
             int32_t   buffer_shaderStorage_changedSlotMax = -1;
            uint32_t*  buffer_shaderStorage_id;
            uintptr_t* buffer_shaderStorage_offset;
            uintptr_t* buffer_shaderStorage_size;

            //TEXTURE
             int32_t   sampler_highestActiveBinding = -1;
            void       texture_markSlotChange(int32_t slot);
             int32_t   texture_changedSlotMin = std::numeric_limits<decltype(texture_changedSlotMin)>::max();
             int32_t   texture_changedSlotMax = -1;
            uint32_t*  texture_id;
             int32_t*  texture_target;

            //SAMPLER
            void       sampler_markSlotChange(int32_t slot);
             int32_t   sampler_changedSlotMin = std::numeric_limits<decltype(sampler_changedSlotMin)>::max();
             int32_t   sampler_changedSlotMax = -1;
            uint32_t*  sampler_id;

            //IMAGE
             int32_t   image_highestActiveBinding = -1;
            void       image_markSlotChange(int32_t slot);
             int32_t   image_changedSlotMin = std::numeric_limits<decltype(image_changedSlotMin)>::max();
             int32_t   image_changedSlotMax = -1;
            uint32_t*  image_id;
            uint32_t*  image_format;
            uint32_t*  image_mipmapLevel;
             int32_t*  image_layer;

            struct Attribute {
                std::string name;
                int32_t type        =  0;
                int32_t arraySize   =  0; //?
            };

            struct Uniform {
                std::string name;
                int32_t location    = -1;
                int32_t type        =  0; //sampler, image, base, vector or matrix type
                int32_t arraySize   =  0; //0 means no array, anything else is an array
                int32_t arrayStride =  0;
            };

            //sampler and image uniforms
            struct BindingUniform {
                std::string name;
                int32_t type = 0;
            };

            //UBO
            struct UniformBlockUniform {
                std::string name;
                int32_t type           = 0;
                int32_t arraySize      = 0;
                int32_t offset         = 0;
                int32_t arrayStride    = 0;
                int32_t matrixStride   = 0;
                bool    matrixRowMajor = 0; //OpenGL default = column-major matrix
            };
            struct UniformBlock {
                std::string name;
                //int32_t blockIndex = 0xFFFFFFFF; //GL_INVALID_INDEX
                int32_t binding;
                int32_t blockSize;
                std::vector<UniformBlockUniform> uniform;
            };

            //atomic counter
            struct AtomicCounterBindingUniform {
                std::string name;
                int32_t offset    = 0;
                int32_t arraySize = 0;
                //int32_t arrayStride = 4; //Think I do not need this, should always be 4
            };
            struct AtomicCounterBinding {
                uint32_t dataSize;
                std::vector<AtomicCounterBindingUniform> uniformList;
            };

            //SSBO
            struct StorageBlockVariable {
                std::string name;
                int32_t type;
                int32_t arraySize;
                int32_t offset;
                int32_t arrayStride;
                int32_t matrixStride;
                bool    matrixRowMajor;

                int32_t topLevelArraySize;
                int32_t topLevelArrayStride;
            };
            struct StorageBlock {
                std::string name;
                int32_t binding;
                std::vector<StorageBlockVariable> variable;
            };

            std::vector<Attribute>            attributeList;
            std::vector<Uniform>              uniformList; //only uniforms with location
            std::vector<BindingUniform>       samplerList;
            std::vector<BindingUniform>       imageList;
            std::vector<UniformBlock>         uniformBlockList;
            std::vector<AtomicCounterBinding> atomicCounterBindingList;
            std::vector<StorageBlock>         storageBlockList;

            void collectInformation();
            void allocateMemory();

            void processPendingChanges();
            void processPendingChangesPipeline();
            void processPendingChangesBuffersUniform();
            void processPendingChangesBuffersAtomicCounter();
            void processPendingChangesBuffersShaderStorage();
            void processPendingChangesTextures();
            void processPendingChangesSamplers();
            void processPendingChangesImages();

            static std::string glTypeToGlslName(int32_t type);
            static std::string glTypeToCppName(int32_t type);
    };
}
