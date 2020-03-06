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
#define GLFW_INCLUDE_NONE //include no OpenGL headers via GLFW
#include <GLFW/glfw3.h>
#include <glCompact/glCompact.hpp>
#include <iostream>

void crash(std::string s){
    std::cout << s << std::endl;
    exit(-1);
}

class FrameworkMain {
    public:
        FrameworkMain() {
            if (!glfwInit())
                crash("Could not initiated GLFW");
        }
        ~FrameworkMain() {
            glfwTerminate();
        }
        void handleEvents() {
            glfwPollEvents();
        }
};

class FrameworkWindow {
    public:
        FrameworkWindow(int sizeX, int sizeY) {
            this->sizeX = sizeX;
            this->sizeY = sizeY;
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            window = glfwCreateWindow(sizeX, sizeY, "glCompact GLFW example", NULL, NULL);
            if (!window)
                crash("Could not create GLFW window");
            glfwSetWindowUserPointer (window, this);
            glfwSetWindowSizeCallback(window, &FrameworkWindow::glfwResizeCallback);
            glfwSetKeyCallback       (window, &FrameworkWindow::glfwKeyCallback);
            glfwMakeContextCurrent   (window);
        }
        ~FrameworkWindow() {
            glfwDestroyWindow(window);
        }
        void swap() {
            glfwSwapBuffers(window);
        }
        int getSizeX()const {
            return sizeX;
        }
        int getSizeY()const {
            return sizeY;
        }
        bool isClosing()const {
            return glfwWindowShouldClose(window);
        }

        GLFWwindow* window;
    private:
        int sizeX;
        int sizeY;

        static void glfwResizeCallback(GLFWwindow* window, int x, int y) {
            auto* frameworkWindow = reinterpret_cast<FrameworkWindow*>(glfwGetWindowUserPointer(window));
            frameworkWindow->resizeCallback(x, y);
        }
        void resizeCallback(int x, int y) {
            sizeX = x;
            sizeY = y;
            glCompact::setWindowFrameSize(x, y);
        }
        static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
            auto* frameworkWindow = reinterpret_cast<FrameworkWindow*>(glfwGetWindowUserPointer(window));
            frameworkWindow->glfwKeyCallback(key, scancode, action, mods);
        }
        void glfwKeyCallback(int key, int scancode, int action, int mods) {
            if (action == GLFW_PRESS) {
                if (key == GLFW_KEY_ESCAPE) {
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
            }
        }
};

class Framework {
    public:
        Framework(int x, int y):
            frameworkWindow(x, y),
            contextScope(glfwGetProcAddress)
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
