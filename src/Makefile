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
FLAGS 	=-g -lm -lstdc++ -lGL -lGLEW -lglfw -I. $(IDIRS)
CFLAGS  =$(FLAGS)
CFLAGS +=-std=c99
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
gbuffer.h spotlight.h vertices.h ssao.h
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
collision.o entity.o octree.o glimgui.o dbgui.o gbuffer.o spotlight.o ssao.o
_OBJ   +=$(LOBJ)
OBJ 		=$(patsubst %,$(ODIR)/%,$(_OBJ))

all: files uzm

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

uzm: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)
	@echo "**success**"

files:
	mkdir -p $(ODIR)
	mkdir -p build
	cp -RTv data build/data/
ifeq ($(OS),Windows_NT)
	cp lib/win/dll/* build
endif

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
