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
//#include "glCompact/glContext.hpp"

#include <cstdint> //C++11
//#include "glCompact/Global.hpp"

#include <glm/fwd.hpp>

#include <string>

namespace glCompact {
    //std::string glErrorToString(GLenum error);
    //void setFrustrumSquare(int x, int y);
    glm::mat4 getOrthoInsideSquare(int resX, int resY, float zNear, float zFar);
    //glm::mat4 getFrustumInsideSquare(float fov, int resX, int resY, float zNear, float zFar);
    glm::mat4 infinitePerspectiveReversed(float fovy, float aspect, float zNear);
    glm::dmat4 infinitePerspectiveReversed(double fovy, double aspect, double zNear);

    /**
        Reversed Infinite Projection Matrix (1=near, 0=far)

        Part of this from: http://timothylottes.blogspot.de/2014/07/infinite-projection-matrix-notes.html

        This matrix is for use with GL_ARB_clip_control (core in 4.5)
            glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE)
            glDepthFunc(GL_GREATER);
            glClearDepth(0.0);
    */

    /*glm::tmat4x4<T> infinitePerspectiveReversed(T fovy, T aspect, T zNear)
    {
        const T f = T(1) / tan(fovy / T(2)); // 1.0 / tan(X) == cotangent(X)
        glm::mat4 projectionMatrix =
        {
            f/aspect, T(0),       T(0),  T(0),
                T(0),    f,       T(0),  T(0),
                T(0), T(0),       T(0), T(-1),
                T(0), T(0), T(2)*zNear,  T(0)
        };
        return projectionMatrix;
    }*/

    /*template <typename T>
    GLM_FUNC_QUALIFIER tmat4x4<T, defaultp> infinitePerspective
    (
            T fovy,
            T aspect,
            T zNear
    )
    {
            T const range = tan(fovy / T(2)) * zNear;
            T const left = -range * aspect;
            T const right = range * aspect;
            T const bottom = -range;
            T const top = range;

            tmat4x4<T, defaultp> Result(T(0));
            Result[0][0] = (T(2) * zNear) / (right - left);
            Result[1][1] = (T(2) * zNear) / (top - bottom);
            Result[2][2] = - T(1);
            Result[2][3] = - T(1);
            Result[3][2] = - T(2) * zNear;
            return Result;
    }*/
}
