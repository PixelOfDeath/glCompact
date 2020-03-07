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
#include "glCompact/glCompact.hpp"
#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"
#include "glCompact/PipelineRasterizationStateChangeInternal.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <glm/glm.hpp>

/**
    \defgroup API API
*/

using namespace glCompact::gl;

namespace glCompact {
    //control/flush commands
    /**
        @param oqoId
        @param waitMode Can be GL_QUERY_WAIT, GL_QUERY_NO_WAIT, GL_QUERY_BY_REGION_WAIT​, GL_QUERY_BY_REGION_NO_WAIT
    */
    /*void setCondition(GLuint oqoId, GLenum waitMode) {
        threadContext->glBeginConditionalRender(oqoId, waitMode);
    }

    void setCondition() {
        threadContext->glEndConditionalRender();
    }*/

    void flush() {
        //TODO: set all pending changes
        threadContextGroup->functions.glFlush();
    }

    void finish() {
        //TODO: set all pending changes
        threadContextGroup->functions.glFinish();
    }

    /**
        \ingroup API
        \brief Set target Frame object for PiepelineRasterization draw calls and blit operations

        \details

        To set the windows frame, use:

            setDrawFrame(getWindowFrame());
    */
    void setDrawFrame(
        Frame& frame
    ) {
        UNLIKELY_IF (frame.id == 0 && &frame != &threadContext->frameWindow)
            throw std::runtime_error("Trying to set empty Frame as drawFrame!");
        threadContext->pending_frame = &frame;
        threadContext->pipelineRasterizationStateChangePending |= PipelineRasterizationStateChange::viewportScissor;
    }

    /**
        \ingroup API
        \brief Get the current draw Frame of this context

        \details This is especially useful for code that draws on arbitary frames.

        E.g. to clear all RGBA targets of the current draw frame, use:

            getDrawFrame().clearRgba();
    */
    Frame& getDrawFrame() {
        return *threadContext->pending_frame;
    }

    /**
        \ingroup API
        \brief Set the size of the window/displayed frame.

        \details The size of the Windows frame is controlled by the underlaying window/widget library.
        They only way to get this information is via the specific callback or quere and feed the information back to glCompact so that viewport and/or scissor can bet set correctly.

        This function will set the viewportOffset and viewportSize to {0, 0} and {x, y}
    */
    void setWindowFrameSize(
        uint32_t x,
        uint32_t y
    ) {
        threadContext->frameWindow.x = x;
        threadContext->frameWindow.y = y;
        threadContext->frameWindow.viewportOffset = {0, 0};
        threadContext->frameWindow.viewportSize   = {x, y};
    }

    /**
        \ingroup API
        \brief Get the window/displayed frame.

        \details This gets a Frame object that represents the output window/display of OpenGL.

        It can be used to set it as the PiepelineRasterization target:

            getDrawFrame(getWindowFrame());
    */
    Frame& getWindowFrame() {
        return threadContext->frameWindow;
    }
}
