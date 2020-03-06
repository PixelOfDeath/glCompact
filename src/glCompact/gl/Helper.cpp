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
#include "glCompact/gl/Helper.hpp"
#include "glCompact/gl/Constants.hpp"
#include <stdexcept>

using namespace std;

namespace glCompact {
    namespace gl {
        std::string typeToGlslAndCNameString(
            int32_t type
        ) {
            switch (type) {
                case GL_FLOAT                                     : return "float";
                case GL_FLOAT_VEC2                                : return "vec2";
                case GL_FLOAT_VEC3                                : return "vec3";
                case GL_FLOAT_VEC4                                : return "vec4";
                case GL_DOUBLE                                    : return "double";
                case GL_DOUBLE_VEC2                               : return "dvec2";
                case GL_DOUBLE_VEC3                               : return "dvec3";
                case GL_DOUBLE_VEC4                               : return "dvec4";
                case GL_INT                                       : return "int";
                case GL_INT_VEC2                                  : return "ivec2";
                case GL_INT_VEC3                                  : return "ivec3";
                case GL_INT_VEC4                                  : return "ivec4";
                case GL_UNSIGNED_INT                              : return "uint";
                case GL_UNSIGNED_INT_VEC2                         : return "uvec2";
                case GL_UNSIGNED_INT_VEC3                         : return "uvec3";
                case GL_UNSIGNED_INT_VEC4                         : return "uvec4";
                case GL_BOOL                                      : return "bool";
                case GL_BOOL_VEC2                                 : return "bvec2";
                case GL_BOOL_VEC3                                 : return "bvec3";
                case GL_BOOL_VEC4                                 : return "bvec4";
                case GL_FLOAT_MAT2                                : return "mat2";
                case GL_FLOAT_MAT3                                : return "mat3";
                case GL_FLOAT_MAT4                                : return "mat4";
                case GL_FLOAT_MAT2x3                              : return "mat2x3";
                case GL_FLOAT_MAT2x4                              : return "mat2x4";
                case GL_FLOAT_MAT3x2                              : return "mat3x2";
                case GL_FLOAT_MAT3x4                              : return "mat3x4";
                case GL_FLOAT_MAT4x2                              : return "mat4x2";
                case GL_FLOAT_MAT4x3                              : return "mat4x3";
                case GL_DOUBLE_MAT2                               : return "dmat2";
                case GL_DOUBLE_MAT3                               : return "dmat3";
                case GL_DOUBLE_MAT4                               : return "dmat4";
                case GL_DOUBLE_MAT2x3                             : return "dmat2x3";
                case GL_DOUBLE_MAT2x4                             : return "dmat2x4";
                case GL_DOUBLE_MAT3x2                             : return "dmat3x2";
                case GL_DOUBLE_MAT3x4                             : return "dmat3x4";
                case GL_DOUBLE_MAT4x2                             : return "dmat4x2";
                case GL_DOUBLE_MAT4x3                             : return "dmat4x3";
                case GL_SAMPLER_1D                                : return "sampler1D";
                case GL_SAMPLER_2D                                : return "sampler2D";
                case GL_SAMPLER_3D                                : return "sampler3D";
                case GL_SAMPLER_CUBE                              : return "samplerCube";
                case GL_SAMPLER_1D_SHADOW                         : return "sampler1DShadow";
                case GL_SAMPLER_2D_SHADOW                         : return "sampler2DShadow";
                case GL_SAMPLER_1D_ARRAY                          : return "sampler1DArray";
                case GL_SAMPLER_2D_ARRAY                          : return "sampler2DArray";
                case GL_SAMPLER_1D_ARRAY_SHADOW                   : return "sampler1DArrayShadow";
                case GL_SAMPLER_2D_ARRAY_SHADOW                   : return "sampler2DArrayShadow";
                case GL_SAMPLER_2D_MULTISAMPLE                    : return "sampler2DMS";
                case GL_SAMPLER_2D_MULTISAMPLE_ARRAY              : return "sampler2DMSArray";
                case GL_SAMPLER_CUBE_SHADOW                       : return "samplerCubeShadow";
                case GL_SAMPLER_BUFFER                            : return "samplerBuffer";
                case GL_SAMPLER_2D_RECT                           : return "sampler2DRect";
                case GL_SAMPLER_2D_RECT_SHADOW                    : return "sampler2DRectShadow";
                case GL_INT_SAMPLER_1D                            : return "isampler1D";
                case GL_INT_SAMPLER_2D                            : return "isampler2D";
                case GL_INT_SAMPLER_3D                            : return "isampler3D";
                case GL_INT_SAMPLER_CUBE                          : return "isamplerCube";
                case GL_INT_SAMPLER_1D_ARRAY                      : return "isampler1DArray";
                case GL_INT_SAMPLER_2D_ARRAY                      : return "isampler2DArray";
                case GL_INT_SAMPLER_2D_MULTISAMPLE                : return "isampler2DMS";
                case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY          : return "isampler2DMSArray";
                case GL_INT_SAMPLER_BUFFER                        : return "isamplerBuffer";
                case GL_INT_SAMPLER_2D_RECT                       : return "isampler2DRect";
                case GL_UNSIGNED_INT_SAMPLER_1D                   : return "usampler1D";
                case GL_UNSIGNED_INT_SAMPLER_2D                   : return "usampler2D";
                case GL_UNSIGNED_INT_SAMPLER_3D                   : return "usampler3D";
                case GL_UNSIGNED_INT_SAMPLER_CUBE                 : return "usamplerCube";
                case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY             : return "usampler1DArray";
                case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY             : return "usampler2DArray";
                case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE       : return "usampler2DMS";
                case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY : return "usampler2DMSArray";
                case GL_UNSIGNED_INT_SAMPLER_BUFFER               : return "usamplerBuffer";
                case GL_UNSIGNED_INT_SAMPLER_2D_RECT              : return "usampler2DRect";
                case GL_IMAGE_1D                                  : return "image1D";
                case GL_IMAGE_2D                                  : return "image2D";
                case GL_IMAGE_3D                                  : return "image3D";
                case GL_IMAGE_2D_RECT                             : return "image2DRect";
                case GL_IMAGE_CUBE                                : return "imageCube";
                case GL_IMAGE_BUFFER                              : return "imageBuffer";
                case GL_IMAGE_1D_ARRAY                            : return "image1DArray";
                case GL_IMAGE_2D_ARRAY                            : return "image2DArray";
                case GL_IMAGE_2D_MULTISAMPLE                      : return "image2DMS";
                case GL_IMAGE_2D_MULTISAMPLE_ARRAY                : return "image2DMSArray";
                case GL_INT_IMAGE_1D                              : return "iimage1D";
                case GL_INT_IMAGE_2D                              : return "iimage2D";
                case GL_INT_IMAGE_3D                              : return "iimage3D";
                case GL_INT_IMAGE_2D_RECT                         : return "iimage2DRect";
                case GL_INT_IMAGE_CUBE                            : return "iimageCube";
                case GL_INT_IMAGE_BUFFER                          : return "iimageBuffer";
                case GL_INT_IMAGE_1D_ARRAY                        : return "iimage1DArray";
                case GL_INT_IMAGE_2D_ARRAY                        : return "iimage2DArray";
                case GL_INT_IMAGE_2D_MULTISAMPLE                  : return "iimage2DMS";
                case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY            : return "iimage2DMSArray";
                case GL_UNSIGNED_INT_IMAGE_1D                     : return "uimage1D";
                case GL_UNSIGNED_INT_IMAGE_2D                     : return "uimage2D";
                case GL_UNSIGNED_INT_IMAGE_3D                     : return "uimage3D";
                case GL_UNSIGNED_INT_IMAGE_2D_RECT                : return "uimage2DRect";
                case GL_UNSIGNED_INT_IMAGE_CUBE                   : return "uimageCube";
                case GL_UNSIGNED_INT_IMAGE_BUFFER                 : return "uimageBuffer";
                case GL_UNSIGNED_INT_IMAGE_1D_ARRAY               : return "uimage1DArray";
                case GL_UNSIGNED_INT_IMAGE_2D_ARRAY               : return "uimage2DArray";
                case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE         : return "uimage2DMS";
                case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY   : return "uimage2DMSArray";
                case GL_UNSIGNED_INT_ATOMIC_COUNTER               : return "atomic_uint";
                default: throw runtime_error("unknown type (" + to_string(type) + ")");
            }
        }

        std::string typeTopConstantNameString(
            int32_t type
        ) {
            switch (type) {
                case GL_FLOAT                                     : return "GL_FLOAT";
                case GL_FLOAT_VEC2                                : return "GL_FLOAT_VEC2";
                case GL_FLOAT_VEC3                                : return "GL_FLOAT_VEC3";
                case GL_FLOAT_VEC4                                : return "GL_FLOAT_VEC4";
                case GL_DOUBLE                                    : return "GL_DOUBLE";
                case GL_DOUBLE_VEC2                               : return "GL_DOUBLE_VEC2";
                case GL_DOUBLE_VEC3                               : return "GL_DOUBLE_VEC3";
                case GL_DOUBLE_VEC4                               : return "GL_DOUBLE_VEC4";
                case GL_INT                                       : return "GL_INT";
                case GL_INT_VEC2                                  : return "GL_INT_VEC2";
                case GL_INT_VEC3                                  : return "GL_INT_VEC3";
                case GL_INT_VEC4                                  : return "GL_INT_VEC4";
                case GL_UNSIGNED_INT                              : return "GL_UNSIGNED_INT";
                case GL_UNSIGNED_INT_VEC2                         : return "GL_UNSIGNED_INT_VEC2";
                case GL_UNSIGNED_INT_VEC3                         : return "GL_UNSIGNED_INT_VEC3";
                case GL_UNSIGNED_INT_VEC4                         : return "GL_UNSIGNED_INT_VEC4";
                case GL_BOOL                                      : return "GL_BOOL";
                case GL_BOOL_VEC2                                 : return "GL_BOOL_VEC2";
                case GL_BOOL_VEC3                                 : return "GL_BOOL_VEC3";
                case GL_BOOL_VEC4                                 : return "GL_BOOL_VEC4";
                case GL_FLOAT_MAT2                                : return "GL_FLOAT_MAT2";
                case GL_FLOAT_MAT3                                : return "GL_FLOAT_MAT3";
                case GL_FLOAT_MAT4                                : return "GL_FLOAT_MAT4";
                case GL_FLOAT_MAT2x3                              : return "GL_FLOAT_MAT2x3";
                case GL_FLOAT_MAT2x4                              : return "GL_FLOAT_MAT2x4";
                case GL_FLOAT_MAT3x2                              : return "GL_FLOAT_MAT3x2";
                case GL_FLOAT_MAT3x4                              : return "GL_FLOAT_MAT3x4";
                case GL_FLOAT_MAT4x2                              : return "GL_FLOAT_MAT4x2";
                case GL_FLOAT_MAT4x3                              : return "GL_FLOAT_MAT4x3";
                case GL_DOUBLE_MAT2                               : return "GL_DOUBLE_MAT2";
                case GL_DOUBLE_MAT3                               : return "GL_DOUBLE_MAT3";
                case GL_DOUBLE_MAT4                               : return "GL_DOUBLE_MAT4";
                case GL_DOUBLE_MAT2x3                             : return "GL_DOUBLE_MAT2x3";
                case GL_DOUBLE_MAT2x4                             : return "GL_DOUBLE_MAT2x4";
                case GL_DOUBLE_MAT3x2                             : return "GL_DOUBLE_MAT3x2";
                case GL_DOUBLE_MAT3x4                             : return "GL_DOUBLE_MAT3x4";
                case GL_DOUBLE_MAT4x2                             : return "GL_DOUBLE_MAT4x2";
                case GL_DOUBLE_MAT4x3                             : return "GL_DOUBLE_MAT4x3";
                case GL_SAMPLER_1D                                : return "GL_SAMPLER_1D";
                case GL_SAMPLER_2D                                : return "GL_SAMPLER_2D";
                case GL_SAMPLER_3D                                : return "GL_SAMPLER_3D";
                case GL_SAMPLER_CUBE                              : return "GL_SAMPLER_CUBE";
                case GL_SAMPLER_1D_SHADOW                         : return "GL_SAMPLER_1D_SHADOW";
                case GL_SAMPLER_2D_SHADOW                         : return "GL_SAMPLER_2D_SHADOW";
                case GL_SAMPLER_1D_ARRAY                          : return "GL_SAMPLER_1D_ARRAY";
                case GL_SAMPLER_2D_ARRAY                          : return "GL_SAMPLER_2D_ARRAY";
                case GL_SAMPLER_1D_ARRAY_SHADOW                   : return "GL_SAMPLER_1D_ARRAY_SHADOW";
                case GL_SAMPLER_2D_ARRAY_SHADOW                   : return "GL_SAMPLER_2D_ARRAY_SHADOW";
                case GL_SAMPLER_2D_MULTISAMPLE                    : return "GL_SAMPLER_2D_MULTISAMPLE";
                case GL_SAMPLER_2D_MULTISAMPLE_ARRAY              : return "GL_SAMPLER_2D_MULTISAMPLE_ARRAY";
                case GL_SAMPLER_CUBE_SHADOW                       : return "GL_SAMPLER_CUBE_SHADOW";
                case GL_SAMPLER_BUFFER                            : return "GL_SAMPLER_BUFFER";
                case GL_SAMPLER_2D_RECT                           : return "GL_SAMPLER_2D_RECT";
                case GL_SAMPLER_2D_RECT_SHADOW                    : return "GL_SAMPLER_2D_RECT_SHADOW";
                case GL_INT_SAMPLER_1D                            : return "GL_INT_SAMPLER_1D";
                case GL_INT_SAMPLER_2D                            : return "GL_INT_SAMPLER_2D";
                case GL_INT_SAMPLER_3D                            : return "GL_INT_SAMPLER_3D";
                case GL_INT_SAMPLER_CUBE                          : return "GL_INT_SAMPLER_CUBE";
                case GL_INT_SAMPLER_1D_ARRAY                      : return "GL_INT_SAMPLER_1D_ARRAY";
                case GL_INT_SAMPLER_2D_ARRAY                      : return "GL_INT_SAMPLER_2D_ARRAY";
                case GL_INT_SAMPLER_2D_MULTISAMPLE                : return "GL_INT_SAMPLER_2D_MULTISAMPLE";
                case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY          : return "GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY";
                case GL_INT_SAMPLER_BUFFER                        : return "GL_INT_SAMPLER_BUFFER";
                case GL_INT_SAMPLER_2D_RECT                       : return "GL_INT_SAMPLER_2D_RECT";
                case GL_UNSIGNED_INT_SAMPLER_1D                   : return "GL_UNSIGNED_INT_SAMPLER_1D";
                case GL_UNSIGNED_INT_SAMPLER_2D                   : return "GL_UNSIGNED_INT_SAMPLER_2D";
                case GL_UNSIGNED_INT_SAMPLER_3D                   : return "GL_UNSIGNED_INT_SAMPLER_3D";
                case GL_UNSIGNED_INT_SAMPLER_CUBE                 : return "GL_UNSIGNED_INT_SAMPLER_CUBE";
                case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY             : return "GL_UNSIGNED_INT_SAMPLER_1D_ARRAY";
                case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY             : return "GL_UNSIGNED_INT_SAMPLER_2D_ARRAY";
                case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE       : return "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE";
                case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY : return "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY";
                case GL_UNSIGNED_INT_SAMPLER_BUFFER               : return "GL_UNSIGNED_INT_SAMPLER_BUFFER";
                case GL_UNSIGNED_INT_SAMPLER_2D_RECT              : return "GL_UNSIGNED_INT_SAMPLER_2D_RECT";
                case GL_IMAGE_1D                                  : return "GL_IMAGE_1D";
                case GL_IMAGE_2D                                  : return "GL_IMAGE_2D";
                case GL_IMAGE_3D                                  : return "GL_IMAGE_3D";
                case GL_IMAGE_2D_RECT                             : return "GL_IMAGE_2D_RECT";
                case GL_IMAGE_CUBE                                : return "GL_IMAGE_CUBE";
                case GL_IMAGE_BUFFER                              : return "GL_IMAGE_BUFFER";
                case GL_IMAGE_1D_ARRAY                            : return "GL_IMAGE_1D_ARRAY";
                case GL_IMAGE_2D_ARRAY                            : return "GL_IMAGE_2D_ARRAY";
                case GL_IMAGE_2D_MULTISAMPLE                      : return "GL_IMAGE_2D_MULTISAMPLE";
                case GL_IMAGE_2D_MULTISAMPLE_ARRAY                : return "GL_IMAGE_2D_MULTISAMPLE_ARRAY";
                case GL_INT_IMAGE_1D                              : return "GL_INT_IMAGE_1D";
                case GL_INT_IMAGE_2D                              : return "GL_INT_IMAGE_2D";
                case GL_INT_IMAGE_3D                              : return "GL_INT_IMAGE_3D";
                case GL_INT_IMAGE_2D_RECT                         : return "GL_INT_IMAGE_2D_RECT";
                case GL_INT_IMAGE_CUBE                            : return "GL_INT_IMAGE_CUBE";
                case GL_INT_IMAGE_BUFFER                          : return "GL_INT_IMAGE_BUFFER";
                case GL_INT_IMAGE_1D_ARRAY                        : return "GL_INT_IMAGE_1D_ARRAY";
                case GL_INT_IMAGE_2D_ARRAY                        : return "GL_INT_IMAGE_2D_ARRAY";
                case GL_INT_IMAGE_2D_MULTISAMPLE                  : return "GL_INT_IMAGE_2D_MULTISAMPLE";
                case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY            : return "GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY";
                case GL_UNSIGNED_INT_IMAGE_1D                     : return "GL_UNSIGNED_INT_IMAGE_1D";
                case GL_UNSIGNED_INT_IMAGE_2D                     : return "GL_UNSIGNED_INT_IMAGE_2D";
                case GL_UNSIGNED_INT_IMAGE_3D                     : return "GL_UNSIGNED_INT_IMAGE_3D";
                case GL_UNSIGNED_INT_IMAGE_2D_RECT                : return "GL_UNSIGNED_INT_IMAGE_2D_RECT";
                case GL_UNSIGNED_INT_IMAGE_CUBE                   : return "GL_UNSIGNED_INT_IMAGE_CUBE";
                case GL_UNSIGNED_INT_IMAGE_BUFFER                 : return "GL_UNSIGNED_INT_IMAGE_BUFFER";
                case GL_UNSIGNED_INT_IMAGE_1D_ARRAY               : return "GL_UNSIGNED_INT_IMAGE_1D_ARRAY";
                case GL_UNSIGNED_INT_IMAGE_2D_ARRAY               : return "GL_UNSIGNED_INT_IMAGE_2D_ARRAY";
                case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE         : return "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE";
                case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY   : return "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY";
                case GL_UNSIGNED_INT_ATOMIC_COUNTER               : return "GL_UNSIGNED_INT_ATOMIC_COUNTER";
                default: throw runtime_error("unknown type (" + to_string(type) + ")");
            }
        }

        int32_t typeToBaseType(
            int32_t type
        ) {
            switch (type) {
                case GL_FLOAT:
                case GL_FLOAT_VEC2:
                case GL_FLOAT_VEC3:
                case GL_FLOAT_VEC4:
                case GL_FLOAT_MAT2:
                case GL_FLOAT_MAT3:
                case GL_FLOAT_MAT4:
                case GL_FLOAT_MAT2x3:
                case GL_FLOAT_MAT2x4:
                case GL_FLOAT_MAT3x2:
                case GL_FLOAT_MAT3x4:
                case GL_FLOAT_MAT4x2:
                case GL_FLOAT_MAT4x3:
                    return GL_FLOAT;
                case GL_DOUBLE:
                case GL_DOUBLE_VEC2:
                case GL_DOUBLE_VEC3:
                case GL_DOUBLE_VEC4:
                case GL_DOUBLE_MAT2:
                case GL_DOUBLE_MAT3:
                case GL_DOUBLE_MAT4:
                case GL_DOUBLE_MAT2x3:
                case GL_DOUBLE_MAT2x4:
                case GL_DOUBLE_MAT3x2:
                case GL_DOUBLE_MAT3x4:
                case GL_DOUBLE_MAT4x2:
                case GL_DOUBLE_MAT4x3:
                    return GL_DOUBLE;
                case GL_INT:
                case GL_INT_VEC2:
                case GL_INT_VEC3:
                case GL_INT_VEC4:
                    return GL_INT;
                case GL_UNSIGNED_INT:
                case GL_UNSIGNED_INT_VEC2:
                case GL_UNSIGNED_INT_VEC3:
                case GL_UNSIGNED_INT_VEC4:
                    return GL_UNSIGNED_INT;
                case GL_BOOL:
                case GL_BOOL_VEC2:
                case GL_BOOL_VEC3:
                case GL_BOOL_VEC4:
                    return GL_BOOL;
                default: throw runtime_error("unknown type (" + to_string(type) + ")");
            }
        }

        int32_t typeToBaseTypeCount(
            int32_t type
        ) {
            switch (type) {
                case GL_FLOAT:
                case GL_DOUBLE:
                case GL_INT:
                case GL_UNSIGNED_INT:
                case GL_BOOL:
                    return 1;
                case GL_FLOAT_VEC2:
                case GL_DOUBLE_VEC2:
                case GL_INT_VEC2:
                case GL_UNSIGNED_INT_VEC2:
                case GL_BOOL_VEC2:
                    return 2;
                case GL_FLOAT_VEC3:
                case GL_DOUBLE_VEC3:
                case GL_INT_VEC3:
                case GL_UNSIGNED_INT_VEC3:
                case GL_BOOL_VEC3:
                    return 3;
                case GL_FLOAT_VEC4:
                case GL_DOUBLE_VEC4:
                case GL_INT_VEC4:
                case GL_UNSIGNED_INT_VEC4:
                case GL_BOOL_VEC4:
                case GL_FLOAT_MAT2:
                case GL_DOUBLE_MAT2:
                    return 4;
                case GL_FLOAT_MAT2x3:
                case GL_FLOAT_MAT3x2:
                case GL_DOUBLE_MAT2x3:
                case GL_DOUBLE_MAT3x2:
                    return 6;
                case GL_FLOAT_MAT2x4:
                case GL_FLOAT_MAT4x2:
                case GL_DOUBLE_MAT2x4:
                case GL_DOUBLE_MAT4x2:
                    return 8;
                case GL_FLOAT_MAT3:
                case GL_DOUBLE_MAT3:
                    return 9;
                case GL_FLOAT_MAT3x4:
                case GL_FLOAT_MAT4x3:
                case GL_DOUBLE_MAT3x4:
                case GL_DOUBLE_MAT4x3:
                    return 12;
                case GL_FLOAT_MAT4:
                case GL_DOUBLE_MAT4:
                    return 16;
                default: throw runtime_error("unknown type (" + to_string(type) + ")");
            }
        }

        int32_t baseTypeToSize(
            int32_t type
        ) {
            switch (type) {
                case GL_BYTE:
                case GL_UNSIGNED_BYTE:
                    return 1;
                case GL_SHORT:
                case GL_UNSIGNED_SHORT:
                case GL_HALF_FLOAT:
                    return 2;
                case GL_FLOAT:
                case GL_INT:
                case GL_UNSIGNED_INT:
                case GL_BOOL:
                case GL_FIXED:
                    return 4;
                case GL_DOUBLE:
                    return 8;
                default: throw runtime_error("unknown type (" + to_string(type) + ")");
            }
        }

        int32_t getTypeSize(
            int32_t type
        ) {
            return typeToBaseTypeCount(type) * baseTypeToSize(typeToBaseType(type));
        }

        bool typeIsSampler(
            int32_t type
        ) {
            switch (type) {
                case GL_SAMPLER_1D:
                case GL_SAMPLER_2D:
                case GL_SAMPLER_3D:
                case GL_SAMPLER_CUBE:
                case GL_SAMPLER_1D_SHADOW:
                case GL_SAMPLER_2D_SHADOW:
                case GL_SAMPLER_1D_ARRAY:
                case GL_SAMPLER_2D_ARRAY:
                case GL_SAMPLER_1D_ARRAY_SHADOW:
                case GL_SAMPLER_2D_ARRAY_SHADOW:
                case GL_SAMPLER_2D_MULTISAMPLE:
                case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
                case GL_SAMPLER_CUBE_SHADOW:
                case GL_SAMPLER_BUFFER:
                case GL_SAMPLER_2D_RECT:
                case GL_SAMPLER_2D_RECT_SHADOW:
                case GL_INT_SAMPLER_1D:
                case GL_INT_SAMPLER_2D:
                case GL_INT_SAMPLER_3D:
                case GL_INT_SAMPLER_CUBE:
                case GL_INT_SAMPLER_1D_ARRAY:
                case GL_INT_SAMPLER_2D_ARRAY:
                case GL_INT_SAMPLER_2D_MULTISAMPLE:
                case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
                case GL_INT_SAMPLER_BUFFER:
                case GL_INT_SAMPLER_2D_RECT:
                case GL_UNSIGNED_INT_SAMPLER_1D:
                case GL_UNSIGNED_INT_SAMPLER_2D:
                case GL_UNSIGNED_INT_SAMPLER_3D:
                case GL_UNSIGNED_INT_SAMPLER_CUBE:
                case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
                case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
                case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
                case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
                case GL_UNSIGNED_INT_SAMPLER_BUFFER:
                case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
                    return true;
            }
            return false;
        }

        bool typeIsImage(
            int32_t type
        ) {
            switch (type) {
                case GL_IMAGE_1D:
                case GL_IMAGE_2D:
                case GL_IMAGE_3D:
                case GL_IMAGE_2D_RECT:
                case GL_IMAGE_CUBE:
                case GL_IMAGE_BUFFER:
                case GL_IMAGE_1D_ARRAY:
                case GL_IMAGE_2D_ARRAY:
                case GL_IMAGE_2D_MULTISAMPLE:
                case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
                case GL_INT_IMAGE_1D:
                case GL_INT_IMAGE_2D:
                case GL_INT_IMAGE_3D:
                case GL_INT_IMAGE_2D_RECT:
                case GL_INT_IMAGE_CUBE:
                case GL_INT_IMAGE_BUFFER:
                case GL_INT_IMAGE_1D_ARRAY:
                case GL_INT_IMAGE_2D_ARRAY:
                case GL_INT_IMAGE_2D_MULTISAMPLE:
                case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
                case GL_UNSIGNED_INT_IMAGE_1D:
                case GL_UNSIGNED_INT_IMAGE_2D:
                case GL_UNSIGNED_INT_IMAGE_3D:
                case GL_UNSIGNED_INT_IMAGE_2D_RECT:
                case GL_UNSIGNED_INT_IMAGE_CUBE:
                case GL_UNSIGNED_INT_IMAGE_BUFFER:
                case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
                case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
                case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
                case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
                    return true;
            }
            return false;
        }

        std::string errorToString(
            int32_t error
        ) {
            switch (error) {
                case GL_NO_ERROR:                       return "no error";
                case GL_INVALID_ENUM:                   return "invalid enum";
                case GL_INVALID_VALUE:                  return "invalid value";
                case GL_INVALID_OPERATION:              return "invalid operation";
                case GL_INVALID_FRAMEBUFFER_OPERATION:  return "invalid framebuffer operation. Framebuffer not complete";
                case GL_OUT_OF_MEMORY:                  return "out of memory";
                case GL_STACK_UNDERFLOW:                return "stack underflow";
                case GL_STACK_OVERFLOW:                 return "stack overflow";
                default: throw runtime_error("unknown error (" + to_string(error) + ")");
            }
        }
    }
}
