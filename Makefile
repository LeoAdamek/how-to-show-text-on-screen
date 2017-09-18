CC=clang
LD=clang

LIBS=icu-uc freetype2 cairo sdl harfbuzz harfbuzz-icu

CFLAGS=$(shell pkg-config --cflags $(LIBS))
LDFLAGS=$(shell pkg-config --libs $(LIBS))

SOURCES=how-to-show-text-on-screen.c
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

%.o: %.c
	$(CC) $(CFLAGS) -g -c -o $@ $<

./how-to-show-text-on-screen: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

clean:
	rm -f ./how-to-show-text-on-screen $(OBJECTS)
