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
    #include "glCompact/ContextInternal.hpp"

//TODO use ATI_meminfo
//WGL_AMD_gpu_association
//GL_NVX_gpu_memory_info

//GLX_MESA_query_renderer //only memory size info I think
/*
WGL_AMD_gpu_association is an extension developed for parallel rendering: you can create a different OpenGL context on each available GPU. This extension comes with hardware query functionalities. With WGL_AMD_gpu_association you can get the amount of graphics memory available for the GPU.

GL_ATI_meminfo is used when you need more detailed information about the available memory for VBO(vertex buffer object) or for your textures.
*/

namespace glCompact {
    class Info {
            friend class contextInterface;
        public:
            Info();
            ~Info();

            /*enum vendorId
            {
                VENDOR_UNKNOWN = 0,
                VENDOR_ATI = 1,
                VENDOR_NVIDIA = 2
            };
            vendorId getVendor()
            {
                return VENDOR_UNKNOWN;
            }*/

            static uintptr_t vramSize;
            uintptr_t vramFree();

            static bool vendorAtiAmd;
            static bool vendorNvidia;
            static bool vendorMesa;
            //GLuint    const& id             = id_;
        private:
            //GLuint    id_;

            void setDefaultValues();
    };
}
