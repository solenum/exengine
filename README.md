# exengine, a C 3D game engine.

**This engine is far from complete**

### What exactly is exengine?
exengine is a 3d engine that takes a slightly different approach than other libraries and engines do, in that it's a code-base you include directly into your own.  Rather than using it as a static/shared library.

This approach allows easy and direct access to the engine back-end should you want to make modifications to suit your specific needs, so think of it as more of a template.

Assuming you don't want to set up your own build system (I can't blame you), you can clone the repo and use the existing build system and file structure as a starting template.  It compiles on Linux, MacOS, and Windows.

The code-base is almost entirely C99, with an exception for [imgui](https://github.com/ocornut/imgui) support, which is also optional and exists only for tools and debugging.

**The contents of main.c/game.c are only supplied as examples to showcase how one might use the engine.**

### What are the features?
* *Simple* and small
* C99 compliant
* Deferred rendering
* Various light casters
* Smooth shadow mapping
* Normal & specular mapping
* Half-kernel SSAO
* IQM model loading
* 3D model animation
* Scene manager
* Instancing
* Polygon soup collision detection
* Smooth collision response
* Various cameras
* Debug GUI with docking
* More to come..

### Depends
* A C99 and C++ compiler (gcc, clang etc)
* GLFW3
* GLEW
* OpenGL 3.3+
* OpenAL

### Getting Started
#### Documentation
The current method of documentation is code comments, every engine header has extensive documentation as to its purpose and how one might use it.  Better documentation will come in the near future.

#### Using & Compiling

Simply clone the repository and install the required libraries and compilers listed below for the OS you are using.

**This only applies to those wanting to use the current build system and game code as a starting template, and not those wanting to include exengine into an existing project or build system.**

#### Linux

````
sudo apt-get update
sudo apt-get install libglfw3-dev libglew-dev libopenal-dev
cd src && make
````

The resulting binary will be in src/build/

#### MacOS

Install [brew](https://brew.sh), and the following packages

````
brew install glfw glew gcc
````

After this compiling is the same as Linux.

#### Windows

Download and install [Cygwin](https://www.cygwin.com/) with the following packages.  *All required libraries for Windows are included in the codebase.*

````
mingw64-x86_64-gcc
mingw64-x86_64-gcc-g++
make
zip
````

Alternatively switch to category view and set ['Devel' to install.](https://i.stack.imgur.com/2uzkB.jpg)  *This method takes up 1-2GB of space*

After this compiling is the same as Linux.

````
cd C:\exengine-testing (or wherever you've put this)
cd src && make
````

the resulting .exe will be in src/build/

#### OpenBSD
Make sure you have the correct dependicies installed, the equivalent of linux using pkg_add.
For OpenBSD specific you will need the 'gmake' and 'gcc', 'g++' packages.
The gcc and g++ packages will install a more modern compiler as egcc and eg++ in your path.

```
doas pkg_add g++ glew openal glfwls
```

after this the compilation is almost the same as in linux, this time specificly using gmake(stands for GNU make).

````
cd src && gmake
````

#### Stand-alone (*recommended*)

Alternatively you can drop the exengine/ directory right into your own project and use it with your own build system.  This method obviously takes more work and time.

### Gallery
![scrot](http://i.imgur.com/4NGlapU.png)
![scrot](https://i.imgur.com/vTKB3T8.png)
![scrot](http://i.imgur.com/H1pMBXI.png)
![scrot](https://i.imgur.com/n6FcPau.png)
![scrot](https://i.imgur.com/t7CcuiK.png)
![scrot](https://i.imgur.com/qesG2md.png)
