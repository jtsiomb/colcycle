# makefile for UNIX/windows builds of colcycle with the OpenGL or SDL backends
# -- options --
PREFIX = /usr/local
BACKEND = gl
# -------------

src = $(wildcard src/*.c) $(wildcard src/$(BACKEND)/*.c)
obj = $(src:.c=.o)
bin = colcycle

CFLAGS = -pedantic -Wall -g -O3 -ffast-math -Isrc $(CFLAGS_$(BACKEND))
LDFLAGS = -lm $(LDFLAGS_$(BACKEND))

CFLAGS_sdl = `pkg-config --cflags sdl`
LDFLAGS_sdl = `pkg-config --libs sdl`
LDFLAGS_gl = $(gl_libs)

ifeq ($(findstring mingw,$(CC)), mingw)
	bin = colcycle.exe
	gl_libs = -lopengl32 -lfreeglut -lglew32
else
	gl_libs = -lGL -lglut -lGLEW
endif

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
