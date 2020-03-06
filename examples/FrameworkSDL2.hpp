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
        FrameworkWindow(int sizeX, int sizeY) {
            this->sizeX = sizeX;
            this->sizeY = sizeY;
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
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
            glCompact::setWindowFrameSize(x, y);
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
        Framework(int x, int y):
            frameworkWindow(x, y),
            contextScope(SDL_GL_GetProcAddress)
        {}
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
