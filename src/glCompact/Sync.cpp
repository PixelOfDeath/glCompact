#include "glCompact/Sync.hpp"
//#include "glCompact/ThreadContext.hpp"
#include "glCompact/ThreadContextGroup.hpp"
#include "glCompact/ToolsInternal.hpp"

#include "glCompact/GlTools.hpp"

#include <stdexcept>

/*
    GL_ARB_sync (Core since 3.2)
    GL_NV_fence


    OpenCL event <-> OpenGL sync object translation extensions
    cl_khr_gl_event and GL_ARB_cl_event?

    Without there seems to be a need to use glFinish, a performance killer that would make per frame information interchange to costly?
*/

using namespace glCompact::gl;

namespace glCompact
{
    Sync::Sync() {
        syncObj = 0;
        glClientWaitSyncDidFlushAlready = false;
    }

    Sync::~Sync() {
        free();
    }

    void Sync::insert() {
        free();
        syncObj = threadContextGroup->functions.glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    void Sync::free() {
        if (syncObj) {
            threadContextGroup->functions.glDeleteSync(syncObj);
            syncObj = 0;
            glClientWaitSyncDidFlushAlready = false;
        }
    }

    /*
        TODO: do we need the GL_ALREADY_SIGNALED/GL_CONDITION_SATISFIED difference? Or can we just return a bool?


        @param flushIfUnsignaled if he sync object is still unsignaled when this function will cause a glFlush (TODO: Q: also the case if timeout is 0?)
        @param timeout timeout in nanoseconds or GL_TIMEOUT_IGNORED
        @return sync status
            GL_ALREADY_SIGNALED    (was signaled before call to glClientWaitSync)
            GL_TIMEOUT_EXPIRED
            GL_CONDITION_SATISFIED (was signaled after call to glClientWaitSync but before timeout)
            GL_WAIT_FAILED
    */
    bool Sync::waitForSync(
        bool flushIfUnsignaled,
        GLuint64 timeout
    ) {
        GLenum flags = 0;
        if (flushIfUnsignaled && !glClientWaitSyncDidFlushAlready) {
            glClientWaitSyncDidFlushAlready = true;
            flags = GL_SYNC_FLUSH_COMMANDS_BIT;
        }
        GLenum result = threadContextGroup->functions.glClientWaitSync(syncObj, flags, timeout);
        UNLIKELY_IF (result == GL_WAIT_FAILED)
            throw std::runtime_error("glClientWaitSync returned error (GL_WAIT_FAILED)");
        bool returnValue = (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED);
        return returnValue;
    }

    bool Sync::isSignaled() {
        if (!syncObj) return false;
        GLenum result = threadContextGroup->functions.glClientWaitSync(syncObj, 0, 0);
        return result == GL_ALREADY_SIGNALED; //probably don't need to test for GL_CONDITION_SATISFIED
    }
    /**
        This function is used to synchronise across context boundary's.

        It causes the GL server to block issues commands that come after this function and wait until sync becomes signalled or a timeout of GL_MAX_SERVER_WAIT_TIMEOUT (Nanoseconds? glGetInteger64v) happens

        NOTE: there is no way to determine if a timeout happened or if the sync object is signaled. For that reason most implementations have a very large/effectively infinity timeout value.

        radeonsi     =
        crimson/win7 = 0x7FFFFFFFFFFFFFFF

         should the sync obj be made sure to be flushed before calling this to prevent a death-lock until GL_MAX_SERVER_WAIT_TIMEOUT is over?

        Example:
            thread1/context1: Issue commands that change DATA, then syncObject.insert(), glFlush and then signals thread2
            thread2/context2: After signal from thread1 call syncObject.withholdFutureCommandsUntilSync(), then directly start issuing commands that are going to use DATA

            OpenGL/OpenCL will make sure that the GL/CL commands from thread2 won't actually be issued until the sync object is reached.
    */
    void Sync::withholdFutureCommandsUntilSync() {
        threadContextGroup->functions.glWaitSync(syncObj, 0, GL_TIMEOUT_IGNORED);
    }
}
