# glCompact
A multi platform, minimal dependency, low level but very simple C++11 Graphics API to interface a subset of OpenGL 3.3+

This is an ALPHA version! The API is not stable and will change in the future!

It borrows some concepts, names and calling conventions from Vulkan.

## Prerequisites

To BUILD this library you need [GLM](https://github.com/g-truc/glm), [python](https://www.python.org), CMake and a C++ build system.

Note that python is only used to generate internal files. It is not part of the dependency when using glCompact.

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

Ubuntu:

```sh
mkdir build
cd build
cmake ..
make -j
sudo make install
```

To also generate the documentation:

```
make doc
```

Visual Studio:

Cmake can create visual studio project files or nmake files.

## Usage

To USE this library you need [GLM](https://github.com/g-truc/glm) and a library to create a Window, the OpenGL context and provide a function to query OpenGL function pointers. (e.g. [SDL2](https://www.libsdl.org)/[GLFW](https://www.glfw.org/)/[SFML](https://www.sfml-dev.org/)/...)

Examples are provided in the examples folder! They also can by used as a base for a new project.

NOTE: Older g++ versions (e.g. on Debian stable) have [broken constructor inherence](https://bugzilla.redhat.com/show_bug.cgi?id=1419687).
The examples contain a comment with workaround.

## License

The whole project is under MIT license!
