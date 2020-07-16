#pragma once
#include <glCompact/GlVersion.hpp>
#include <glCompact/GlesVersion.hpp>

namespace glCompact {
    namespace config {
        //If enabled this will use a thread_local pointer for the context state tracking information.
        //For thread local performance reasons this library should be linked statically or if used as a dynamic library there should be a static dependency to it.
        #define GLCOMPACT_MULTIPLE_CONTEXT

        //You want this to stay undefined in nerly all use cases! This is only useful if you want to create sepperate context groups
        //to access different devices/drivers from one application at the same time. Each with their own OpenGL version/features/extensions/function pointers/...
        //#define GLCOMPACT_MULTIPLE_CONTEXT_GROUP

        //glCompact requeres at last OpenGL 3.3 (or OpenGL ES 3.2 ?). Running glCompact with anything lower will break stuff!
        //The minimum version values can be set higher to enable constand folding for extensions and values that are always present with later versions.
        //FeatureSetting will behave like mustBeSupported for all features that are core since this OpenGL version!
        //While it is possible to enable both gl and gles support here, it will force to only garantie the lowest extensions/values from both selected versions!
        namespace version {
            constexpr GlVersion   glMin   = GlVersion::v33;            //glCompact requires this to be at last v33
            constexpr GlesVersion glesMin = GlesVersion::notSupported; //glCompact requires this to be at last v32 ?
        }

        enum class FeatureSetting {
            notSupported,
            mustBeSupported,
            runtimeDetection
        };

        //Core since 4.0
            constexpr FeatureSetting drawIndirect                 = FeatureSetting::notSupported; //GL_ARB_draw_indirect, core since 4.0
            constexpr FeatureSetting blendModePerDrawbuffer       = FeatureSetting::notSupported; //GL_ARB_draw_buffers_blend, core as non-ARB since 4.0

        //Core since 4.2
            constexpr FeatureSetting drawBaseInstance             = FeatureSetting::notSupported; //GL_ARB_base_instance, core since 4.2; GL_EXT_base_instance - ES extension! Not used in glCompact right now
            constexpr FeatureSetting bptc                         = FeatureSetting::notSupported; //GL_ARB_texture_compression_bptc, core since 4.2; texture formats
            constexpr FeatureSetting atomicCounter                = FeatureSetting::notSupported; //GL_ARB_shader_atomic_counters, core since 4.2

        //Core since 4.3
            constexpr FeatureSetting pipelineCompute              = FeatureSetting::notSupported; //GL_ARB_compute_shader, core since 4.3
            constexpr FeatureSetting shaderStorageBufferObject    = FeatureSetting::notSupported; //GL_ARB_shader_storage_buffer_object, core since 4.3
            constexpr FeatureSetting astc                         = FeatureSetting::notSupported; //GL_KHR_texture_compression_astc_hdr, core since 4.3; astc texture formats
            constexpr FeatureSetting textureView                  = FeatureSetting::notSupported; //GL_ARB_texture_view, core since 4.3

        //Core since 4.4
            constexpr FeatureSetting bufferStaging                = FeatureSetting::notSupported; //GL_ARB_buffer_storage, core since 4.4

        //Core since 4.6
            constexpr FeatureSetting drawIndirectCount            = FeatureSetting::notSupported; //GL_ARB_indirect_parameters, core as non-ARB since 4.6
            constexpr FeatureSetting polygonOffsetClamp           = FeatureSetting::notSupported; //GL_ARB_polygon_offset_clamp, core since 4.6; GL_EXT_polygon_offset_clamp
            constexpr FeatureSetting anisotropicFilter            = FeatureSetting::mustBeSupported; //GL_ARB_texture_filter_anisotropic, core since 4.6; GL_EXT_texture_filter_anisotropic (This is a ubiquitous extension, even tho it is not core, you can expect it to be present on all drivers not made out of wood)

        //not part of Core
            constexpr FeatureSetting spirv                        = FeatureSetting::notSupported; //GL_ARB_spirv_extensions (spirv >1.0); GL_ARB_gl_spirv (spirv 1.0)

        //DSA is mostly about making OpenGL less painful to use and not about performance!
        //So it makes sense to NOT always use them in glCompact, because uniform access is hidden behind the API anyway.
        //Intel recommends using the non DSA versions because it causes lower overheat for gl client->server communication.
        //Some of the more "modern" uniform getter functions only exist in DAS form. So this has no effect on them.
        //I leave this as an option to always be able to test how different drivers behave. But default will be always off.
        constexpr bool ENABLE_USE_OF_DSA_UNIFORM_FUNCTIONS = false;

        //Max. amount of vertex buffers/buffer index. Standard: 16/32 - inbuild variables (e.g. r290 limited to 0..28=29 locations!)
        //I guess default variables like gl_position takes take away from 32 locations?
        //vec3/vec4/mat3x?/mat4x? double types can (but not must) use up two times the amount of locations!
        constexpr int MAX_ATTRIBUTES                     = 32;
        constexpr int MAX_RGBA_ATTACHMENTS               = 8; //basically MAX_COLOR_ATTACHMENTS, but prefer to use the RGBA naming cheme

        namespace Workarounds {
            //r290 win10 blob (adrenalin-edition-18.6.1-june13) driver bug! (e.g. Mesa works fine!)
            //If we delete the current bound draw FBO it does not set to the default FBO (0) - this is not standard conform and breaks my state tracking
            //so we need to set currentId to something invalide e.g. -1 to force a glBindFramebuffer call in all cases!
            //TODO: in the future maybe only enably this if needed
            constexpr bool AMD_DELETING_ACTIVE_FBO_NOT_SETTING_DEFAULT_FBO = true;

            //Bug in Mesa 19.2.8 glBindBuffersRange does not like the size value to be 0, even when the buffer ID is 0!
            constexpr bool MESA_BIND_BUFFER_RANGE_NULL_ERROR_WHEN_SIZE_IS_NULL = true;
        }

        //to make debuging easy
        constexpr bool PRINT_PIPELINE_INFO_AFTER_LOAD = 0;

        //The OpenGL Standard defines that function without an active context on the calling thread have no effect.
        //This enables an assert check to catch this in glCompact functions.
        #define GLCOMPACT_DEBUG_ASSERT_THREAD_HAS_ACTIVE_CONTEXT
    }
}
