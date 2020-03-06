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
///\cond HIDDEN_FROM_DOXYGEN
#pragma once
#include "glCompact/gl/Types.hpp"

namespace glCompact {
    namespace gl {
        class Values {
            public:
                void init();
                //Core forever?
                    GLint   GL_MAX_TEXTURE_SIZE                              = 0;

                //Core since 1.2
                    //GL_EXT_texture3D
                    GLint   GL_MAX_3D_TEXTURE_SIZE                           = 0;

                //Core since 1.3
                    //GL_ARB_texture_cube_map
                    GLint   GL_MAX_CUBE_MAP_TEXTURE_SIZE                     = 0;

                //Core since 2.0
                    GLint   GL_MAX_DRAW_BUFFERS                              = 0;

                    //GL_ARB_vertex_shader
                    GLint   GL_MAX_VERTEX_UNIFORM_COMPONENTS                 = 0;
                    GLint   GL_MAX_VARYING_FLOATS                            = 0;
                    GLint   GL_MAX_VERTEX_ATTRIBS                            = 0;
                    GLint   GL_MAX_TEXTURE_IMAGE_UNITS                       = 0;
                    GLint   GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS                = 0;
                    GLint   GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS              = 0;
                    GLint   GL_MAX_TEXTURE_COORDS                            = 0;

                //Core since 3.0
                    //GL_ARB_framebuffer_object
                    GLint   GL_MAX_SAMPLES                                   = 0; //for all RenderBuffer formats except integer
                    GLint   GL_MAX_COLOR_ATTACHMENTS                         = 0;
                    GLint   GL_MAX_RENDERBUFFER_SIZE                         = 0;

                    //GL_EXT_texture_array
                    GLint   GL_MAX_ARRAY_TEXTURE_LAYERS                      = 0;

                //Core since 3.1
                    //GL_ARB_texture_buffer_object
                    GLint   GL_MAX_TEXTURE_BUFFER_SIZE                       = 0;

                    //GL_ARB_uniform_buffer_object
                    GLint   GL_MAX_VERTEX_UNIFORM_BLOCKS                     = 0;
                    GLint   GL_MAX_GEOMETRY_UNIFORM_BLOCKS                   = 0;
                    GLint   GL_MAX_FRAGMENT_UNIFORM_BLOCKS                   = 0;
                    GLint   GL_MAX_COMBINED_UNIFORM_BLOCKS                   = 0;
                    GLint   GL_MAX_UNIFORM_BUFFER_BINDINGS                   = 0;
                    GLint   GL_MAX_UNIFORM_BLOCK_SIZE                        = 0;
                    GLint   GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS        = 0;
                    GLint   GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS      = 0;
                    GLint   GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS      = 0;
                    GLint   GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT               = 0;

                //Core since 3.2
                    //GL_ARB_texture_multisample
                    GLint   GL_MAX_COLOR_TEXTURE_SAMPLES                     = 0;
                    GLint   GL_MAX_DEPTH_TEXTURE_SAMPLES                     = 0;
                    GLint   GL_MAX_INTEGER_SAMPLES                           = 0;

                    //GL_ARB_sync
                    GLint64 GL_MAX_SERVER_WAIT_TIMEOUT                       = 0;

                //Core since 3.3
                    //GL_ARB_blend_func_extended
                    GLint   GL_MAX_DUAL_SOURCE_DRAW_BUFFERS                  = 0;

                //Core since 4.0
                    //GL_ARB_transform_feedback3
                    GLint   GL_MAX_TRANSFORM_FEEDBACK_BUFFERS                = 0;
                    GLint   GL_MAX_VERTEX_STREAMS                            = 0;

                //Core since 4.2
                    //GL_ARB_shader_atomic_counters
                    GLint   GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS             = 0;
                    GLint   GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS       = 0;
                    GLint   GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS    = 0;
                    GLint   GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS           = 0;
                    GLint   GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS           = 0;
                    GLint   GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS           = 0;
                    GLint   GL_MAX_VERTEX_ATOMIC_COUNTERS                    = 0;
                    GLint   GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS              = 0;
                    GLint   GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS           = 0;
                    GLint   GL_MAX_GEOMETRY_ATOMIC_COUNTERS                  = 0;
                    GLint   GL_MAX_FRAGMENT_ATOMIC_COUNTERS                  = 0;
                    GLint   GL_MAX_COMBINED_ATOMIC_COUNTERS                  = 0;
                    GLint   GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE                = 0;
                    GLint   GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS            = 0;

                    //GL_ARB_shader_image_load_store
                    GLint   GL_MAX_IMAGE_UNITS                               = 0;
                    GLint   GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS = 0;
                    GLint   GL_MAX_IMAGE_SAMPLES                             = 0;
                    GLint   GL_MAX_VERTEX_IMAGE_UNIFORMS                     = 0;
                    GLint   GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS               = 0;
                    GLint   GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS            = 0;
                    GLint   GL_MAX_GEOMETRY_IMAGE_UNIFORMS                   = 0;
                    GLint   GL_MAX_FRAGMENT_IMAGE_UNIFORMS                   = 0;
                    GLint   GL_MAX_COMBINED_IMAGE_UNIFORMS                   = 0;

                //Core since 4.3
                    //GL_ARB_vertex_attrib_binding
                    GLint   GL_MAX_VERTEX_ATTRIB_BINDINGS                    = 0;
                    GLint   GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET             = 0;

                    //GL_ARB_texture_buffer_range
                    GLint   GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT               = 0;

                    //GL_ARB_compute_shader
                    GLint   GL_MAX_COMPUTE_UNIFORM_BLOCKS                    = 0;
                    GLint   GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS               = 0;
                    GLint   GL_MAX_COMPUTE_IMAGE_UNIFORMS                    = 0;
                    GLint   GL_MAX_COMPUTE_SHARED_MEMORY_SIZE                = 0;
                    GLint   GL_MAX_COMPUTE_UNIFORM_COMPONENTS                = 0;
                    GLint   GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS            = 0;
                    GLint   GL_MAX_COMPUTE_ATOMIC_COUNTERS                   = 0;
                    GLint   GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS       = 0;
                    GLint   GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS            = 0;
                    GLint   GL_MAX_COMPUTE_WORK_GROUP_COUNT                  = 0;
                    GLint   GL_MAX_COMPUTE_WORK_GROUP_SIZE                   = 0;

                    //GL_ARB_framebuffer_no_attachments
                    GLint   GL_MAX_FRAMEBUFFER_WIDTH                         = 0;
                    GLint   GL_MAX_FRAMEBUFFER_HEIGHT                        = 0;
                    GLint   GL_MAX_FRAMEBUFFER_LAYERS                        = 0;
                    GLint   GL_MAX_FRAMEBUFFER_SAMPLES                       = 0;

                    //GL_ARB_shader_storage_buffer_object
                    GLint   GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS              = 0;
                    GLint   GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS            = 0;
                    GLint   GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS        = 0;
                    GLint   GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS     = 0;
                    GLint   GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS            = 0;
                    GLint   GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS             = 0;
                    GLint   GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS            = 0;
                    GLint   GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS            = 0;
                    GLint   GL_MAX_SHADER_STORAGE_BLOCK_SIZE                 = 0;
                    GLint   GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT        = 0;

                    //GL_KHR_debug
                    GLint   GL_MAX_DEBUG_MESSAGE_LENGTH                      = 0;
                    GLint   GL_MAX_DEBUG_LOGGED_MESSAGES                     = 0;
                    GLint   GL_MAX_DEBUG_GROUP_STACK_DEPTH                   = 0;
                    GLint   GL_MAX_LABEL_LENGTH                              = 0;

                //Not core
                    //GL_ARB_sparse_buffer
                    GLint   GL_SPARSE_BUFFER_PAGE_SIZE_ARB                   = 0;

                    //GL_EXT_texture_filter_anisotropic
                    GLint   GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT                = 0;
        };
    }
}
///\endcond
