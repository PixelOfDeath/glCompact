#include <SDL.h>
#include <glCompact/glCompact.hpp>
#include <iostream>

void crash(std::string s){
    std::cout << s << std::endl;
    exit(-1);
}

class FrameworkWindow {
        friend class FrameworkMain;
    public:
        FrameworkWindow(int glMayor, int glMinor, bool gles, int sizeX, int sizeY) {
            this->sizeX = sizeX;
            this->sizeY = sizeY;
            if (gles) {
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glMayor);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glMinor);
            } else {
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glMayor);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glMinor);
            }
            window = SDL_CreateWindow(
                "glCompact SDL2 example",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                sizeX,
                sizeY,
                SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
            );
            if (!window)
                crash("SDL2 could not create window: " + std::string(SDL_GetError()));
            SDL_SetWindowData(window, "this", this);

            glContext = SDL_GL_CreateContext(window);
            if (!glContext)
                crash("SDL2 could not create OpenGL context: " + std::string(SDL_GetError()));
        }
        ~FrameworkWindow() {
            SDL_GL_DeleteContext(glContext);
            SDL_DestroyWindow(window);
        }
        void swap() {
            SDL_GL_SwapWindow(window);
        }
        int getSizeX()const {
            return sizeX;
        }
        int getSizeY()const {
            return sizeY;
        }
        bool isClosing()const {
            return closing;
        }

        SDL_Window*   window;
        SDL_GLContext glContext;
    private:
        int sizeX;
        int sizeY;
        bool closing = false;

        void resizeCallback(int x, int y) {
            sizeX = x;
            sizeY = y;
            glCompact::setDisplayFrameSize(x, y);
        }
        void sdlKeyDownCallback(SDL_Keysym keysym) {
            if (keysym.sym == SDLK_ESCAPE) {
                closing = true;
            }
        }
        void sdlCloseCallback() {
            closing = true;
        }
};

class FrameworkMain {
    public:
        FrameworkMain() {
            if (SDL_Init(SDL_INIT_VIDEO))
                crash("SDL2 could not be initiate: " + std::string(SDL_GetError()));
        }
        ~FrameworkMain() {
            SDL_Quit();
        }
        static FrameworkWindow* getEventWindow(SDL_Event event) {
            //this is part of the event head, that should be the same for all events that have a windowID
            return reinterpret_cast<FrameworkWindow*>(SDL_GetWindowData(SDL_GetWindowFromID(event.window.windowID), "this"));
        }
        void handleEvents() {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                switch(event.type) {
                    case SDL_KEYDOWN: {
                        getEventWindow(event)->sdlKeyDownCallback(event.key.keysym);
                        break;
                    }
                    case SDL_WINDOWEVENT: {
                        switch(event.window.event) {
                            case SDL_WINDOWEVENT_RESIZED: {
                                getEventWindow(event)->resizeCallback(event.window.data1, event.window.data2);
                                break;
                            }
                            case SDL_WINDOWEVENT_CLOSE: {
                                getEventWindow(event)->sdlCloseCallback();
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }
};

class Framework {
    public:
        Framework(int glMayor, int glMinor, bool gles, int x, int y):
            frameworkWindow(glMayor, glMinor, gles, x, y),
            contextScope(SDL_GL_GetProcAddress)
        {
            glCompact::setDisplayFrameSize(x, y);
        }
        ~Framework(){}
        void handleEvents() {
            frameworkMain.handleEvents();
        }
        void swap() {
            frameworkWindow.swap();
        }
        bool isClosing() const {
            return frameworkWindow.isClosing();
        }
    private:
        FrameworkMain   frameworkMain;
        FrameworkWindow frameworkWindow;
        glCompact::ContextScope contextScope;
};
