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
#include "glCompact/glCompact.hpp"
#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"
#include "glCompact/PipelineRasterizationStateChangeInternal.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <glm/glm.hpp>

using namespace glCompact::gl;

namespace glCompact {
    //control/flush commands
    /**
        @param oqoId
        @param waitMode Can be GL_QUERY_WAIT, GL_QUERY_NO_WAIT, GL_QUERY_BY_REGION_WAIT​, GL_QUERY_BY_REGION_NO_WAIT
    */
    /*void drawConditionBegin(GLuint oqoId, GLenum waitMode) {
        threadContext->glBeginConditionalRender(oqoId, waitMode);
    }

    void drawConditionEnd() {
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

    void setDrawFrame(
        Frame& frame
    ) {
        UNLIKELY_IF (frame.id == 0 && &frame != &threadContext->frameWindow)
            throw std::runtime_error("Trying to set empty Frame as drawFrame!");
        threadContext->pending_frame = &frame;
        threadContext->pipelineRasterizationStateChangePending += PipelineRasterizationStateChange::viewportScissor;
    }

    Frame& getDrawFrame() {
        return *threadContext->pending_frame;
    }

    //This function also will reset the viewportOffset and viewportSize to {0, 0} and {x, y}
    void setWindowFrameSize(
        uint32_t x,
        uint32_t y
    ) {
        threadContext->frameWindow.x = x;
        threadContext->frameWindow.y = y;
        threadContext->frameWindow.viewportOffset = {0, 0};
        threadContext->frameWindow.viewportSize   = {x, y};
    }

    Frame& getWindowFrame() {
        return threadContext->frameWindow;
    }

    /*void setVao(const VAO* vao) {
        threadContext->pending_vaoId = vao ? vao->vaoId : threadContext->default_vaoId;
    }*/



}
