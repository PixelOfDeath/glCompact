#include <SFML/Graphics.hpp>
#include <glCompact/glCompact.hpp>
#include <iostream>

void crash(std::string s){
    std::cout << s << std::endl;
    exit(-1);
}

class FrameworkWindow : private sf::Window {
        friend class Framework;
    public:
        //TODO: something with sf::ContextSettings::Attribute::Core is broken
        FrameworkWindow(int glMayor, int glMinor, bool gles, int sizeX, int sizeY) :
            sf::Window(sf::VideoMode(sizeX, sizeY),
                    "glCompact SFML example",
                    sf::Style::Default,
                    sf::ContextSettings(0, 0, 0, glMayor, glMinor, sf::ContextSettings::Attribute::Default))
        {
            if (gles != 0)
                crash("SFML does not support runtime selection of creating a GLES context! Use GLFW or SDL2 for the examples using GLES!");
            setVerticalSyncEnabled(true);
        }
        void handleEvents() {
            sf::Event event;
            while (pollEvent(event)) {
                switch (event.type) {
                    case sf::Event::Closed:
                        close();
                        break;
                    case sf::Event::KeyPressed:
                        if (event.key.code == sf::Keyboard::Escape) {
                            close();
                        }
                        break;
                    case sf::Event::Resized:
                        glCompact::setWindowFrameSize(event.size.width, event.size.height);
                        break;
                    default: break;
                }
            }
        }
        void swap() {
            display();
        }
        int getSizeX()const {
            return getSize().x;
        }
        int getSizeY()const {
            return getSize().y;
        }
        bool isClosing()const {
            return !isOpen();
        }
    private:
        int sizeX;
        int sizeY;
};

class Framework {
    public:
        Framework(int glMayor, int glMinor, bool gles, int x, int y):
            frameworkWindow(glMayor, glMinor, gles, x, y),
            contextScope(sf::Context::getFunction)
        {}
        ~Framework(){}
        void handleEvents() {
            frameworkWindow.handleEvents();
        }
        void swap() {
            frameworkWindow.swap();
        }
        bool isClosing() const {
            return frameworkWindow.isClosing();
        }
    private:
        FrameworkWindow frameworkWindow;
        glCompact::ContextScope contextScope;
};
