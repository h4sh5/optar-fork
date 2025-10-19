# to compile to webassembly: (make sure emscripten dev environment is installed correctly)

CC=emcc
CFLAGS=-O3 -Wall -sALLOW_MEMORY_GROWTH=1 -sUSE_LIBPNG -sINVOKE_RUN=0 
LDLIBS=

all: optar.js unoptar.js

install:
	install optar /usr/local/bin/
	install unoptar /usr/local/bin
	install pgm2ps /usr/local/bin

uninstall:
	rm /usr/local/bin/optar
	rm /usr/local/bin/unoptar
	rm /usr/local/bin/pgm2ps

clean:
	rm -f wasm/*.wasm

common.o: common.c optar.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

parity.o: parity.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

optar.o: optar.c optar.h font.h parity.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

golay_codes.o: golay_codes.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

golay.o: golay.c parity.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

unoptar.o: unoptar.c optar.h parity.h
	$(CC) -c -I/usr/local/include/libpng $(CPPFLAGS) $(CFLAGS) -o $@ $<

optar.js: optar.o common.o golay_codes.o parity.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o wasm/$@ $^


golay: golay.o parity.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

unoptar.js: unoptar.o common.o golay_codes.o parity.o
	$(CC) $(LDFLAGS) -o wasm/$@ $^ $(LDLIBS) $(CFLAGS)
