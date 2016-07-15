# options
PREFIX = /usr/local
BACKEND = gl
# -------

src = $(wildcard src/*.c) $(wildcard src/$(BACKEND)/*.c)
obj = $(src:.c=.o)
bin = colcycle

CFLAGS = -pedantic -Wall -g -O3 -ffast-math -Isrc $(CFLAGS_$(BACKEND))
LDFLAGS = -lm $(LDFLAGS_$(BACKEND))

CFLAGS_sdl = `pkg-config --cflags sdl`
LDFLAGS_sdl = `pkg-config --libs sdl`
LDFLAGS_gl = -lGL -lglut -lGLEW

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
