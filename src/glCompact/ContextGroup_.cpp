#include "glCompact/ContextGroup_.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/gl/ConstantsCustom.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/ToolsInternal.hpp"

#include <string>
#include <regex>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    template<>
    int32_t ContextGroup_::getValue(GLint pname) {
        int32_t ret = 0;
        functions.glGetIntegerv(pname, &ret);
        return ret;
    }

    template<>
    int32_t ContextGroup_::getValue(GLint pname, uint32_t index) {
        int32_t ret = 0;
        functions.glGetIntegeri_v(pname, index, &ret);
        return ret;
    }

    template<>
    uint32_t ContextGroup_::getValue(GLint pname) {
        uint32_t ret = 0;
        functions.glGetIntegerv(pname, reinterpret_cast<int32_t*>(&ret));
        return ret;
    }

    template<>
    uint32_t ContextGroup_::getValue(GLint pname, uint32_t index) {
        uint32_t ret = 0;
        functions.glGetIntegeri_v(pname, index, reinterpret_cast<int32_t*>(&ret));
        return ret;
    }

    template<>
    int64_t ContextGroup_::getValue(GLint pname) {
        int64_t ret = 0;
        functions.glGetInteger64v(pname, &ret);
        return ret;
    }

    template<>
    int64_t ContextGroup_::getValue(GLint pname, uint32_t index) {
        int64_t ret = 0;
        functions.glGetInteger64i_v(pname, index, &ret);
        return ret;
    }

    template<>
    uint64_t ContextGroup_::getValue(GLint pname) {
        uint64_t ret = 0;
        functions.glGetInteger64v(pname, reinterpret_cast<int64_t*>(&ret));
        return ret;
    }

    template<>
    uint64_t ContextGroup_::getValue(GLint pname, uint32_t index) {
        uint64_t ret = 0;
        functions.glGetInteger64i_v(pname, index, reinterpret_cast<int64_t*>(&ret));
        return ret;
    }

    template<>
    float ContextGroup_::getValue(GLint pname) {
        float ret = 0;
        functions.glGetFloatv(pname, &ret);
        return ret;
    }

    template<>
    float ContextGroup_::getValue(GLint pname, uint32_t index) {
        float ret = 0;
        functions.glGetFloati_v(pname, index, &ret);
        return ret;
    }

    template<>
    double ContextGroup_::getValue(GLint pname) {
        double ret = 0;
        functions.glGetDoublev(pname, &ret);
        return ret;
    }

    template<>
    double ContextGroup_::getValue(GLint pname, uint32_t index) {
        double ret = 0;
        functions.glGetDoublei_v(pname, index, &ret);
        return ret;
    }

    ContextGroup_::ContextGroup_(
        void*(*getGlFunctionPointer)(const char* glFunctionName)
    ) {
        functions.init(getGlFunctionPointer);

        const char* versionStringPtr = reinterpret_cast<const char*>(functions.glGetString(GL_VERSION));
        if (!versionStringPtr)
            crash("glGetString(GL_VERSION) returned 0! No active OpenGL context in this thread? Can not initalize without one!");
        version.string = string(versionStringPtr);

        //On Mesa:
        //GL   version string: "X.X ..."
        //GLES version string: "OpenGL ES X.X ..."
        smatch match;
        if (!regex_match(version.string, match, regex("^(OpenGL ES |)([0-9]).([0-9]) .*")))
            crash("Failed to parse version from glGetString(GL_VERSION): " + version.string);
        version.es    = (match[1] == "OpenGL ES ") ? 1 : 0;
        version.major = stoi(match[2]);
        version.minor = stoi(match[3]);
        if (version.major >  config::glMinMajor
        || (version.major == config::glMinMajor && version.minor >= config::glMinMinor)) {
            //all okey dokey!
        } else {
            crash("glCompact needs at last OpenGL " + to_string(config::glMinMajor) + "." + to_string(config::glMinMinor) + "!");
        }

        int32_t contextProfileMask = getValue<int32_t>(GL_CONTEXT_PROFILE_MASK);
        version.core              = contextProfileMask & GL_CONTEXT_CORE_PROFILE_BIT;
        version.forwardCompatible = contextProfileMask & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT;
        version.debug             = contextProfileMask & GL_CONTEXT_FLAG_DEBUG_BIT;
        version.robustAccess      = contextProfileMask & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT;
        version.noErrorReporting  = contextProfileMask & GL_CONTEXT_FLAG_NO_ERROR_BIT;

        extensions.init(this);
        setAllCoreExtensionTrue();
        getAllValue();
        checkAndSetFeatures();

        //DISABELING EXTENSIONS TO TEST DIFFERENT PATHS
        //*const_cast<bool*>(&extensions.GL_ARB_texture_storage)       = false; //NOTE: MAY CRASH IF TEXTURE VIEWS ARE USED! (Because they only can be created from texStorage objects!)
        //*const_cast<bool*>(&extensions.GL_ARB_multi_bind)            = false;
        //*const_cast<bool*>(&extensions.GL_ARB_vertex_attrib_binding) = false;

        //*const_cast<bool*>(&extensions.GL_ARB_direct_state_access)   = false;
        //*const_cast<bool*>(&extensions.GL_EXT_direct_state_access)   = false;

        //*const_cast<bool*>(&extensions.GL_ARB_ES3_compatibility)     = false;
    }

    /*
        There are extensions in OpenGL that behaves exactly the same as the later implemented core functionality.
        Somebody hat the ingenious idea to not list extensions in the supported list if the OpenGL version running integrates them as core functionality.
        This forces one to always check the extension list AND if the OpenGL version is => where the extension became part of the core standard.

        This function simply sets all extensions to true if the OpenGL version is met where they became part of core.

        Only checking the extension now is enough!
    */
    void ContextGroup_::setAllCoreExtensionTrue() {
        if (version.equalOrGreater(1, 2)) {
            extensions.GL_EXT_texture3D                         = true;
            //extensions.GL_OES_texture_3D = true;
        }
        if (version.equalOrGreater(1, 3)) {
            extensions.GL_ARB_multitexture                      = true;
            extensions.GL_ARB_transpose_matrix                  = true;
            extensions.GL_ARB_multisample                       = true;
            extensions.GL_ARB_texture_env_add                   = true;
            extensions.GL_ARB_texture_cube_map                  = true;
            extensions.GL_ARB_texture_compression               = true;
            extensions.GL_ARB_texture_border_clamp              = true;
            extensions.GL_ARB_texture_env_combine               = true;
            extensions.GL_ARB_texture_env_dot3                  = true;
        }
        if (version.equalOrGreater(1, 4)) {
            extensions.GL_ARB_point_parameters                  = true;
            extensions.GL_ARB_texture_env_crossbar              = true;
            extensions.GL_ARB_texture_mirrored_repeat           = true;
            extensions.GL_ARB_depth_texture                     = true;
            extensions.GL_ARB_shadow                            = true;
            extensions.GL_ARB_window_pos                        = true;
        }
        if (version.equalOrGreater(1, 5)) {
            extensions.GL_ARB_vertex_buffer_object              = true;
            extensions.GL_ARB_occlusion_query                   = true;
        }
        if (version.equalOrGreater(2, 0)) {
            extensions.GL_ARB_shader_objects                    = true;
            extensions.GL_ARB_vertex_shader                     = true;
            extensions.GL_ARB_fragment_shader                   = true;
            extensions.GL_ARB_shading_language_100              = true;
            extensions.GL_ARB_texture_non_power_of_two          = true;
            extensions.GL_ARB_point_sprite                      = true;
            extensions.GL_ARB_draw_buffers                      = true;
        }
        if (version.equalOrGreater(2, 1)) {
            extensions.GL_ARB_pixel_buffer_object               = true;
        }
        if (version.equalOrGreater(3, 0)) {
            extensions.GL_ARB_color_buffer_float                = true;
            extensions.GL_ARB_half_float_pixel                  = true;
            extensions.GL_ARB_texture_float                     = true;
            extensions.GL_ARB_depth_buffer_float                = true;
            extensions.GL_ARB_framebuffer_object                = true;
            extensions.GL_ARB_framebuffer_sRGB                  = true;
            extensions.GL_ARB_half_float_vertex                 = true;
            extensions.GL_ARB_map_buffer_range                  = true;
            extensions.GL_ARB_texture_compression_rgtc          = true;
            extensions.GL_ARB_texture_rg                        = true;
            extensions.GL_ARB_vertex_array_object               = true;
            extensions.GL_ARB_shader_texture_lod                = true;
            extensions.GL_EXT_texture_array                     = true;
        }
        if (version.equalOrGreater(3, 1)) {
            extensions.GL_ARB_draw_instanced                    = true;
            extensions.GL_ARB_texture_buffer_object             = true;
            extensions.GL_ARB_uniform_buffer_object             = true;
            extensions.GL_ARB_copy_buffer                       = true;
            extensions.GL_ARB_texture_rectangle                 = true;
        }
        if (version.equalOrGreater(3, 2)) {
            extensions.GL_ARB_geometry_shader4                  = true;
            extensions.GL_ARB_instanced_arrays                  = true;
            extensions.GL_ARB_depth_clamp                       = true;
            extensions.GL_ARB_draw_elements_base_vertex         = true;
            extensions.GL_ARB_fragment_coord_conventions        = true;
            extensions.GL_ARB_provoking_vertex                  = true;
            extensions.GL_ARB_seamless_cube_map                 = true;
            extensions.GL_ARB_sync                              = true;
            extensions.GL_ARB_texture_multisample               = true;
            extensions.GL_ARB_vertex_array_bgra                 = true;
        }
        if (version.equalOrGreater(3, 3)) {
            extensions.GL_ARB_blend_func_extended               = true;
            extensions.GL_ARB_explicit_attrib_location          = true;
            extensions.GL_ARB_occlusion_query2                  = true;
            extensions.GL_ARB_sampler_objects                   = true;
            extensions.GL_ARB_shader_bit_encoding               = true;
            extensions.GL_ARB_texture_rgb10_a2ui                = true;
            extensions.GL_ARB_texture_swizzle                   = true;
            extensions.GL_ARB_timer_query                       = true;
            extensions.GL_ARB_vertex_type_2_10_10_10_rev        = true;
        }
        if (version.equalOrGreater(4, 0)) {
            extensions.GL_ARB_draw_buffers_blend                = true;
            extensions.GL_ARB_sample_shading                    = true;
            extensions.GL_ARB_texture_cube_map_array            = true; //uses ARB suffix, but only brings new const. And the Core constants have the same values.
            extensions.GL_ARB_texture_gather                    = true;
            extensions.GL_ARB_draw_indirect                     = true;
            extensions.GL_ARB_gpu_shader5                       = true;
            extensions.GL_ARB_gpu_shader_fp64                   = true;
            extensions.GL_ARB_shader_subroutine                 = true;
            extensions.GL_ARB_tessellation_shader               = true;
            extensions.GL_ARB_texture_buffer_object_rgb32       = true;
            extensions.GL_ARB_transform_feedback2               = true;
            extensions.GL_ARB_transform_feedback3               = true;
        }
        if (version.equalOrGreater(4, 1)) {
            extensions.GL_ARB_ES2_compatibility                 = true;
            extensions.GL_ARB_get_program_binary                = true;
            extensions.GL_ARB_separate_shader_objects           = true;
            extensions.GL_ARB_shader_precision                  = true;
            extensions.GL_ARB_vertex_attrib_64bit               = true;
            extensions.GL_ARB_viewport_array                    = true;
        }
        if (version.equalOrGreater(4, 2)) {
            extensions.GL_ARB_base_instance                     = true;
            extensions.GL_ARB_shading_language_420pack          = true;
            extensions.GL_ARB_transform_feedback_instanced      = true;
            extensions.GL_ARB_compressed_texture_pixel_storage  = true;
            extensions.GL_ARB_conservative_depth                = true;
            extensions.GL_ARB_internalformat_query              = true;
            extensions.GL_ARB_map_buffer_alignment              = true;
            extensions.GL_ARB_shader_atomic_counters            = true;
            extensions.GL_ARB_shader_image_load_store           = true;
            extensions.GL_ARB_shading_language_packing          = true;
            extensions.GL_ARB_texture_storage                   = true;
            extensions.GL_ARB_texture_compression_bptc          = true;
        }
        if (version.equalOrGreater(4, 3)) {
            extensions.GL_KHR_texture_compression_astc_ldr      = true;
            extensions.GL_ARB_arrays_of_arrays                  = true;
            extensions.GL_ARB_clear_buffer_object               = true;
            extensions.GL_ARB_compute_shader                    = true;
            extensions.GL_ARB_copy_image                        = true;
            extensions.GL_ARB_texture_view                      = true;
            extensions.GL_ARB_vertex_attrib_binding             = true;
            extensions.GL_ARB_robustness_isolation              = true;
            extensions.GL_ARB_ES3_compatibility                 = true;
            extensions.GL_ARB_explicit_uniform_location         = true;
            extensions.GL_ARB_fragment_layer_viewport           = true;
            extensions.GL_ARB_framebuffer_no_attachments        = true;
            extensions.GL_ARB_internalformat_query2             = true;
            extensions.GL_ARB_invalidate_subdata                = true;
            extensions.GL_ARB_multi_draw_indirect               = true;
            extensions.GL_ARB_program_interface_query           = true;
            extensions.GL_ARB_robust_buffer_access_behavior     = true;
            extensions.GL_ARB_shader_image_size                 = true;
            extensions.GL_ARB_shader_storage_buffer_object      = true;
            extensions.GL_ARB_stencil_texturing                 = true;
            extensions.GL_ARB_texture_buffer_range              = true;
            extensions.GL_ARB_texture_query_levels              = true;
            extensions.GL_ARB_texture_storage_multisample       = true;
            extensions.GL_KHR_debug                             = true;
        }
        if (version.equalOrGreater(4, 4)) {
            extensions.GL_ARB_buffer_storage                    = true;
            extensions.GL_ARB_clear_texture                     = true;
            extensions.GL_ARB_enhanced_layouts                  = true;
            extensions.GL_ARB_multi_bind                        = true;
            extensions.GL_ARB_query_buffer_object               = true;
            extensions.GL_ARB_texture_mirror_clamp_to_edge      = true;
            extensions.GL_ARB_texture_stencil8                  = true;
            extensions.GL_ARB_vertex_type_10f_11f_11f_rev       = true;
        }
        if (version.equalOrGreater(4, 5)) {
            extensions.GL_ARB_ES3_1_compatibility               = true;
            extensions.GL_ARB_clip_control                      = true;
            extensions.GL_ARB_conditional_render_inverted       = true;
            extensions.GL_ARB_cull_distance                     = true;
            extensions.GL_ARB_direct_state_access               = true;
            extensions.GL_ARB_get_texture_sub_image             = true;
            extensions.GL_ARB_texture_barrier                   = true;
            extensions.GL_KHR_context_flush_control             = true;
            extensions.GL_ARB_derivative_control                = true;
            extensions.GL_ARB_shader_texture_image_samples      = true;
            extensions.GL_KHR_robustness                        = true;
        }
        if (version.equalOrGreater(4, 6)) {
            extensions.GL_ARB_indirect_parameters               = true;
            extensions.GL_ARB_pipeline_statistics_query         = true;
            extensions.GL_ARB_polygon_offset_clamp              = true;
            extensions.GL_KHR_no_error                          = true;
            extensions.GL_ARB_shader_atomic_counter_ops         = true; //GLSL Only
            extensions.GL_ARB_shader_draw_parameters            = true;
            extensions.GL_ARB_shader_group_vote                 = true; //GLSL Only
            extensions.GL_ARB_gl_spirv                          = true;
            extensions.GL_ARB_spirv_extensions                  = true;
            extensions.GL_ARB_texture_filter_anisotropic        = true;
            extensions.GL_ARB_transform_feedback_overflow_query = true;
        }
    }

    void ContextGroup_::getAllValue() {
        //Core forever?
            values.GL_MAX_TEXTURE_SIZE                                  = getValue<int32_t>(GL_MAX_TEXTURE_SIZE);

        //Core since 1.2
            if (extensions.GL_EXT_texture3D) {
                values.GL_MAX_3D_TEXTURE_SIZE                           = getValue<int32_t>(GL_MAX_3D_TEXTURE_SIZE);
            }
        //Core since 1.3
            //GL_ARB_texture_cube_map
            values.GL_MAX_CUBE_MAP_TEXTURE_SIZE                         = getValue<int32_t>(GL_MAX_CUBE_MAP_TEXTURE_SIZE);

        //Core since 2.0
            if (version.equalOrGreater(2, 0)) {
                values.GL_MAX_DRAW_BUFFERS                              = getValue<int32_t>(GL_MAX_DRAW_BUFFERS);
            }
            if (extensions.GL_ARB_vertex_shader) {
                values.GL_MAX_VERTEX_UNIFORM_COMPONENTS                 = getValue<int32_t>(GL_MAX_VERTEX_UNIFORM_COMPONENTS);
                values.GL_MAX_VARYING_FLOATS                            = getValue<int32_t>(GL_MAX_VARYING_FLOATS);
                values.GL_MAX_VERTEX_ATTRIBS                            = getValue<int32_t>(GL_MAX_VERTEX_ATTRIBS);
                values.GL_MAX_TEXTURE_IMAGE_UNITS                       = getValue<int32_t>(GL_MAX_TEXTURE_IMAGE_UNITS);
                values.GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS                = getValue<int32_t>(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
                values.GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS              = getValue<int32_t>(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
                values.GL_MAX_TEXTURE_COORDS                            = getValue<int32_t>(GL_MAX_TEXTURE_COORDS);
            }

        //Core since 3.0
            if (extensions.GL_ARB_framebuffer_object) {
                values.GL_MAX_SAMPLES                                   = getValue<int32_t>(GL_MAX_SAMPLES);
                values.GL_MAX_COLOR_ATTACHMENTS                         = getValue<int32_t>(GL_MAX_COLOR_ATTACHMENTS);
                values.GL_MAX_RENDERBUFFER_SIZE                         = getValue<int32_t>(GL_MAX_RENDERBUFFER_SIZE);
            }
            if (extensions.GL_EXT_texture_array) {
                values.GL_MAX_ARRAY_TEXTURE_LAYERS                      = getValue<int32_t>(GL_MAX_ARRAY_TEXTURE_LAYERS);
            }
        //Core since 3.1
            if (extensions.GL_ARB_texture_buffer_object) {
                values.GL_MAX_TEXTURE_BUFFER_SIZE                       = getValue<int32_t>(GL_MAX_TEXTURE_BUFFER_SIZE);
            }
            if (extensions.GL_ARB_uniform_buffer_object) {
                values.GL_MAX_VERTEX_UNIFORM_BLOCKS                     = getValue<int32_t>(GL_MAX_VERTEX_UNIFORM_BLOCKS);
                values.GL_MAX_GEOMETRY_UNIFORM_BLOCKS                   = getValue<int32_t>(GL_MAX_GEOMETRY_UNIFORM_BLOCKS);
                values.GL_MAX_FRAGMENT_UNIFORM_BLOCKS                   = getValue<int32_t>(GL_MAX_FRAGMENT_UNIFORM_BLOCKS);
                values.GL_MAX_COMBINED_UNIFORM_BLOCKS                   = getValue<int32_t>(GL_MAX_COMBINED_UNIFORM_BLOCKS);
                values.GL_MAX_UNIFORM_BUFFER_BINDINGS                   = getValue<int32_t>(GL_MAX_UNIFORM_BUFFER_BINDINGS);
                values.GL_MAX_UNIFORM_BLOCK_SIZE                        = getValue<int32_t>(GL_MAX_UNIFORM_BLOCK_SIZE);
                values.GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS        = getValue<int32_t>(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS);
                values.GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS      = getValue<int32_t>(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS);
                values.GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS      = getValue<int32_t>(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS);
                values.GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT               = getValue<int32_t>(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT);
            }
        //Core since 3.2
            if (extensions.GL_ARB_texture_multisample) {
                values.GL_MAX_COLOR_TEXTURE_SAMPLES                     = getValue<int32_t>(GL_MAX_COLOR_TEXTURE_SAMPLES);
                values.GL_MAX_DEPTH_TEXTURE_SAMPLES                     = getValue<int32_t>(GL_MAX_DEPTH_TEXTURE_SAMPLES);
                values.GL_MAX_INTEGER_SAMPLES                           = getValue<int32_t>(GL_MAX_INTEGER_SAMPLES);
            }
            if (extensions.GL_ARB_sync) {
                values.GL_MAX_SERVER_WAIT_TIMEOUT                       = getValue<int32_t>(GL_MAX_SERVER_WAIT_TIMEOUT);
            }
        //Core since 3.3
            if (extensions.GL_ARB_blend_func_extended) {
                values.GL_MAX_DUAL_SOURCE_DRAW_BUFFERS                  = getValue<int32_t>(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS);
            }
        //Core since 4.0
            if (extensions.GL_ARB_transform_feedback3) {
                values.GL_MAX_TRANSFORM_FEEDBACK_BUFFERS                = getValue<int32_t>(GL_MAX_TRANSFORM_FEEDBACK_BUFFERS);
                values.GL_MAX_VERTEX_STREAMS                            = getValue<int32_t>(GL_MAX_VERTEX_STREAMS);
            }
        //Core since 4.2
            if (extensions.GL_ARB_shader_atomic_counters) {
                values.GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS             = getValue<int32_t>(GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS);
                values.GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS       = getValue<int32_t>(GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS);
                values.GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS    = getValue<int32_t>(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS);
                values.GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS           = getValue<int32_t>(GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS);
                values.GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS           = getValue<int32_t>(GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS);
                values.GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS           = getValue<int32_t>(GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS);
                values.GL_MAX_VERTEX_ATOMIC_COUNTERS                    = getValue<int32_t>(GL_MAX_VERTEX_ATOMIC_COUNTERS);
                values.GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS              = getValue<int32_t>(GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS);
                values.GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS           = getValue<int32_t>(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS);
                values.GL_MAX_GEOMETRY_ATOMIC_COUNTERS                  = getValue<int32_t>(GL_MAX_GEOMETRY_ATOMIC_COUNTERS);
                values.GL_MAX_FRAGMENT_ATOMIC_COUNTERS                  = getValue<int32_t>(GL_MAX_FRAGMENT_ATOMIC_COUNTERS);
                values.GL_MAX_COMBINED_ATOMIC_COUNTERS                  = getValue<int32_t>(GL_MAX_COMBINED_ATOMIC_COUNTERS);
                values.GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE                = getValue<int32_t>(GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE);
                values.GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS            = getValue<int32_t>(GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS);
            }
            if (extensions.GL_ARB_shader_image_load_store) {
                values.GL_MAX_IMAGE_UNITS                               = getValue<int32_t>(GL_MAX_IMAGE_UNITS);
                values.GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS = getValue<int32_t>(GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS);
                values.GL_MAX_IMAGE_SAMPLES                             = getValue<int32_t>(GL_MAX_IMAGE_SAMPLES);
                values.GL_MAX_VERTEX_IMAGE_UNIFORMS                     = getValue<int32_t>(GL_MAX_VERTEX_IMAGE_UNIFORMS);
                values.GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS               = getValue<int32_t>(GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS);
                values.GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS            = getValue<int32_t>(GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS);
                values.GL_MAX_GEOMETRY_IMAGE_UNIFORMS                   = getValue<int32_t>(GL_MAX_GEOMETRY_IMAGE_UNIFORMS);
                values.GL_MAX_FRAGMENT_IMAGE_UNIFORMS                   = getValue<int32_t>(GL_MAX_FRAGMENT_IMAGE_UNIFORMS);
                values.GL_MAX_COMBINED_IMAGE_UNIFORMS                   = getValue<int32_t>(GL_MAX_COMBINED_IMAGE_UNIFORMS);
            }
        //Core since 4.3
            if (extensions.GL_ARB_vertex_attrib_binding) {
                values.GL_MAX_VERTEX_ATTRIB_BINDINGS                    = getValue<int32_t>(GL_MAX_VERTEX_ATTRIB_BINDINGS);
                values.GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET             = getValue<int32_t>(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET);
            }
            if (extensions.GL_ARB_texture_buffer_range) {
                values.GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT               = getValue<int32_t>(GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT);
            }
            if (extensions.GL_ARB_compute_shader) {
                values.GL_MAX_COMPUTE_UNIFORM_BLOCKS                    = getValue<int32_t>(GL_MAX_COMPUTE_UNIFORM_BLOCKS);
                values.GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS               = getValue<int32_t>(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS);
                values.GL_MAX_COMPUTE_IMAGE_UNIFORMS                    = getValue<int32_t>(GL_MAX_COMPUTE_IMAGE_UNIFORMS);
                values.GL_MAX_COMPUTE_SHARED_MEMORY_SIZE                = getValue<int32_t>(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE);
                values.GL_MAX_COMPUTE_UNIFORM_COMPONENTS                = getValue<int32_t>(GL_MAX_COMPUTE_UNIFORM_COMPONENTS);
                values.GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS            = getValue<int32_t>(GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS);
                values.GL_MAX_COMPUTE_ATOMIC_COUNTERS                   = getValue<int32_t>(GL_MAX_COMPUTE_ATOMIC_COUNTERS);
                values.GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS       = getValue<int32_t>(GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS);
                values.GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS            = getValue<int32_t>(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS);
                values.GL_MAX_COMPUTE_WORK_GROUP_COUNT[0]               = getValue<uint32_t>(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0);
                values.GL_MAX_COMPUTE_WORK_GROUP_COUNT[1]               = getValue<uint32_t>(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1);
                values.GL_MAX_COMPUTE_WORK_GROUP_COUNT[2]               = getValue<uint32_t>(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2);
                values.GL_MAX_COMPUTE_WORK_GROUP_SIZE[0]                = getValue<uint32_t>(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0);
                values.GL_MAX_COMPUTE_WORK_GROUP_SIZE[1]                = getValue<uint32_t>(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1);
                values.GL_MAX_COMPUTE_WORK_GROUP_SIZE[2]                = getValue<uint32_t>(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2);
            }
            if (extensions.GL_ARB_framebuffer_no_attachments) {
                values.GL_MAX_FRAMEBUFFER_WIDTH                         = getValue<int32_t>(GL_MAX_FRAMEBUFFER_WIDTH);
                values.GL_MAX_FRAMEBUFFER_HEIGHT                        = getValue<int32_t>(GL_MAX_FRAMEBUFFER_HEIGHT);
                values.GL_MAX_FRAMEBUFFER_LAYERS                        = getValue<int32_t>(GL_MAX_FRAMEBUFFER_LAYERS);
                values.GL_MAX_FRAMEBUFFER_SAMPLES                       = getValue<int32_t>(GL_MAX_FRAMEBUFFER_SAMPLES);
            }
            if (extensions.GL_ARB_shader_storage_buffer_object) {
                values.GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS              = getValue<int32_t>(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS);
                values.GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS            = getValue<int32_t>(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS);
                values.GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS        = getValue<int32_t>(GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS);
                values.GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS     = getValue<int32_t>(GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS);
                values.GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS            = getValue<int32_t>(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS);
                values.GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS             = getValue<int32_t>(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS);
                values.GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS            = getValue<int32_t>(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS);
                values.GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS            = getValue<int32_t>(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS);
                values.GL_MAX_SHADER_STORAGE_BLOCK_SIZE                 = getValue<int32_t>(GL_MAX_SHADER_STORAGE_BLOCK_SIZE);
                values.GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT        = getValue<int32_t>(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT);
            }
            if (extensions.GL_KHR_debug) {
                values.GL_MAX_DEBUG_MESSAGE_LENGTH                      = getValue<int32_t>(GL_MAX_DEBUG_MESSAGE_LENGTH);
                values.GL_MAX_DEBUG_LOGGED_MESSAGES                     = getValue<int32_t>(GL_MAX_DEBUG_LOGGED_MESSAGES);
                values.GL_MAX_DEBUG_GROUP_STACK_DEPTH                   = getValue<int32_t>(GL_MAX_DEBUG_GROUP_STACK_DEPTH);
                values.GL_MAX_LABEL_LENGTH                              = getValue<int32_t>(GL_MAX_LABEL_LENGTH);
            }
        //Not core
            if (extensions.GL_ARB_sparse_buffer) {
                values.GL_SPARSE_BUFFER_PAGE_SIZE_ARB                   = getValue<int32_t>(GL_SPARSE_BUFFER_PAGE_SIZE_ARB);
            }
            if (extensions.GL_EXT_texture_filter_anisotropic) {
                values.GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT                = getValue<int32_t>(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT);
            }
    }

    static bool checkAndSetFeature(config::FeatureSetting featureSetting, string featureName, bool runtimeCondition) {
        if (featureSetting == config::FeatureSetting::notSupported) return false;
        if (featureSetting == config::FeatureSetting::mustBeSupported && !runtimeCondition)
            crash("Feature config::" + featureName + " set to mustBeSupported, but not supported!");
        return runtimeCondition;
    }

    void ContextGroup_::checkAndSetFeatures() {
        feature.drawIndirect              = checkAndSetFeature(config::drawIndirect               , "drawIndirect"                , config::glEqualOrGreater(4, 0) || extensions.GL_ARB_draw_indirect);
        feature.blendModePerDrawbuffer    = checkAndSetFeature(config::blendModePerDrawbuffer     , "blendModePerDrawbuffer"      , config::glEqualOrGreater(4, 0) || extensions.GL_ARB_draw_buffers_blend);
        feature.drawBaseInstance          = checkAndSetFeature(config::drawBaseInstance           , "drawBaseInstance"            , config::glEqualOrGreater(4, 2) || extensions.GL_ARB_base_instance);
        feature.bptc                      = checkAndSetFeature(config::bptc                       , "bptc"                        , config::glEqualOrGreater(4, 2) || extensions.GL_ARB_texture_compression_bptc);
        feature.atomicCounter             = checkAndSetFeature(config::atomicCounter              , "atomicCounter"               , config::glEqualOrGreater(4, 2) || extensions.GL_ARB_shader_atomic_counters);
        feature.shaderStorageBufferObject = checkAndSetFeature(config::shaderStorageBufferObject  , "shaderStorageBufferObject"   , config::glEqualOrGreater(4, 3) || extensions.GL_ARB_shader_storage_buffer_object);
        feature.astc                      = checkAndSetFeature(config::astc                       , "astc"                        , config::glEqualOrGreater(4, 3) || extensions.GL_KHR_texture_compression_astc_hdr);
        feature.textureView               = checkAndSetFeature(config::textureView                , "textureView"                 , config::glEqualOrGreater(4, 3) || extensions.GL_ARB_texture_view);
        feature.drawIndirectCount         = checkAndSetFeature(config::drawIndirectCount          , "drawIndirectCount"           , config::glEqualOrGreater(4, 6) || extensions.GL_ARB_indirect_parameters);
        feature.polygonOffsetClamp        = checkAndSetFeature(config::polygonOffsetClamp         , "polygonOffsetClamp"          , config::glEqualOrGreater(4, 6) || extensions.GL_ARB_polygon_offset_clamp);
        feature.anisotropicFilter         = checkAndSetFeature(config::anisotropicFilter          , "anisotropicFilter"           , config::glEqualOrGreater(4, 6) || extensions.GL_ARB_texture_filter_anisotropic || extensions.GL_EXT_texture_filter_anisotropic);
        feature.spirv                     = checkAndSetFeature(config::spirv                      , "spirv"                       ,                                   extensions.GL_ARB_gl_spirv);
    }
}
