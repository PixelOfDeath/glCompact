#pragma once
#include "glCompact/gl/Types.hpp"

#include <glm/glm.hpp>

namespace glCompact {
    namespace gl {
        class Values {
            public:
                void init();
                //Core forever?
                    int32_t GL_MAX_TEXTURE_SIZE;

                //Core since 1.2
                    //GL_EXT_texture3D
                    int32_t GL_MAX_3D_TEXTURE_SIZE;

                //Core since 1.3
                    //GL_ARB_texture_cube_map
                    int32_t GL_MAX_CUBE_MAP_TEXTURE_SIZE;

                //Core since 2.0
                    int32_t GL_MAX_DRAW_BUFFERS;

                    //GL_ARB_vertex_shader
                    int32_t GL_MAX_VERTEX_UNIFORM_COMPONENTS;
                    int32_t GL_MAX_VARYING_COMPONENTS; //before 3.0: GL_MAX_VARYING_FLOATS
                    int32_t GL_MAX_VERTEX_ATTRIBS;
                    int32_t GL_MAX_TEXTURE_IMAGE_UNITS;
                    int32_t GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS;
                    int32_t GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS;

                //Core since 3.0
                    //GL_ARB_framebuffer_object
                    int32_t GL_MAX_SAMPLES; //for all RenderBuffer formats except integer
                    int32_t GL_MAX_COLOR_ATTACHMENTS;
                    int32_t GL_MAX_RENDERBUFFER_SIZE;

                    //GL_EXT_texture_array
                    int32_t GL_MAX_ARRAY_TEXTURE_LAYERS;

                //Core since 3.1
                    //GL_ARB_texture_buffer_object
                    int32_t GL_MAX_TEXTURE_BUFFER_SIZE;

                    //GL_ARB_uniform_buffer_object
                    int32_t GL_MAX_VERTEX_UNIFORM_BLOCKS;
                    int32_t GL_MAX_GEOMETRY_UNIFORM_BLOCKS;
                    int32_t GL_MAX_FRAGMENT_UNIFORM_BLOCKS;
                    int32_t GL_MAX_COMBINED_UNIFORM_BLOCKS;
                    int32_t GL_MAX_UNIFORM_BUFFER_BINDINGS;
                    int32_t GL_MAX_UNIFORM_BLOCK_SIZE;
                    int32_t GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT;

                //Core since 3.2
                    //GL_ARB_texture_multisample
                    int32_t GL_MAX_COLOR_TEXTURE_SAMPLES;
                    int32_t GL_MAX_DEPTH_TEXTURE_SAMPLES;
                    int32_t GL_MAX_INTEGER_SAMPLES;

                    //GL_ARB_sync
                    int64_t GL_MAX_SERVER_WAIT_TIMEOUT;

                //Core since 3.3
                    //GL_ARB_blend_func_extended
                    int32_t GL_MAX_DUAL_SOURCE_DRAW_BUFFERS;

                //Core since 4.0
                    //GL_ARB_transform_feedback3
                    int32_t GL_MAX_TRANSFORM_FEEDBACK_BUFFERS;
                    int32_t GL_MAX_VERTEX_STREAMS;

                //Core since 4.2
                    //GL_ARB_shader_atomic_counters
                    int32_t GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS;
                    int32_t GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS;
                    int32_t GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS;
                    int32_t GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS;
                    int32_t GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS;
                    int32_t GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS;
                    int32_t GL_MAX_VERTEX_ATOMIC_COUNTERS;
                    int32_t GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS;
                    int32_t GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS;
                    int32_t GL_MAX_GEOMETRY_ATOMIC_COUNTERS;
                    int32_t GL_MAX_FRAGMENT_ATOMIC_COUNTERS;
                    int32_t GL_MAX_COMBINED_ATOMIC_COUNTERS;
                    int32_t GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE;
                    int32_t GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS;

                    //GL_ARB_shader_image_load_store
                    int32_t GL_MAX_IMAGE_UNITS;
                    int32_t GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS;
                    int32_t GL_MAX_IMAGE_SAMPLES;
                    int32_t GL_MAX_VERTEX_IMAGE_UNIFORMS;
                    int32_t GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS;
                    int32_t GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS;
                    int32_t GL_MAX_GEOMETRY_IMAGE_UNIFORMS;
                    int32_t GL_MAX_FRAGMENT_IMAGE_UNIFORMS;
                    int32_t GL_MAX_COMBINED_IMAGE_UNIFORMS;

                //Core since 4.3
                    //GL_ARB_vertex_attrib_binding
                    int32_t GL_MAX_VERTEX_ATTRIB_BINDINGS;
                    int32_t GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET;

                    //GL_ARB_texture_buffer_range
                    int32_t GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT;

                    //GL_ARB_compute_shader
                    int32_t GL_MAX_COMPUTE_UNIFORM_BLOCKS;
                    int32_t GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS;
                    int32_t GL_MAX_COMPUTE_IMAGE_UNIFORMS;
                    int32_t GL_MAX_COMPUTE_SHARED_MEMORY_SIZE;
                    int32_t GL_MAX_COMPUTE_UNIFORM_COMPONENTS;
                    int32_t GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS;
                    int32_t GL_MAX_COMPUTE_ATOMIC_COUNTERS;
                    int32_t GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS;
                    int32_t GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS;
                    glm::uvec3 GL_MAX_COMPUTE_WORK_GROUP_COUNT;
                    glm::uvec3 GL_MAX_COMPUTE_WORK_GROUP_SIZE;

                    //GL_ARB_framebuffer_no_attachments
                    int32_t GL_MAX_FRAMEBUFFER_WIDTH;
                    int32_t GL_MAX_FRAMEBUFFER_HEIGHT;
                    int32_t GL_MAX_FRAMEBUFFER_LAYERS;
                    int32_t GL_MAX_FRAMEBUFFER_SAMPLES;

                    //GL_ARB_shader_storage_buffer_object
                    int32_t GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS;
                    int32_t GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS;
                    int32_t GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS;
                    int32_t GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS;
                    int32_t GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS;
                    int32_t GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS;
                    int32_t GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS;
                    int32_t GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS;
                    int32_t GL_MAX_SHADER_STORAGE_BLOCK_SIZE;
                    int32_t GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT;

                    //GL_KHR_debug
                    int32_t GL_MAX_DEBUG_MESSAGE_LENGTH;
                    int32_t GL_MAX_DEBUG_LOGGED_MESSAGES;
                    int32_t GL_MAX_DEBUG_GROUP_STACK_DEPTH;
                    int32_t GL_MAX_LABEL_LENGTH;

                //Not core
                    //GL_ARB_sparse_buffer
                    int32_t GL_SPARSE_BUFFER_PAGE_SIZE_ARB;

                    //GL_EXT_texture_filter_anisotropic
                    int32_t GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT;
        };
    }
}
