#include "glCompact/PipelineCompute.hpp"
#include "glCompact/Context_.hpp"
#include "glCompact/threadContext_.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/threadContextGroup_.hpp"
#include "glCompact/config.hpp"

    #include "glCompact/ToolsInternal.hpp"
    #include "glCompact/GlTools.hpp"

#include <glm/glm.hpp>

#include <fstream>
#include <vector>

//#include <unistd.h>
    #include <iostream> //just for debug, to be removed

/** \class glCompact::PipelineCompute
 *  \brief Class to load, setup and dispatch compute shader
 *  \details Depends on GL_ARB_compute_shader (Core since 4.3)
 */

///\cond HIDDEN_FROM_DOXYGEN
/*
    GL_ARB_compute_shader (Core since 4.3)
    Compute shaders can be put together into one program and switched on/off seperatly. (That is more of an anti patern, probably not going to implement that)

    GetIntegeri v with target set to MAX_COMPUTE_WORK_GROUP_COUNT and index set to zero, one, or two, representing the X, Y, and Z dimensions
    respectively.

    After the program has been linked, the local work group
    size of the program may be queried by calling GetProgramiv with pname set to
    COMPUTE_WORK_GROUP_SIZE

    The maximum size of a local work group may be determined by calling Get-
    Integeri v with target set to MAX_COMPUTE_WORK_GROUP_SIZE and index set to
    0, 1, or 2 to retrieve the maximum work size in the X, Y and Z dimension, respec-
    tively.

    Furthermore, the maximum number of invocations in a single local work
    group (i.e., the product of the three dimensions) may be determined by calling
    GetIntegerv with pname set to MAX_COMPUTE_WORK_GROUP_INVOCATIONS


    Performance: On some GPUs switching between rasterizing and compute shaders can have performance costs. On others (AMD) it actually is best to mix them because they can run in parallel.
    And like HT/SMT can get more out of the hardware. Especially if one task is more memory bound and the other computation bound.


    glGetIntegeri_v()

    GL_MAX_COMPUTE_WORK_GROUP_COUNT       (Minimum support: 65535, 65535, 65535)
    GL_MAX_COMPUTE_WORK_GROUP_SIZE        (Minimum support:  1024,  1024,    64)
    GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS (Minimum support:  1024) <- this is workGroup x*y*z
    GL_MAX_COMPUTE_SHARED_MEMORY_SIZE     (Minimum support: 32 KiB)
*/
///\endcond

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    PipelineCompute::PipelineCompute(
        const std::string&          shaderString
    ) {
        loadString_(shaderString);
    }

    //TODO: one of the only instances where I depend on catch to work (blocks people form changing throw into a simple crash)
    PipelineCompute::PipelineCompute(
        const std::string&          path,
        const std::string&          fileName
    ):
        loadedFromFile(true)
    {
        //TODO: what about empty filename string?
        string fullFileName = (path + "/" + fileName);
        this->fileName = fullFileName;
        string shaderString;
        try {
            shaderString = fileToString(fullFileName);
        } catch (const std::exception& e) {
            throw std::runtime_error("Can not read compute shader file \"" + fullFileName + "\". " + e.what());
        }
        loadString_(shaderString);
    }

    void PipelineCompute::loadString_(
        const std::string& computeShaderString
    ) {
        GLuint shaderId = threadContextGroup_->functions.glCreateShader(GL_COMPUTE_SHADER);
        const char *pCString = computeShaderString.c_str();
        threadContextGroup_->functions.glShaderSource(shaderId, 1, reinterpret_cast<const GLchar**>(&pCString), NULL);
        threadContextGroup_->functions.glCompileShader(shaderId);
        GLint status;
        threadContextGroup_->functions.glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
        bool compileSuccessful = status == GL_TRUE;
        string shaderLog = getShaderInfoLog(shaderId);
        if (!shaderLog.empty())
            infoLog_ += "COMPUTE SHADER LOG:\n" + shaderLog + "\n";
        UNLIKELY_IF (!compileSuccessful) {
            if (shaderId) threadContextGroup_->functions.glDeleteShader(shaderId);
            throw std::runtime_error("Error loading shader:\n" + infoLog_);
        }

        id = threadContextGroup_->functions.glCreateProgram();
        //if (binaryRetrievableHint && threadContext->extensions.GL_ARB_get_program_binary)
        //    threadContextGroup_->functions.glProgramParameteri(id, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
        //else
        //    glProgramParameteri(id, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_FALSE);

        threadContextGroup_->functions.glAttachShader(id, shaderId);
        threadContextGroup_->functions.glLinkProgram(id);

        GLint linkStatus;
        threadContextGroup_->functions.glGetProgramiv(id, GL_LINK_STATUS, &linkStatus);
        string programLog = getProgramInfoLog(id);
        if (!programLog.empty())
            infoLog_ += "PROGRAM LINK STATUS:\n" + getProgramInfoLog(id) + "\n";

        threadContextGroup_->functions.glDetachShader(id, shaderId);
        threadContextGroup_->functions.glDeleteShader(shaderId);

        UNLIKELY_IF (!linkStatus) {
            threadContextGroup_->functions.glDeleteProgram(id);
            id = 0;
            throw std::runtime_error("Error linking shader:\n" + infoLog_);
        }

        collectInformation();
        allocateMemory();
    }

    void PipelineCompute::collectInformation() {
        PipelineInterface::collectInformation();
        threadContextGroup_->functions.glGetProgramiv(id, GL_COMPUTE_WORK_GROUP_SIZE, &workGroupSize[0]);
    }

    /** \brief dispatch compute shader
     *
     * \param x
     * \param y
     * \param z
     */
    void PipelineCompute::dispatch(
        uint32_t groupCountX,
        uint32_t groupCountY,
        uint32_t groupCountZ
    ) {
        UNLIKELY_IF (!threadContextGroup_->extensions.GL_ARB_compute_shader)
            throw std::runtime_error("missing support for GL_ARB_compute_shader (Core since 4.3)!");
        processPendingChanges();
        threadContextGroup_->functions.glDispatchCompute(groupCountX, groupCountY, groupCountZ);
    }

    /** \brief dispatch compute shader, getting group count parameters from a buffer
     *
     * <pre>
     * typedef struct DispatchIndirectCommand {
     *     uint32_t groupCountX;
     *     uint32_t groupCountY;
     *     uint32_t groupCountZ;
     * } DispatchIndirectCommand;
     * </pre>
     * \param buffer
     * \param offset offset to DispatchIndirectCommand structure in buffer
    */
    void PipelineCompute::dispatchIndirect(
        const BufferInterface& buffer,
        uintptr_t              offset
    ) {
        UNLIKELY_IF (!threadContextGroup_->extensions.GL_ARB_compute_shader)
            throw std::runtime_error("missing support for GL_ARB_compute_shader (Core since 4.3)!");
        UNLIKELY_IF (!buffer.id)
            throw std::runtime_error("does not take empty buffer!");
        processPendingChanges();
        threadContext_->cachedBindDispatchIndirectBuffer(buffer.id);
        threadContextGroup_->functions.glDispatchComputeIndirect(offset);
    }

    glm::ivec3 PipelineCompute::getMaxWorkGroupCount() {
        return threadContextGroup_->values.GL_MAX_COMPUTE_WORK_GROUP_COUNT;
    }

    glm::ivec3 PipelineCompute::getMaxWorkGroupSize() {
        return threadContextGroup_->values.GL_MAX_COMPUTE_WORK_GROUP_SIZE;
    }

    uint32_t PipelineCompute::getMaxWorkGroupInvocation() {
        return threadContextGroup_->values.GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS;
    }

    /*
        This returns a multi-line string that identificates/describes a shader/pipeline object for warning or error messages.
    */
    string PipelineCompute::getPipelineIdentificationString() {
        //typeid.name() does not return perfeclty identifiable class names, but close enough for now
        string s = string("")
            + "typeid.name: \"" + typeid(*this).name() + "\" based on glCompact::PipelineCompute\n";
        if (loadedFromFile) {
            s += " loaded from this file:\n";
            s += "  " + fileName + "\n";
        } else {
            s += " loaded from string\n";
            //TODO: Output whole shader here? No real other way to identificate and be clear about what caused error/warnings.
            //Could be an issue with spamming to much on screen/log. Or make full output a one-time event for the application runtime?
            //Also would need clear documentation for users that may not want to output glsl to the world.
        }
        return s;
    }

    string PipelineCompute::getPipelineInformationQueryString() {
        return(PipelineInterface::getPipelineInformationQueryString());
    }

    void PipelineCompute::processPendingChanges() {
        processPendingChangesPipeline();
        PipelineInterface::processPendingChanges();
    }

    void PipelineCompute::processPendingChangesPipeline() {
        threadContext_->cachedBindShader(id);
        if (threadContext_->pipeline != this) {
            PipelineInterface::processPendingChangesPipeline();
            threadContext_->pipeline = this;
        }
    }
}
