src = $(wildcard src/*.c)
obj = $(src:.c=.o)
bin = ccimg

CFLAGS = -pedantic -Wall -g `pkg-config --cflags sdl`
LDFLAGS = `pkg-config --libs sdl` -lm

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
