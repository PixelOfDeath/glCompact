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
        FrameworkWindow(int glMayor, int glMinor, bool gles, int sizeX, int sizeY) {
            this->sizeX = sizeX;
            this->sizeY = sizeY;

            if (gles) {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glMayor);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glMinor);
            } else {
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glMayor);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glMinor);
            }

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
            glCompact::setDisplayFrameSize(x, y);
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
        Framework(int glMayor, int glMinor, bool gles, int x, int y):
            frameworkWindow(glMayor, glMinor, gles, x, y),
            contextScope(glfwGetProcAddress)
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
