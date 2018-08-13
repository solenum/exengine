# Example Makefile

# -- Dirs -- #
CC 			=gcc
CPP     =g++
EDIR    =exengine
LDIR		=lib
BDIR		=build
ODIR 		=obj
IDIR    =inc
_IDIRS  =lib inc lib/physfs 
IDIRS   =$(patsubst %,-I%/,$(_IDIRS))

# -- Flags -- #
FLAGS 	=-g -lm -lstdc++ -lGL -lGLEW -lglfw -lopenal -I. $(IDIRS) '-Wl,-z,origin' '-Wl,-rpath,$$ORIGIN/lib'
CFLAGS  =$(FLAGS)
CFLAGS +=-std=c99 -O2
CPPFLAGS=

# -- Windows -- #
ifeq ($(OS),Windows_NT)
CC 			=x86_64-w64-mingw32-gcc
CPP     =x86_64-w64-mingw32-g++
FLAGS 	=-g -lm -static -static-libgcc -static-libstdc++ -lstdc++ -Llib/win -lopengl32 -lglew32 -lglfw3dll -lopenal32 -DGLEW_NO_GLU -I. $(IDIRS)
CFLAGS  =$(FLAGS)
CFLAGS +=-std=c99
CPPFLAGS=-lstdc++
endif

# -- MacOS -- #
UNAME = $(shell uname -s)
ifeq ($(UNAME),Darwin)
FLAGS   =-g -lstdc++ -framework OpenGl -framework Foundation -framework IOKit -lglfw -lglew -lphysfs -framework OpenAL -I. $(IDIRS) -Wno-unused-command-line-argument
CFLAGS  =$(FLAGS)
CFLAGS +=-std=c99 -O2
endif

# -- Files -- #
# engine deps
_EDEPS  =exe_conf.h exe_io.h window.h shader.h mesh.h mathlib.h camera.h \
texture.h stb_image.h iqm.h framebuffer.h pointlight.h exe_list.h scene.h \
model.h dirlight.h skybox.h collision.h entity.h octree.h glimgui.h dbgui.h \
gbuffer.h spotlight.h vertices.h ssao.h engine.h reflectionprobe.h defaults.h input.h \
sound.h cache.h
EDEPS		=$(patsubst %,$(EDIR)/%,$(_EDEPS))

# engine srcs
ENGINEOBJ = window.o mesh.o camera.o texture.o iqm.o \
framebuffer.o pointlight.o scene.o model.o dirlight.o skybox.o \
collision.o entity.o octree.o glimgui.o dbgui.o gbuffer.o spotlight.o \
ssao.o engine.o reflectionprobe.o shader.o defaults.o input.o sound.o cache.o

# lib deps
_PHYSFS_DEPS =physfs_casefolding.h  physfs.h  physfs_internal.h  physfs_lzmasdk.h  physfs_miniz.h  physfs_platforms.h
PHYSFS_DEPS  =$(patsubst %,$(LDIR)/physfs/%,$(_PHYSFS_DEPS))

# lib srcs
LOBJ		=cimgui.o fontAtlas.o drawList.o listClipper.o imgui.o imgui_draw.o \
imgui_demo.o imgui_dock.o \
physfs_archiver_7z.o physfs_archiver_mvl.o physfs_archiver_wad.o physfs_platform_posix.o \
physfs_archiver_dir.o physfs_archiver_qpak.o physfs_archiver_zip.o physfs_platform_qnx.o \
physfs_archiver_grp.o physfs_archiver_slb.o physfs_byteorder.o physfs_platform_unix.o \
physfs_archiver_hog.o physfs_archiver_unpacked.o physfs.o physfs_platform_windows.o \
physfs_archiver_iso9660.o physfs_archiver_vdf.o physfs_platform_os2.o physfs_unicode.o

ifeq ($(UNAME),Darwin)
LOBJ += physfs_platform_apple.o
endif

# usr srcs (add yours here)
_OBJ		=main.o game.o
_OBJ   +=$(LOBJ)
_OBJ   +=$(ENGINEOBJ)
OBJ 		=$(patsubst %,$(ODIR)/%,$(_OBJ))

# user deps
_DEPS		=game.h
DEPS		=$(patsubst %,$(IDIR)/%,$(_DEPS))

all: files game

# user src
$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# engine srcs
$(ODIR)/%.o: $(EDIR)/%.c $(EDEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# physfs src
$(ODIR)/%.o: $(LDIR)/physfs/%.c $(PHYSFS_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ifeq ($(UNAME),Darwin)
$(ODIR)/%.o: $(LDIR)/physfs/%.m $(PHYSFS_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
endif

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
	rm $(BDIR)/lib/*
	cp $$(ldd $(BDIR)/game | grep -i libglew | awk 'NF == 4 {print $$3}; NF == 2 {print $$1}') $(BDIR)/lib
	cp $$(ldd $(BDIR)/game | grep -i libglfw | awk 'NF == 4 {print $$3}; NF == 2 {print $$1}') $(BDIR)/lib
	cp $$(ldd $(BDIR)/game | grep -i libopenal | awk 'NF == 4 {print $$3}; NF == 2 {print $$1}') $(BDIR)/lib

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
