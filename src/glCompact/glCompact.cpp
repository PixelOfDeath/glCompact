#include "glCompact/glCompact.hpp"
#include "glCompact/threadContext_.hpp"
#include "glCompact/threadContextGroup_.hpp"
#include "glCompact/PipelineRasterizationStateChangeInternal.hpp"
#include "glCompact/ToolsInternal.hpp"

#include <glm/glm.hpp>

using namespace std;
using namespace glCompact::gl;

/**
    \defgroup API API
    \brief This is user facing API of the library
*/

namespace glCompact {
    /*
        @param oqoId
        @param waitMode Can be GL_QUERY_WAIT, GL_QUERY_NO_WAIT, GL_QUERY_BY_REGION_WAIT​, GL_QUERY_BY_REGION_NO_WAIT
    */
    /*void setCondition(GLuint oqoId, GLenum waitMode) {
        threadContext->glBeginConditionalRender(oqoId, waitMode);
    }

    void setCondition() {
        threadContext->glEndConditionalRender();
    }*/

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
        UNLIKELY_IF (frame.id == 0 && &frame != &threadContext_->frameWindow)
            throw runtime_error("Trying to set empty Frame as drawFrame!");
        threadContext_->pending_frame = &frame;
        threadContext_->pipelineRasterizationStateChangePending |= PipelineRasterizationStateChange::viewportScissor;
    }

    /**
        \ingroup API
        \brief Set target Frame object for PiepelineRasterization draw calls and blit operations to NULL
    */
    void setDrawFrame() {
        threadContext_->pending_frame = NULL;
    }

    /**
        \ingroup API
        \brief Get the current draw Frame of this context

        \details This is especially useful for code that draws on arbitary frames.

        E.g. to clear all RGBA targets of the current draw frame, use:

            getDrawFrame().clearRgba();
    */
    Frame& getDrawFrame() {
        UNLIKELY_IF (!threadContext_->pending_frame)
            throw runtime_error("No draw frame set!");
        return *threadContext_->pending_frame;
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
        UNLIKELY_IF (!threadContext_->isMainContext)
            throw runtime_error("Not the main context, only the main context can access to the drawFrame!");
        threadContext_->frameWindow.size.x = x;
        threadContext_->frameWindow.size.y = y;
        threadContext_->frameWindow.viewportOffset = {0, 0};
        threadContext_->frameWindow.viewportSize   = {x, y};
    }

    /**
        \ingroup API
        \brief Get the window/displayed frame.

        \details This gets a Frame object that represents the output window/display of OpenGL.

        It can be used to set it as the PiepelineRasterization target:

            getDrawFrame(getWindowFrame());
    */
    Frame& getWindowFrame() {
        UNLIKELY_IF (!threadContext_->isMainContext)
            throw runtime_error("Not the main context, only the main context can access to the drawFrame!");
        return threadContext_->frameWindow;
    }

    //transform feedback
    //If I implement this it maybe will be part of PipelineRasterization or its own object.
    //void setTransformFeedbackLayout(const TransformFeedbackLayout& layout);
    //void setTransformFeedbackBuffer(uint32_t slot, BufferInterface& buffer);

    void flush() {
        //TODO: set all pending changes
        threadContext_->processPendingChangesMemoryBarriers();
        threadContextGroup_->functions.glFlush();
    }

    void finish() {
        //TODO: set all pending changes
        threadContext_->processPendingChangesMemoryBarriers();
        threadContextGroup_->functions.glFinish();
    }
}
