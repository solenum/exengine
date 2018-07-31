# -- Dirs -- #
CC 			=gcc
CPP     =g++
EDIR    =exengine
LDIR		=lib
BDIR		=build
ODIR 		=obj
IDIR    =inc
_IDIRS  =lib inc
IDIRS   =$(patsubst %,-I%/,$(_IDIRS))

# -- Flags -- #
FLAGS 	=-g -lm -lstdc++ -lGL -lGLEW -lglfw -lphysfs -I. $(IDIRS) '-Wl,-z,origin' '-Wl,-rpath,$$ORIGIN/lib'
CFLAGS  =$(FLAGS)
CFLAGS +=-std=c99 -O2
CPPFLAGS=

# -- Windows -- #
ifeq ($(OS),Windows_NT)
CC 			=x86_64-w64-mingw32-gcc
CPP     =x86_64-w64-mingw32-g++
FLAGS 	=-g -lm -static -static-libgcc -static-libstdc++ -lstdc++ -Llib/win -lopengl32 -lglew32 -lglfw3dll -DGLEW_NO_GLU -I. $(IDIRS) 
CFLAGS  =$(FLAGS)
CFLAGS +=-std=c99
CPPFLAGS=-lstdc++
endif

# -- Files -- #
# engine deps
_EDEPS  =exe_conf.h exe_io.h window.h shader.h mesh.h mathlib.h camera.h \
texture.h stb_image.h iqm.h framebuffer.h pointlight.h exe_list.h scene.h \
model.h dirlight.h skybox.h collision.h entity.h octree.h glimgui.h dbgui.h \
gbuffer.h spotlight.h vertices.h ssao.h engine.h reflectionprobe.h defaults.h
EDEPS		=$(patsubst %,$(EDIR)/%,$(_EDEPS))

# user deps
_DEPS		=game.h
DEPS		=$(patsubst %,$(IDIR)/%,$(_DEPS))

# lib srcs
LOBJ		=cimgui.o fontAtlas.o drawList.o listClipper.o imgui.o imgui_draw.o \
imgui_demo.o imgui_dock.o

# all srcs
_OBJ		=main.o game.o window.o mesh.o camera.o texture.o iqm.o \
framebuffer.o pointlight.o scene.o model.o dirlight.o skybox.o \
collision.o entity.o octree.o glimgui.o dbgui.o gbuffer.o spotlight.o \
ssao.o engine.o reflectionprobe.o shader.o defaults.o
_OBJ   +=$(LOBJ)
OBJ 		=$(patsubst %,$(ODIR)/%,$(_OBJ))

all: files game

# user srcs
$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# engine srcs
$(ODIR)/%.o: $(EDIR)/%.c $(EDEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# cimgui srcs
$(ODIR)/%.o: $(LDIR)/cimgui/%.cpp 
	$(CPP) -c -o $@ $< $(CPPFLAGS) $(FLAGS)

# imgui srcs
$(ODIR)/%.o: $(LDIR)/imgui/%.cpp
	$(CPP) -c -o $@ $< $(CPPFLAGS) $(FLAGS)

game: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)
	@echo "**success**"

files:
	mkdir -p $(ODIR)
	mkdir -p $(BDIR)/licence
	cp licence/* $(BDIR)/licence/
	zip -r -FS build/data.ex data/
ifeq ($(OS),Windows_NT)
	cp lib/win/dll/* build
endif

release-linux:
	mkdir -p $(BDIR)/lib
	cp $$(ldd $(BDIR)/game | grep -i libglew | awk 'NF == 4 {print $$3}; NF == 2 {print $$1}') $(BDIR)/lib
	cp $$(ldd $(BDIR)/game | grep -i libglfw | awk 'NF == 4 {print $$3}; NF == 2 {print $$1}') $(BDIR)/lib
	cp $$(ldd $(BDIR)/game | grep -i libphysfs | awk 'NF == 4 {print $$3}; NF == 2 {print $$1}') $(BDIR)/lib

#release:
#ifeq ($(OS),Windows_NT)
#	copy /b $(BDIR)/game.exe+$(BDIR)/data.exe $(BDIR)/release.exe
#else
#	cat $(BDIR)/game $(BDIR)/data.ex > $(BDIR)/release
#	chmod +x $(BDIR)/release
#endif

.PHONY: clean release-linux

clean:
	rm -f $(ODIR)/*.o
