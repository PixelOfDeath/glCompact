#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"
#include "glCompact/Sampler.hpp"
//#include "glCompact/ShaderInterface.hpp"

#include "glCompact/ToolsInternal.hpp"
#include "glCompact/GlTools.hpp"

    #include <iostream>
    #include <string>
#include <algorithm> //for min/max in msvc
//#include <exception> //std::runtime_error is not defined in my current GCC version (known bug: gcc (Ubuntu 4.8.2-19ubuntu1) 4.8.2)


/*
    GL_ARB_sampler_objects (Core since 3.3)

    GL_EXT_texture_filter_anisotropic (Not core because of stupid patents)
        Anisotropy filtering only exist as a extensions. But is basically always present
        GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT (Min. 2; This value is 16 on nearly all implementations)

    GL_ARB_texture_filter_anisotropic (Core since 4.6)
        GL_MAX_TEXTURE_MAX_ANISOTROPY (Minium support is 16)

    glSamplerParameterIiv and glSamplerParameterIuiv are only used to set raw integer values to TEXTURE_BORDER_COLOR! So we don't need them at all!


    GL_CLAMP                //clamp between edge and border. Clamed position is a mix of edge pixel and border color. That at last is the spec, but often not implmented this way! long depricated (Also OpenGL ES does not support border color?!)
    GL_REPEAT
    GL_CLAMP_TO_BORDER      //clamp to border. Clamed position has only border color. (I have no idea why this border color stuff is not depricated even in 4.6)
    GL_CLAMP_TO_EDGE
    GL_MIRRORED_REPEAT
    GL_MIRROR_CLAMP_TO_EDGE //ARB_texture_mirror_clamp_to_edge (Core since 4.4)


    max binding point is GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
*/

using namespace glCompact::gl;

namespace glCompact {
    Sampler::Sampler() {
        if (threadContextGroup->extensions.GL_ARB_direct_state_access) {
            threadContextGroup->functions.glCreateSamplers(1, &id);
        } else {
            threadContextGroup->functions.glGenSamplers(1, &id);
            //this needs to be bound once before OpenGL creates the sampler for real, and before we can use operations on it like setting parameters
            threadContextGroup->functions.glBindSampler(0, id);
            threadContext->sampler_id[0] = id;
            threadContext->sampler_markSlotChange(0);
        }
    }

    Sampler::~Sampler() {
        detachFromThreadContext();
        threadContextGroup->functions.glDeleteSamplers(1, &id);
    }

    /** \brief set magnification filter (default is MagnificationFilter::nearest)
     *
     */
    void Sampler::setMagnificationFilter(
        MagnificationFilter magnificationFilter
    ) {
        setParameter(GL_TEXTURE_MAG_FILTER, static_cast<int32_t>(magnificationFilter));
    }

    /** \brief set minification filter (default is MinificationFilter::nearestFromMipmap0)
     *
     */
    void Sampler::setMinificationFilter(
        MinificationFilter minificationFilter
    ) {
        setParameter(GL_TEXTURE_MIN_FILTER, static_cast<int32_t>(minificationFilter));
    }

    /** \brief Set the maximum anisotropy filtering (Default is 1.0f, meaning no anisotropy filtering is used)
     *
     */
    void Sampler::setMaxAnisotropy(
        float maxAnisotropy
    ) {
        //if (!threadContextGroup->extensions.GL_ARB_texture_filter_anisotropic && !threadContextGroup->extensions.GL_EXT_texture_filter_anisotropic)
        //    throw std::runtime_error("blabla...");

        //NOTE: GL_TEXTURE_MAX_ANISOTROPY_EXT is the same constant! So this works for both, the EXT and the ARB version!
        setParameter(GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy);
    }

    /** \brief sets the minimum level-of-detail parameter.
     * \details This floating-point value limits the selection of highest resolution mipmap (lowest mipmap level). The initial value is -1000.
     */
    void Sampler::setMinLod(
        float minLod
    ) {
        setParameter(GL_TEXTURE_MIN_LOD, minLod);
    }

    /** \brief Sets the maximum level-of-detail parameter
     * \details This floating-point value limits the selection of the lowest resolution mipmap (highest mipmap level). The initial value is 1000.
     */
    void Sampler::setMaxLod(
        float maxLod
    ) {
        setParameter(GL_TEXTURE_MAX_LOD, maxLod);
    }

    void Sampler::setLodBias(
        float lodBias
    ) {
        setParameter(GL_TEXTURE_LOD_BIAS, lodBias);
    }

    void Sampler::setWrapModeX(
        WrapMode wrapModeX
    ) {
        setParameter(GL_TEXTURE_WRAP_S, static_cast<int32_t>(wrapModeX));
    }

    void Sampler::setWrapModeY(
        WrapMode wrapModeY
    ) {
        setParameter(GL_TEXTURE_WRAP_T, static_cast<int32_t>(wrapModeY));
    }

    void Sampler::setWrapModeZ(
        WrapMode wrapModeZ
    ) {
        setParameter(GL_TEXTURE_WRAP_T, static_cast<int32_t>(wrapModeZ));
    }

    void Sampler::setWrapModeXY(
        WrapMode wrapModeXY
    ) {
        setWrapModeX(wrapModeXY);
        setWrapModeY(wrapModeXY);
    }

    void Sampler::setWrapModeXY(
        WrapMode wrapModeX,
        WrapMode wrapModeY
    ) {
        setWrapModeX(wrapModeX);
        setWrapModeY(wrapModeY);
    }

    void Sampler::setWrapModeXYZ(
        WrapMode wrapModeXYZ
    ) {
        setWrapModeX(wrapModeXYZ);
        setWrapModeY(wrapModeXYZ);
        setWrapModeZ(wrapModeXYZ);
    }

    void Sampler::setWrapModeXYZ(
        WrapMode wrapModeX,
        WrapMode wrapModeY,
        WrapMode wrapModeZ
    ) {
        setWrapModeX(wrapModeX);
        setWrapModeY(wrapModeY);
        setWrapModeZ(wrapModeZ);
    }

    /** \brief Sets depth compare mode to sample binary from a depth or stancil texture
     *
     */
    void Sampler::setDepthCompareMode(
        CompareOperator compareOperator
    ) {
        setParameter(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        setParameter(GL_TEXTURE_COMPARE_FUNC, static_cast<int32_t>(compareOperator));
    }

    void Sampler::setDepthCompareModeOff() {
        setParameter(GL_TEXTURE_COMPARE_MODE, GL_NONE);
    }

    void Sampler::setParameter(
        GLenum pname,
        float  value
    ) {
        threadContextGroup->functions.glSamplerParameterf(id, pname, value);
    }

    void Sampler::setParameter(
        GLenum  pname,
        int32_t value
    ) {
        threadContextGroup->functions.glSamplerParameteri(id, pname, value);
    }

    void Sampler::detachFromThreadContext() {
        //TODO: also unbind here? (Should be unbound automaticly from active VAO when deleting it)
        LOOPI(Config::MAX_SAMPLER_BINDINGS) {
            if (threadContext->sampler_id[i] == id) threadContext->sampler_id[i] = 0;
        }
    }
}
