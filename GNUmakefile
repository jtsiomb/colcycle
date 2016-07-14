src = $(wildcard src/*.c) $(wildcard src/sdl/*.c)
obj = $(src:.c=.o)
bin = colcycle

CFLAGS = -std=c89 -pedantic -Wall -g -Isrc `pkg-config --cflags sdl`
LDFLAGS = `pkg-config --libs sdl` -lm

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
