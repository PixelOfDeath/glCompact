/*
    glCompact
    Copyright (C) 2019-2020 Frederik Uje vom Hofe

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
