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
#pragma once

/*
    3.3 spec: "The vertices are fed back after vertex color clamping, but before flatshading and clipping."
    POINTS   : POINTS
    LINES    : LINES, LINE_LOOP, LINE_STRIP
    TRIANGLES: TRIANGLES, TRIANGLE_STRIP, TRIANGLE_FAN



    GL_EXT_transform_feedback

    Transform Feedback is part of the core 3.0. But it has many limitations!

        GL_SEPARATE_ATTRIBS
            GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS    (min. 4)
            GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS       (min. 4)
        GL_INTERLEAVED_ATTRIBS
            GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS (min. 64 (double counts as 2))
        offset for buffer binding must be multiple of 4!

    ARB_transform_feedback2               (Core since 4.0)
    ARB_transform_feedback3               (Core since 4.0) (Advanced Interleaving)
    ARB_transform_feedback_instanced      (Core since 4.2)
    ARB_enhanced_layouts                  (Core since 4.4)
    ARB_transform_feedback_overflow_query (Core since 4.6)

    EXT_transform_feedback
    NV_transform_feedback

    ARB_gpu_shader_fp64 (Core since 4.0) (Double-precision Alignment)


    glGenTransformFeedbacks +bind / glCreateTransformFeedbacks
    glBindBuffer / glTransformFeedbackBuffer


    GL_MAX_TRANSFORM_FEEDBACK_BUFFERS






    Only primitive output: GL_TRIANGLES, GL_LINES, GL_POINTS
    GL_POINTS    <- GL_POINTS
    GL_LINES     <- GL_LINES, GL_LINE_LOOP, GL_LINE_STRIP
    GL_TRIANGLES <- GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUADS, GL_QUAD_STRIP, GL_POLYGON

    Output is always 32bit integer or floating point.
    Only this data types : GLfloat​, GLdouble​, GLint​, GLuint​

    Date source by highest preference: geometry shader, tessellation shader, vertex shader, draw command type (the last two !?)
 */

#include "glCompact/ContextInternal.hpp"

namespace glCompact {
    class TransformFeedback {
        //friend class ContextInterface;
    };

    class TransformFeedbackLayout {
        //friend class ContextInterface;
        void addBufferIndex();
        void addOutputType(int index, GLenum type);
        void finalize();

        void clear();
    };
}
