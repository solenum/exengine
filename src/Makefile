# -- Dirs -- #
CC 			=gcc
IDIR		=inc
LDIR		=lib
BDIR		=build
ODIR 		=obj
# -- Flags -- #
CFLAGS 	=-std=c99 -g -lm -lGL -lGLEW -lglfw -I$(IDIR)
# -- Files -- #
_DEPS		=conf.h io.h game.h window.h shader.h mesh.h mathlib.h camera.h texture.h stb_image.h iqm.h framebuffer.h pointlight.h list.h scene.h
DEPS		=$(patsubst %,$(IDIR)/%,$(_DEPS))
_OBJ		=main.o conf.o game.o window.o mesh.o camera.o texture.o iqm.o framebuffer.o pointlight.o list.o scene.o
OBJ 		=$(patsubst %,$(ODIR)/%,$(_OBJ))

all: files uzm

$(ODIR)/%.o: %.c $(DEPS)
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