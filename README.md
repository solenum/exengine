# exengine, a C 3D game engine.
**WARNING! Test repo, likely unstable and messy.**

**An official release repo will come in the *near* future.  Please see the [TODO](TODO.md) list.**

### What exactly is exengine?
Began as an experiment, evolved into something feasible for actually making games.  exengine takes a slightly
different approach than other libraries and engines do, in that its a code-base you include directly into yours.  Rather than using it as a static/shared library.

This approach allows easy and direct access to the engine back-end should you want to make modifications to suit your specific needs.

The code-base is almost entirely C99, with an exception for [imgui](https://github.com/ocornut/imgui) support.

**The contents of main.c/game.c are only supplied as examples to showcase how one might use exengine.  While these source files and the current build system can work as a nice starting template, it is suggested that you include the src/exengine/ directory into your own project and setup a build system to suit your own needs.**

Everything outside of src/exengine/ is supplied as an example **ONLY**, use it if you so wish.

### What are the features?
* *Simple* and small
* C99 compliant
* Various light casters
* Smooth shadow mapping
* Normal & specular mapping
* IQM model loading
* 3D model animation
* Scene manager
* Polygon soup collision detection
* Smooth collision response
* Various cameras
* View models
* Debug GUI with docking
* More to come, check the [TODO](TODO.md) list

### Depends
* A C99 and C++ compiler (gcc, clang etc)
* GLFW3
* GLEW
* OpenGL 3.3+

### Getting Started
#### Documentation
Docs will be available once an official release repository is out.  For now refer to the engine headers should you want to experiment with this.

#### Using & Compiling

Simply clone the repository and include src/exengine/ and src/lib/ into your codebase.  The only build system supplied currently is a Linux makefile, for those on Windows you'll need to use Cygwin.  See compilation instructions below.

**This only applies to those wanting to use the current build system and game code as a starting template, and not those wanting to include exengine into an existing project or build system.**

#### Linux

````
sudo apt-get update
sudo apt-get install libglfw3-dev libglew-dev
cd src && make
````

The resulting binary will be in src/build/

#### Windows

Download and install [Cygwin](https://www.cygwin.com/) with the following packages.

````
mingw64-x86_64-gcc
mingw64-x86_64-gcc-g++
make
````

Alternatively switch to category view and set ['Devel' to install.](https://i.stack.imgur.com/2uzkB.jpg)  *This method takes up 1-2GB of space*

After this compiling is the same as Linux.

````
cd C:\exengine-testing (or wherever you've put this)
cd src && make
````

the resulting .exe will be in src/build/

#### Stand-alone (*recommended*)

Alternatively you can drop the exengine/ directory right into your own project and use it with your own build system.  This method obviously takes more work and time but is the recommended approach.  Everything outside of src/exengine/ is supplied as an example **ONLY**

### Gallery
![scrot](http://i.imgur.com/4NGlapU.png)
![scrot](http://i.imgur.com/H1pMBXI.png)
