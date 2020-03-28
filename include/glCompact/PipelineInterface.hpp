#pragma once
#include "glCompact/BufferInterface.hpp"
#include "glCompact/TextureInterface.hpp"
#include "glCompact/TextureSelector.hpp"
#include "glCompact/SurfaceFormat.hpp"
#include "glCompact/Sampler.hpp"

#include <glm/fwd.hpp>

#include <string>
#include <vector>
#include <initializer_list>

namespace glCompact {
    class PipelineInterface {
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
            /*
                TODO:
                 - prevent from accessing glCompact managed uniforms via this class. E.g. texture sampler, so users can't change slot number
                 - prevent duplicated access to uniform with several of this classe?
                 - make a specialized getUniformLocation function that limits itself for that purpose?!
                 - warning if uniformName is not found? Only in debug mode? (OpenGL always removes unused uniforms from shaders)

                TODO: Instad of the class pointer this could just store the gl shader ID. So on 64 bit systems it would only be 32bit+32bit large instad of 64bit+32bit!

                TODO: Not sure how I could implement warning of uniforms that do NOT get associated UniformSetter, because UniformSetter constructors all run after loading of Pipeline class.
            */

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
                        shaderId             = pParent->id;
                        uniformLocation      = pParent->getUniformLocation(uniformName);
                        uniformLocationCount = pParent->getUniformArrayCount(uniformLocation);
                        uniformLocationStep  = pParent->getUniformArrayStep (uniformLocation);
                        if (uniformLocation == -1)
                            pParent->warning("UniformSetter did not find uniform with the name \"" + uniformName + "\"\n");
                    }
                    UniformSetter(PipelineInterface* const pParent, const std::string& uniformName, const T& initValue):
                        UniformSetter(pParent, uniformName)
                    {
                        setUniform(shaderId, uniformLocation, initValue);
                    }
                    UniformSetter(PipelineInterface* const pParent, const std::string& uniformName, std::initializer_list<T> list):
                        UniformSetter(pParent, uniformName)
                    {
                        int32_t count = std::min(uniformLocationCount, int32_t(list.size()));
                        setUniform(shaderId, uniformLocation, *list.begin(), count);
                    }
                    const T& operator=(const T& newValue) {
                        setUniform(shaderId, uniformLocation, newValue);
                        return newValue;
                    }
                    //This allows to transparently use all constructors from TInput (makes working with glm types way easier!)
                    //Keep the TInput to T constructor line sepperate! So any kind of compile time type conversation error is easiert to read!
                    template<typename TInput>
                    const TInput& operator=(const TInput& newValue) {
                        T newValueConverted(newValue);
                        setUniform(shaderId, uniformLocation, newValueConverted);
                        return newValue;
                    }
                    std::initializer_list<T> operator=(std::initializer_list<T> list) {
                        setUniform(shaderId, uniformLocation, *list.begin(), std::min<int32_t>(list.size(), uniformLocationCount));
                        return list;
                    }
                    UniformSetter<T> operator[](int32_t i) {
                        if (i < uniformLocationCount)
                            return UniformSetter<T>(shaderId, uniformLocation + (uniformLocationStep * i), uniformLocationCount - i, uniformLocationStep);
                        return UniformSetter<T>(shaderId, -1, 0, 0);
                    }
                private:
                    uint32_t shaderId;
                    int32_t  uniformLocation;
                    int32_t  uniformLocationCount;
                    int32_t  uniformLocationStep;
                    UniformSetter(uint32_t shaderId, int32_t uniformLocation, int32_t uniformLocationCount, int32_t uniformLocationStep):
                        shaderId(shaderId), uniformLocation(uniformLocation), uniformLocationCount(uniformLocationCount), uniformLocationStep(uniformLocationStep){}
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
            int32_t getUniformArrayCount(int32_t uniformLocation);
            int32_t getUniformArrayStep (int32_t uniformLocation);

            /*int32_t getUniformArraySize(const std::string &uniformName) {
                for (auto& uniform : uniformList)
                    if (uniform.name == uniformName) return uniform.arraySize ? uniform.arraySize : 1;
            }
            int32_t getUniformArrayStep(const std::string &uniformName) {
                for (auto& uniform : uniformList)
                    if (uniform.name == uniformName) {

                        std::string uniformName1 = uniformName + "[1]";
                        int32_t location1 = threadContextGroup_->functions.glGetUniformLocation(id, uniformName1.c_str());

                        return location1 - uniform.location;
                    }
                return 0;
            }*/

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

            int32_t textureUniformLocation[Config::MAX_SAMPLER_BINDINGS];

            void activate();
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

            //TEXTURE/SAMPLER
             int32_t  sampler_highestActiveBinding = -1;
            uint32_t  texture_id    [Config::MAX_SAMPLER_BINDINGS] = {};
             int32_t  texture_target[Config::MAX_SAMPLER_BINDINGS] = {};

            uint32_t  sampler_id    [Config::MAX_SAMPLER_BINDINGS] = {};

            //BUFFER UNIFORM
             int32_t  buffer_uniform_highestActiveBinding = - 1;
            uint32_t  buffer_uniform_id    [Config::MAX_UNIFORM_BUFFER_BINDINGS] = {};
            uintptr_t buffer_uniform_offset[Config::MAX_UNIFORM_BUFFER_BINDINGS] = {};
            uintptr_t buffer_uniform_size  [Config::MAX_UNIFORM_BUFFER_BINDINGS] = {};

            //IMAGE
            //void setImage_(int slot, const ImagesSelection imagesSelection, ImageFormat* imageFormat);
             int32_t  image_highestActiveBinding = -1;
            uint32_t  image_id         [Config::MAX_IMAGE_BINDINGS] = {};
            uint32_t  image_format     [Config::MAX_IMAGE_BINDINGS] = {};
            uint32_t  image_mipmapLevel[Config::MAX_IMAGE_BINDINGS] = {};
             int32_t  image_layer      [Config::MAX_IMAGE_BINDINGS] = {};

            //BUFFER ATOMIC COUNTER
             int32_t  buffer_atomicCounter_highestActiveBinding = -1;
            uint32_t  buffer_atomicCounter_id    [Config::MAX_UNIFORM_BUFFER_BINDINGS] = {};
            uintptr_t buffer_atomicCounter_offset[Config::MAX_UNIFORM_BUFFER_BINDINGS] = {};
            uintptr_t buffer_atomicCounter_size  [Config::MAX_UNIFORM_BUFFER_BINDINGS] = {};

            //BUFFER SHADER STORAGE
             int32_t  buffer_shaderStorage_highestActiveBinding = - 1;
            uint32_t  buffer_shaderStorage_id    [Config::MAX_SHADERSTORAGE_BUFFER_BINDINGS] = {};
            uintptr_t buffer_shaderStorage_offset[Config::MAX_SHADERSTORAGE_BUFFER_BINDINGS] = {};
            uintptr_t buffer_shaderStorage_size  [Config::MAX_SHADERSTORAGE_BUFFER_BINDINGS] = {};

            struct Uniform {
                std::string name;
                int32_t location  = -1;
                int32_t type      =  0; //sampler, image, base, vector or matrix type
                int32_t arraySize =  0; //0 means no array, anything else is an array
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

            std::vector<Uniform>              uniformList; //only uniforms with location
            std::vector<BindingUniform>       samplerList;
            std::vector<BindingUniform>       imageList;
            std::vector<UniformBlock>         uniformBlockList;
            std::vector<AtomicCounterBinding> atomicCounterBindingList;
            std::vector<StorageBlock>         storageBlockList;

            void collectInformation();

            void processPendingChanges();
            void processPendingChangesBuffersUniform();
            void processPendingChangesBuffersAtomicCounter();
            void processPendingChangesBuffersShaderstorage();
            void processPendingChangesTextures();
            void processPendingChangesSamplers();
            void processPendingChangesImages();
    };
}
