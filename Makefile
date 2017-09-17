CC=clang
LD=clang

LIBS=harfbuzz icu-uc freetype2 cairo sdl

CFLAGS=$(shell pkg-config --cflags $(LIBS))
LDFLAGS=$(shell pkg-config --libs $(LIBS))

SOURCES=how-to-show-text-on-screen.c
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

./how-to-show-text-on-screen: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

clean:
	rm -f ./how-to-show-text-on-screen $(OBJECTS)
