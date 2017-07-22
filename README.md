# exengine, a C 3D game engine.
**WARNING! Test-ground repo, likely unstable and messy.**

**An official release repo will come in the *near* future.  Please see the [TODO](TODO.md) list.**

### What exactly is exengine?
Began as an experiment, evolved into something feasible for actually making games.  exengine takes a slightly
different approach than other libraries and engines do, in that its a code-base you include directly into yours.  Rather than using it as a static/shared library.

This approach allows easy and direct access to the engine back-end should you want to make modifications to suit your specific needs.

The code-base is almost entirely C99, with an exception for [imgui](https://github.com/ocornut/imgui) support.

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

### Depends
* A C99 and C++ compiler (gcc, clang etc)
* GLFW3
* GLEW
* OpenGL 3.3+

### Gallery
![scrot](http://i.imgur.com/4NGlapU.png)
![scrot](http://i.imgur.com/H1pMBXI.png)
![scrot](http://i.imgur.com/mpe1AjU.gif)