### General / Enhancements

```text
exengine/<ALL FILES>
[X]  Finish writing this damn TODO list
[ ]  Write CONTRIBUTING.md
[ ]  Migrate all exengine source files and file names to use the prefix ex_
[ ]  Migrate include guards to use the prefix EX_
[ ]  Implement quad-trees for polygon soup etc
[ ]  Implement Light culling
[ ]  Finish render profiler
[ ]  Add CPU profiler
[ ]  Add scene editor using imgui
[ ]  Document headers better
[ ]  Embed default shaders
[ ]  Add simple raycast functions to collision.c
[ ]  Add user-level input functions & handling
[ ]  Implement user-level file io for safe locations
[ ]  Add instanced rendering for things like grass etc
[ ]  Migrate to forward+ rendering?
[ ]  Write API documentation
[ ]  A better cross-platform build system
```

### Modifications

```text
exengine/framebuffer.c
[ ]  Make it more arbitrary and modular for general canvas usage
```

```text
exengine/entity.c
[ ]  Make collision functions use packets directly rather than entities
[ ]  Add easy physical control & movement
```

```text
exengine/shader.h
[ ]  Split shader loading and compiling into separate functions
```

```text
exengine/window.c
[ ]  Add better input data storage
```

```text
exengine/camera.c
[ ]  Add various other camera types such as ortho
[ ]  Rewrite to be more modular
```

```text
exengine/exe_conf.h
[ ]  Add config saving to safe directory
```

```text
exengine/scene.c
[ ]  Add scene gravity and tuning
```

```text
exengine/iqm.c
[ ]  Load and store animation names
```

```text
exengine/model.c
[ ]  Implement smooth animation transitions
[ ]  Set animations by name
[ ]  Add fine control for animation speed etc
[ ]  Fix model->bone attachments
```