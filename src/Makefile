# -- Dirs -- #
CC 			=gcc
IDIR		=inc
EDIR    =exengine
LDIR		=lib
BDIR		=build
ODIR 		=obj
# -- Flags -- #
CFLAGS 	=-std=c99 -g -lm -lGL -lGLEW -lglfw -I.
# -- Files -- #
_EDEPS  =exe_conf.h exe_io.h window.h shader.h mesh.h mathlib.h camera.h \
texture.h stb_image.h iqm.h framebuffer.h pointlight.h exe_list.h scene.h \
model.h dirlight.h skybox.h collision.h entity.h octree.h
EDEPS		=$(patsubst %,$(EDIR)/%,$(_EDEPS))

_DEPS		=game.h
DEPS		=$(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ		=main.o game.o window.o mesh.o camera.o texture.o iqm.o \
framebuffer.o pointlight.o scene.o model.o dirlight.o skybox.o \
collision.o entity.o octree.o
OBJ 		=$(patsubst %,$(ODIR)/%,$(_OBJ))

all: files uzm

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(EDIR)/%.c $(EDEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

uzm: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)

files:
	mkdir -p $(ODIR)
	mkdir -p build
	cp -RTv data build/data/

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o