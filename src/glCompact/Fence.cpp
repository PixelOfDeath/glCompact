#include "glCompact/Fence.hpp"
#include "glCompact/ContextGroup_.hpp"
#include "glCompact/ThreadContextGroup_.hpp"
#include "glCompact/ToolsInternal.hpp"

/*
    GL_ARB_sync (Core since 3.2)
    GL_NV_fence

    OpenCL event <-> OpenGL sync object translation extensions
    cl_khr_gl_event and GL_ARB_cl_event?

    Without fences the only alternative is glFinish, and that is a performance killer!
*/

using namespace glCompact::gl;

namespace glCompact {
    /**
        \ingroup API
        \class glCompact::Fence

        \brief Fence object

        \details The Fence object can be inserted into the OpenGL command stream to later check completion of all previous issued commands.

        Unlike memory barriers, fences can be queried in other contexts/threads and even other APIs.

        Note that fences can only wait on all previous issued commands in the current thread/context. Not on specific commands!
        Therefor it can make sense to move background loading of assets into other threads/contexts. To have a sepperate stream
        of commands with their own fences.
    */
    Fence::~Fence() {
        free();
    }

    void Fence::insert() {
        free();
        fenceObj = threadContextGroup_->functions.glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    void Fence::free() {
        if (fenceObj) {
            threadContextGroup_->functions.glDeleteSync(fenceObj);
            fenceObj = 0;
            glClientWaitSyncDidFlush = false;
        }
    }

    /**
        \brief check if the fence is signaled
        @return true if the fence is signaled
    */
    bool Fence::isSignaled() {
        return isSignaledOrWait(0);
    }

    /**
        \brief check if the fence is signaled. If not wait a maximum of timeoutNanoseconds for it to become signaled.

        @param timeoutNanoseconds timeout in nanoseconds or the default  GL_TIMEOUT_IGNORED = uint64_t(~0) = 0xFFFFFFFFFFFFFFFF
        @return true if the fence is signaled
    */
    bool Fence::isSignaledOrWait(
        GLuint64 timeoutInNanoseconds
    ) {
        /*
            GL_ALREADY_SIGNALED    was signaled before call to glClientWaitSync
            GL_TIMEOUT_EXPIRED
            GL_CONDITION_SATISFIED was signaled after call to glClientWaitSync but before timeout
            GL_WAIT_FAILED         Not sure what to do with this one...

            TODO:
            Does GL_SYNC_FLUSH_COMMANDS_BIT have any effect if the fence was created by another context? How exactly did OpenGL 4.5 change fences?
        */
        int32_t result = threadContextGroup_->functions.glClientWaitSync(fenceObj, glClientWaitSyncDidFlush ? 0 : GL_SYNC_FLUSH_COMMANDS_BIT, timeoutInNanoseconds);
        glClientWaitSyncDidFlush = true;
        return (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED);
    }

    /**
        \brief This stalls the OpenGL command stream until the fence is signaled.

        Note: Use with caution, this can cause a context death lock!

        NOTE: There is no way to determine if a timeout happened or if the sync object is signaled. For that reason most implementations have a very large/effectively infinity GL_MAX_SERVER_WAIT_TIMEOUT value.

        Example:
            thread1/context1: Issue commands that change DATA, then syncObject.insert(), glFlush and then signals thread2
            thread2/context2: After signal from thread1 call syncObject.isSignaledOrStallCommandStream(), then directly start issuing commands that are going to use DATA

            OpenGL/OpenCL will make sure that the GL/CL commands from thread2 won't actually be issued until the sync object is reached.
    */
    void Fence::isSignaledOrStallCommandStream() {
        threadContextGroup_->functions.glWaitSync(fenceObj, 0, GL_TIMEOUT_IGNORED);
    }
}
