#include "glCompact/ContextGroup_.hpp"
#include "glCompact/gl/Constants.hpp"
#include "glCompact/gl/ConstantsCustom.hpp"
#include "glCompact/ThreadContextGroup.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <string>

using namespace std;
using namespace glCompact::gl;

namespace glCompact {
    ContextGroup_::ContextGroup_(
        void*(*getGlFunctionPointer)(const char* glFunctionName)
    ) {
        functions.init(getGlFunctionPointer);

        const char* versionString = reinterpret_cast<const char*>(functions.glGetString(GL_VERSION));
        if (!versionString)
            crash("No active OpenGL context in this thread! Can not initalize without one!");

        //TODO make this a regex in case anyone returns the version with more then one digit per major/minor
        //raw ascii to integer
        int major = versionString[0] - 48;
        int minor = versionString[2] - 48;
        //this needs 3.0 minimum, but the user may incorrectly initiated OpenGL and we want to give at last a decent error message when already crashing here!
        //functions.glGetIntegerv(GL_MAJOR_VERSION, &major);
        //functions.glGetIntegerv(GL_MINOR_VERSION, &minor);
        if (major >  Config::MIN_MAJOR
        || (major == Config::MIN_MAJOR && minor >= Config::MIN_MINOR)) {
            //all okey dokey!
        } else {
            crash("glCompact needs at last OpenGL " + std::to_string(Config::MIN_MAJOR) + "." + std::to_string(Config::MIN_MINOR) + " but only detected " + std::to_string(version.major) + "." + std::to_string(version.minor) + "!");
        }
        version.major = major;
        version.minor = minor;
        GLenum contextProfileMask;
        functions.glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &contextProfileMask);
        version.core              = contextProfileMask & GL_CONTEXT_CORE_PROFILE_BIT;
        version.forwardCompatible = contextProfileMask & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT;
        version.debug             = contextProfileMask & GL_CONTEXT_FLAG_DEBUG_BIT;
        version.robustAccess      = contextProfileMask & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT;
        version.noErrorReporting  = contextProfileMask & GL_CONTEXT_FLAG_NO_ERROR_BIT;

        extensions.init(this);
        setAllCoreExtensionTrue();
        getAllValue();
        checkAndOrSetFeatures();

        //DISABELING EXTENSIONS TO TEST DIFFERENT PATHS
        //*const_cast<bool*>(&extensions.GL_ARB_texture_storage)       = false; //NOTE: MAY CRASH IF TEXTURE VIEWS ARE USED! (Because they only can be created from texStorage objects!)
        //*const_cast<bool*>(&extensions.GL_ARB_multi_bind)            = false;
        //*const_cast<bool*>(&extensions.GL_ARB_vertex_attrib_binding) = false;

        //*const_cast<bool*>(&extensions.GL_ARB_direct_state_access)   = false;
        //*const_cast<bool*>(&extensions.GL_EXT_direct_state_access)   = false;

        //*const_cast<bool*>(&extensions.GL_ARB_ES3_compatibility)     = false;
    }

    /**
    * There are extensions in OpenGL that behaves exactly the same as the later implemented core functionality.
    * Somebody hat the ingenious idea to not list extensions in the supported list if the OpenGL version running integrates them as core functionality.
    * This forces one to always check the extension list AND if the OpenGL version is => where the extension became part of the core standard.
    *
    * This function simply sets all extensions to true if the OpenGL version is met where they became part of core.
    *
    * Only checking the extension now is enough!
    */
    void ContextGroup_::setAllCoreExtensionTrue() {
        if (version.equalOrGreater(1, 2)) {
            extensions.GL_EXT_texture3D = true;
            //extensions.GL_OES_texture_3D = true;
        }
        if (version.equalOrGreater(1, 3)) {
            extensions.GL_ARB_multitexture = true;
            extensions.GL_ARB_transpose_matrix = true;
            extensions.GL_ARB_multisample = true;
            extensions.GL_ARB_texture_env_add = true;
            extensions.GL_ARB_texture_cube_map = true;
            extensions.GL_ARB_texture_compression = true;
            extensions.GL_ARB_texture_border_clamp = true;
            extensions.GL_ARB_texture_env_combine = true;
            extensions.GL_ARB_texture_env_dot3 = true;
        }
        if (version.equalOrGreater(1, 4)) {
            extensions.GL_ARB_point_parameters = true;
            extensions.GL_ARB_texture_env_crossbar = true;
            extensions.GL_ARB_texture_mirrored_repeat = true;
            extensions.GL_ARB_depth_texture = true;
            extensions.GL_ARB_shadow = true;
            extensions.GL_ARB_window_pos = true;
        }
        if (version.equalOrGreater(1, 5)) {
            extensions.GL_ARB_vertex_buffer_object = true;
            extensions.GL_ARB_occlusion_query = true;
        }
        if (version.equalOrGreater(2, 0)) {
            extensions.GL_ARB_shader_objects = true;
            extensions.GL_ARB_vertex_shader = true;
            extensions.GL_ARB_fragment_shader = true;
            extensions.GL_ARB_shading_language_100 = true;
            extensions.GL_ARB_texture_non_power_of_two = true;
            extensions.GL_ARB_point_sprite = true;
            extensions.GL_ARB_draw_buffers = true;
        }
        if (version.equalOrGreater(2, 1)) {
            extensions.GL_ARB_pixel_buffer_object = true;
        }
        if (version.equalOrGreater(3, 0)) {
            extensions.GL_ARB_color_buffer_float = true;
            extensions.GL_ARB_half_float_pixel = true;
            extensions.GL_ARB_texture_float = true;
            extensions.GL_ARB_depth_buffer_float = true;
            extensions.GL_ARB_framebuffer_object = true;
            extensions.GL_ARB_framebuffer_sRGB = true;
            extensions.GL_ARB_half_float_vertex = true;
            extensions.GL_ARB_map_buffer_range = true;
            extensions.GL_ARB_texture_compression_rgtc = true;
            extensions.GL_ARB_texture_rg = true;
            extensions.GL_ARB_vertex_array_object = true;
            extensions.GL_ARB_shader_texture_lod = true;
            extensions.GL_EXT_texture_array = true;
        }
        if (version.equalOrGreater(3, 1)) {
            extensions.GL_ARB_draw_instanced = true;
            extensions.GL_ARB_texture_buffer_object = true;
            extensions.GL_ARB_uniform_buffer_object = true;
            extensions.GL_ARB_copy_buffer = true;
            extensions.GL_ARB_texture_rectangle = true;
        }
        if (version.equalOrGreater(3, 2)) {
            extensions.GL_ARB_geometry_shader4 = true;
            extensions.GL_ARB_instanced_arrays = true;
            extensions.GL_ARB_depth_clamp = true;
            extensions.GL_ARB_draw_elements_base_vertex = true;
            extensions.GL_ARB_fragment_coord_conventions = true;
            extensions.GL_ARB_provoking_vertex = true;
            extensions.GL_ARB_seamless_cube_map = true;
            extensions.GL_ARB_sync = true;
            extensions.GL_ARB_texture_multisample = true;
            extensions.GL_ARB_vertex_array_bgra = true;
        }
        if (version.equalOrGreater(3, 3)) {
            extensions.GL_ARB_blend_func_extended = true;
            extensions.GL_ARB_explicit_attrib_location = true;
            extensions.GL_ARB_occlusion_query2 = true;
            extensions.GL_ARB_sampler_objects = true;
            extensions.GL_ARB_shader_bit_encoding = true;
            extensions.GL_ARB_texture_rgb10_a2ui = true;
            extensions.GL_ARB_texture_swizzle = true;
            extensions.GL_ARB_timer_query = true;
            extensions.GL_ARB_vertex_type_2_10_10_10_rev = true;
        }
        if (version.equalOrGreater(4, 0)) {
            extensions.GL_ARB_draw_buffers_blend = true;
            extensions.GL_ARB_sample_shading = true;
            extensions.GL_ARB_texture_cube_map_array /*uses ARB suffix, but only brings new const. And the Core constants have the same values.*/ = true;
            extensions.GL_ARB_texture_gather = true;
            extensions.GL_ARB_draw_indirect = true;
            extensions.GL_ARB_gpu_shader5 = true;
            extensions.GL_ARB_gpu_shader_fp64 = true;
            extensions.GL_ARB_shader_subroutine = true;
            extensions.GL_ARB_tessellation_shader = true;
            extensions.GL_ARB_texture_buffer_object_rgb32 = true;
            extensions.GL_ARB_transform_feedback2 = true;
            extensions.GL_ARB_transform_feedback3 = true;
        }
        if (version.equalOrGreater(4, 1)) {
            extensions.GL_ARB_ES2_compatibility = true;
            extensions.GL_ARB_get_program_binary = true;
            extensions.GL_ARB_separate_shader_objects = true;
            extensions.GL_ARB_shader_precision = true;
            extensions.GL_ARB_vertex_attrib_64bit = true;
            extensions.GL_ARB_viewport_array = true;
        }
        if (version.equalOrGreater(4, 2)) {
            extensions.GL_ARB_base_instance = true;
            extensions.GL_ARB_shading_language_420pack = true;
            extensions.GL_ARB_transform_feedback_instanced = true;
            extensions.GL_ARB_compressed_texture_pixel_storage = true;
            extensions.GL_ARB_conservative_depth = true;
            extensions.GL_ARB_internalformat_query = true;
            extensions.GL_ARB_map_buffer_alignment = true;
            extensions.GL_ARB_shader_atomic_counters = true;
            extensions.GL_ARB_shader_image_load_store = true;
            extensions.GL_ARB_shading_language_packing = true;
            extensions.GL_ARB_texture_storage = true;
            extensions.GL_ARB_texture_compression_bptc = true;
        }
        if (version.equalOrGreater(4, 3)) {
            extensions.GL_KHR_texture_compression_astc_ldr = true;
            extensions.GL_ARB_arrays_of_arrays = true;
            extensions.GL_ARB_clear_buffer_object = true;
            extensions.GL_ARB_compute_shader = true;
            extensions.GL_ARB_copy_image = true;
            extensions.GL_ARB_texture_view = true;
            extensions.GL_ARB_vertex_attrib_binding = true;
            extensions.GL_ARB_robustness_isolation = true;
            extensions.GL_ARB_ES3_compatibility = true;
            extensions.GL_ARB_explicit_uniform_location = true;
            extensions.GL_ARB_fragment_layer_viewport = true;
            extensions.GL_ARB_framebuffer_no_attachments = true;
            extensions.GL_ARB_internalformat_query2 = true;
            extensions.GL_ARB_invalidate_subdata = true;
            extensions.GL_ARB_multi_draw_indirect = true;
            extensions.GL_ARB_program_interface_query = true;
            extensions.GL_ARB_robust_buffer_access_behavior = true;
            extensions.GL_ARB_shader_image_size = true;
            extensions.GL_ARB_shader_storage_buffer_object = true;
            extensions.GL_ARB_stencil_texturing = true;
            extensions.GL_ARB_texture_buffer_range = true;
            extensions.GL_ARB_texture_query_levels = true;
            extensions.GL_ARB_texture_storage_multisample = true;
            extensions.GL_KHR_debug = true;
        }
        if (version.equalOrGreater(4, 4)) {
            extensions.GL_ARB_buffer_storage = true;
            extensions.GL_ARB_clear_texture = true;
            extensions.GL_ARB_enhanced_layouts = true;
            extensions.GL_ARB_multi_bind = true;
            extensions.GL_ARB_query_buffer_object = true;
            extensions.GL_ARB_texture_mirror_clamp_to_edge = true;
            extensions.GL_ARB_texture_stencil8 = true;
            extensions.GL_ARB_vertex_type_10f_11f_11f_rev = true;
        }
        if (version.equalOrGreater(4, 5)) {
            extensions.GL_ARB_ES3_1_compatibility = true;
            extensions.GL_ARB_clip_control = true;
            extensions.GL_ARB_conditional_render_inverted = true;
            extensions.GL_ARB_cull_distance = true;
            extensions.GL_ARB_direct_state_access = true;
            extensions.GL_ARB_get_texture_sub_image = true;
            extensions.GL_ARB_texture_barrier = true;
            extensions.GL_KHR_context_flush_control = true;
            extensions.GL_ARB_derivative_control = true;
            extensions.GL_ARB_shader_texture_image_samples = true;
            extensions.GL_KHR_robustness = true;
        }
        if (version.equalOrGreater(4, 6)) {
            extensions.GL_ARB_indirect_parameters = true;
            extensions.GL_ARB_pipeline_statistics_query = true;
            extensions.GL_ARB_polygon_offset_clamp = true;
            extensions.GL_KHR_no_error = true;
            extensions.GL_ARB_shader_atomic_counter_ops = true; //GLSL Only
            extensions.GL_ARB_shader_draw_parameters = true;
            extensions.GL_ARB_shader_group_vote = true; //GLSL Only
            extensions.GL_ARB_gl_spirv = true;
            extensions.GL_ARB_spirv_extensions = true;
            extensions.GL_ARB_texture_filter_anisotropic = true;
            extensions.GL_ARB_transform_feedback_overflow_query = true;
        }
    }

    void ContextGroup_::getAllValue() {
        //Core forever?
            functions.glGetIntegerv(GL_MAX_TEXTURE_SIZE, &values.GL_MAX_TEXTURE_SIZE);

        //Core since 1.2
            if (extensions.GL_EXT_texture3D) {
                functions.glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &values.GL_MAX_3D_TEXTURE_SIZE);
            }
        //Core since 1.3
            //GL_ARB_texture_cube_map
            functions.glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &values.GL_MAX_CUBE_MAP_TEXTURE_SIZE);

        //Core since 2.0
            if (version.equalOrGreater(2, 0)) {
                functions.glGetIntegerv(GL_MAX_DRAW_BUFFERS, &values.GL_MAX_DRAW_BUFFERS);
            }
            if (extensions.GL_ARB_vertex_shader) {
                functions.glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS,    &values.GL_MAX_VERTEX_UNIFORM_COMPONENTS);
                functions.glGetIntegerv(GL_MAX_VARYING_FLOATS,               &values.GL_MAX_VARYING_FLOATS);
                functions.glGetIntegerv(GL_MAX_VERTEX_ATTRIBS,               &values.GL_MAX_VERTEX_ATTRIBS);
                functions.glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,          &values.GL_MAX_TEXTURE_IMAGE_UNITS);
                functions.glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,   &values.GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
                functions.glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &values.GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
                functions.glGetIntegerv(GL_MAX_TEXTURE_COORDS,               &values.GL_MAX_TEXTURE_COORDS);
            }
        //Core since 3.0
            if (extensions.GL_ARB_framebuffer_object) {
                functions.glGetIntegerv(GL_MAX_SAMPLES,           &values.GL_MAX_SAMPLES);
                functions.glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &values.GL_MAX_COLOR_ATTACHMENTS);
                functions.glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &values.GL_MAX_RENDERBUFFER_SIZE);
            }
            if (extensions.GL_EXT_texture_array) {
                functions.glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &values.GL_MAX_ARRAY_TEXTURE_LAYERS);
            }
        //Core since 3.1
            if (extensions.GL_ARB_texture_buffer_object) {
                functions.glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &values.GL_MAX_TEXTURE_BUFFER_SIZE);
            }
            if (extensions.GL_ARB_uniform_buffer_object) {
                functions.glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS,                &values.GL_MAX_VERTEX_UNIFORM_BLOCKS);
                functions.glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS,              &values.GL_MAX_GEOMETRY_UNIFORM_BLOCKS);
                functions.glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS,              &values.GL_MAX_FRAGMENT_UNIFORM_BLOCKS);
                functions.glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS,              &values.GL_MAX_COMBINED_UNIFORM_BLOCKS);
                functions.glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS,              &values.GL_MAX_UNIFORM_BUFFER_BINDINGS);
                functions.glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE,                   &values.GL_MAX_UNIFORM_BLOCK_SIZE);
                functions.glGetIntegerv(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS,   &values.GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS);
                functions.glGetIntegerv(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS, &values.GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS);
                functions.glGetIntegerv(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS, &values.GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS);
                functions.glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,          &values.GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT);
            }
        //Core since 3.2
            if (extensions.GL_ARB_texture_multisample) {
                functions.glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &values.GL_MAX_COLOR_TEXTURE_SAMPLES);
                functions.glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &values.GL_MAX_DEPTH_TEXTURE_SAMPLES);
                functions.glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &values.GL_MAX_INTEGER_SAMPLES);
            }
            if (extensions.GL_ARB_sync) {
                functions.glGetInteger64v(GL_MAX_SERVER_WAIT_TIMEOUT, &values.GL_MAX_SERVER_WAIT_TIMEOUT);
            }
        //Core since 3.3
            if (extensions.GL_ARB_blend_func_extended) {
                functions.glGetIntegerv(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS, &values.GL_MAX_DUAL_SOURCE_DRAW_BUFFERS);
            }
        //Core since 4.0
            if (extensions.GL_ARB_transform_feedback3) {
                functions.glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_BUFFERS, &values.GL_MAX_TRANSFORM_FEEDBACK_BUFFERS);
                functions.glGetIntegerv(GL_MAX_VERTEX_STREAMS, &values.GL_MAX_VERTEX_STREAMS);
            }
        //Core since 4.2
            if (extensions.GL_ARB_shader_atomic_counters) {
                functions.glGetIntegerv(GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS, &values.GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS);
                functions.glGetIntegerv(GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS, &values.GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS);
                functions.glGetIntegerv(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS, &values.GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS);
                functions.glGetIntegerv(GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS, &values.GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS);
                functions.glGetIntegerv(GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS, &values.GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS);
                functions.glGetIntegerv(GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS, &values.GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS);
                functions.glGetIntegerv(GL_MAX_VERTEX_ATOMIC_COUNTERS, &values.GL_MAX_VERTEX_ATOMIC_COUNTERS);
                functions.glGetIntegerv(GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS, &values.GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS);
                functions.glGetIntegerv(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS, &values.GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS);
                functions.glGetIntegerv(GL_MAX_GEOMETRY_ATOMIC_COUNTERS, &values.GL_MAX_GEOMETRY_ATOMIC_COUNTERS);
                functions.glGetIntegerv(GL_MAX_FRAGMENT_ATOMIC_COUNTERS, &values.GL_MAX_FRAGMENT_ATOMIC_COUNTERS);
                functions.glGetIntegerv(GL_MAX_COMBINED_ATOMIC_COUNTERS, &values.GL_MAX_COMBINED_ATOMIC_COUNTERS);
                functions.glGetIntegerv(GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE, &values.GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE);
                functions.glGetIntegerv(GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS, &values.GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS);
            }
            if (extensions.GL_ARB_shader_image_load_store) {
                functions.glGetIntegerv(GL_MAX_IMAGE_UNITS, &values.GL_MAX_IMAGE_UNITS);
                functions.glGetIntegerv(GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS, &values.GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS);
                functions.glGetIntegerv(GL_MAX_IMAGE_SAMPLES, &values.GL_MAX_IMAGE_SAMPLES);
                functions.glGetIntegerv(GL_MAX_VERTEX_IMAGE_UNIFORMS, &values.GL_MAX_VERTEX_IMAGE_UNIFORMS);
                functions.glGetIntegerv(GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS, &values.GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS);
                functions.glGetIntegerv(GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS, &values.GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS);
                functions.glGetIntegerv(GL_MAX_GEOMETRY_IMAGE_UNIFORMS, &values.GL_MAX_GEOMETRY_IMAGE_UNIFORMS);
                functions.glGetIntegerv(GL_MAX_FRAGMENT_IMAGE_UNIFORMS, &values.GL_MAX_FRAGMENT_IMAGE_UNIFORMS);
                functions.glGetIntegerv(GL_MAX_COMBINED_IMAGE_UNIFORMS, &values.GL_MAX_COMBINED_IMAGE_UNIFORMS);
            }
        //Core since 4.3
            if (extensions.GL_ARB_vertex_attrib_binding) {
                functions.glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &values.GL_MAX_VERTEX_ATTRIB_BINDINGS);
                functions.glGetIntegerv(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET, &values.GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET);
            }
            if (extensions.GL_ARB_texture_buffer_range) {
                functions.glGetIntegerv(GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT, &values.GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT);
            }
            if (extensions.GL_ARB_compute_shader) {
                functions.glGetIntegerv(GL_MAX_COMPUTE_UNIFORM_BLOCKS, &values.GL_MAX_COMPUTE_UNIFORM_BLOCKS);
                functions.glGetIntegerv(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, &values.GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS);
                functions.glGetIntegerv(GL_MAX_COMPUTE_IMAGE_UNIFORMS, &values.GL_MAX_COMPUTE_IMAGE_UNIFORMS);
                functions.glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &values.GL_MAX_COMPUTE_SHARED_MEMORY_SIZE);
                functions.glGetIntegerv(GL_MAX_COMPUTE_UNIFORM_COMPONENTS, &values.GL_MAX_COMPUTE_UNIFORM_COMPONENTS);
                functions.glGetIntegerv(GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS, &values.GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS);
                functions.glGetIntegerv(GL_MAX_COMPUTE_ATOMIC_COUNTERS, &values.GL_MAX_COMPUTE_ATOMIC_COUNTERS);
                functions.glGetIntegerv(GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS, &values.GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS);
                functions.glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &values.GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS);
                functions.glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_COUNT, &values.GL_MAX_COMPUTE_WORK_GROUP_COUNT);
                functions.glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_SIZE, &values.GL_MAX_COMPUTE_WORK_GROUP_SIZE);
            }
            if (extensions.GL_ARB_framebuffer_no_attachments) {
                functions.glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &values.GL_MAX_FRAMEBUFFER_WIDTH);
                functions.glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &values.GL_MAX_FRAMEBUFFER_HEIGHT);
                functions.glGetIntegerv(GL_MAX_FRAMEBUFFER_LAYERS, &values.GL_MAX_FRAMEBUFFER_LAYERS);
                functions.glGetIntegerv(GL_MAX_FRAMEBUFFER_SAMPLES, &values.GL_MAX_FRAMEBUFFER_SAMPLES);
            }
            if (extensions.GL_ARB_shader_storage_buffer_object) {
                functions.glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &values.GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS);
                functions.glGetIntegerv(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS, &values.GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS);
                functions.glGetIntegerv(GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS, &values.GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS);
                functions.glGetIntegerv(GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS, &values.GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS);
                functions.glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &values.GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS);
                functions.glGetIntegerv(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, &values.GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS);
                functions.glGetIntegerv(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, &values.GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS);
                functions.glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &values.GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS);
                functions.glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &values.GL_MAX_SHADER_STORAGE_BLOCK_SIZE);
                functions.glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &values.GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT);
            }
            if (extensions.GL_KHR_debug) {
                functions.glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH, &values.GL_MAX_DEBUG_MESSAGE_LENGTH);
                functions.glGetIntegerv(GL_MAX_DEBUG_LOGGED_MESSAGES, &values.GL_MAX_DEBUG_LOGGED_MESSAGES);
                functions.glGetIntegerv(GL_MAX_DEBUG_GROUP_STACK_DEPTH, &values.GL_MAX_DEBUG_GROUP_STACK_DEPTH);
                functions.glGetIntegerv(GL_MAX_LABEL_LENGTH, &values.GL_MAX_LABEL_LENGTH);
            }
        //Not core
            if (extensions.GL_ARB_sparse_buffer) {
                functions.glGetIntegerv(GL_SPARSE_BUFFER_PAGE_SIZE_ARB, &values.GL_SPARSE_BUFFER_PAGE_SIZE_ARB);
            }
            if (extensions.GL_EXT_texture_filter_anisotropic) {
                functions.glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &values.GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT);
            }
    }

    template <Config::FeatureSetting featureSetting>
    static void checkAndOrSetFeature(string featureName, bool& featureSupported, bool runtimeCondition) {
        if (featureSetting == Config::FeatureSetting::notSupported) return;
        if (featureSetting == Config::FeatureSetting::mustBeSupported) {
            if (runtimeCondition) return;
            crash("Feature Config::" + featureName + " set to mustBeSupported, but not supported!");
        }
        if (featureSetting == Config::FeatureSetting::runtimeDetection) {
            featureSupported = runtimeCondition;
        }
    }

    void ContextGroup_::checkAndOrSetFeatures() {
        checkAndOrSetFeature<Config::drawIndirect               >("drawIndirect"                , feature.drawIndirect              , Config::glEqualOrGreater(4, 0) || threadContextGroup_->extensions.GL_ARB_draw_indirect);
        checkAndOrSetFeature<Config::blendModePerDrawbuffer     >("blendModePerDrawbuffer"      , feature.blendModePerDrawbuffer    , Config::glEqualOrGreater(4, 0) || threadContextGroup_->extensions.GL_ARB_draw_buffers_blend);
        checkAndOrSetFeature<Config::drawBaseInstance           >("drawBaseInstance"            , feature.drawBaseInstance          , Config::glEqualOrGreater(4, 2) || threadContextGroup_->extensions.GL_ARB_base_instance);
        checkAndOrSetFeature<Config::bptc                       >("bptc"                        , feature.bptc                      , Config::glEqualOrGreater(4, 2) || threadContextGroup_->extensions.GL_ARB_texture_compression_bptc);
        checkAndOrSetFeature<Config::atomicCounter              >("atomicCounter"               , feature.atomicCounter             , Config::glEqualOrGreater(4, 2) || threadContextGroup_->extensions.GL_ARB_shader_atomic_counters);
        checkAndOrSetFeature<Config::shaderStorageBufferObject  >("shaderStorageBufferObject"   , feature.shaderStorageBufferObject , Config::glEqualOrGreater(4, 3) || threadContextGroup_->extensions.GL_ARB_shader_storage_buffer_object);
        checkAndOrSetFeature<Config::astc                       >("astc"                        , feature.astc                      , Config::glEqualOrGreater(4, 3) || threadContextGroup_->extensions.GL_KHR_texture_compression_astc_hdr);
        checkAndOrSetFeature<Config::textureView                >("textureView"                 , feature.textureView               , Config::glEqualOrGreater(4, 3) || threadContextGroup_->extensions.GL_ARB_texture_view);
        checkAndOrSetFeature<Config::drawIndirectCount          >("drawIndirectCount"           , feature.drawIndirectCount         , Config::glEqualOrGreater(4, 6) || threadContextGroup_->extensions.GL_ARB_indirect_parameters);
        checkAndOrSetFeature<Config::polygonOffsetClamp         >("polygonOffsetClamp"          , feature.polygonOffsetClamp        , Config::glEqualOrGreater(4, 6) || threadContextGroup_->extensions.GL_ARB_polygon_offset_clamp);
        checkAndOrSetFeature<Config::anisotropicFilter          >("anisotropicFilter"           , feature.anisotropicFilter         , Config::glEqualOrGreater(4, 6) || threadContextGroup_->extensions.GL_ARB_texture_filter_anisotropic || threadContextGroup_->extensions.GL_EXT_texture_filter_anisotropic);
        checkAndOrSetFeature<Config::spirv                      >("spirv"                       , feature.spirv                     ,                                   threadContextGroup_->extensions.GL_ARB_gl_spirv);
    }
}
