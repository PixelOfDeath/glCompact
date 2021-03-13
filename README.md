# glCompact
A multi platform, minimal dependency, low level but very simple C++11 Graphics API to interface a subset of OpenGL 3.3+

This is an ALPHA version! The API is not stable and will change in the future!

It borrows some concepts, names and calling conventions from Vulkan.

## Prerequisites

To BUILD this library you need [GLM](https://github.com/g-truc/glm), [python](https://www.python.org), CMake and a C++ build system.

Note that python is only used to generate files during compilation. It is not a runtime dependency for projects using glCompact.

Ubuntu:

```
sudo apt install libglm-dev python cmake g++
```

To also generate the documentation:

```
sudo apt install doxygen
```

To be able to build the examples, install one of the supported framework libraries:

```
sudo apt install libglfw3-dev
```

```
sudo apt install libsdl2-dev
```

```
sudo apt install libsfml-dev
```

## Installation

glCompact is meant to be included as a cmake source sub directory. This enables the use of different compile time configurations files and better optimisation for each project using it.

Create an enviroment variable "glCompact_DIR", containing your glCompact path. And include it in your projects CMakeLists.txt:
```
SET(glCompact_DIR $ENV{glCompact_DIR})
add_subdirectory(${glCompact_DIR} ${PROJECT_BINARY_DIR}/glCompact)

#...

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${GLCOMPACT_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    ${GLCOMPACT_LIBRARY}
)

```

GLCOMPACT_INCLUDE_DIRS and GLCOMPACT_LIBRARY are set by the glCompact CMakeLists.txt. GLCOMPACT_INCLUDE_DIRS also includes GLM!


To generate the documentation, execute this in the glCompact folder:

```
mkdir build
cd build
cmake ..
make doc
```

## Usage

To USE this library you need [GLM](https://github.com/g-truc/glm) and a library to create a Window, the OpenGL context and provide a function to query OpenGL function pointers. (e.g. [SDL2](https://www.libsdl.org)/[GLFW](https://www.glfw.org/)/[SFML](https://www.sfml-dev.org/)/...)

Examples are provided in the examples folder! They also can by used as a base for a new project.

## License

The whole project is under MIT license!
