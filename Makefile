LDFLAGS=-L/usr/local/lib `libpng-config --L_opts`
#add -sUSE_LIBPNG	=1 for emcc
# add -g in line below for debug with gdb
CFLAGS=-O3 -Wall -Wuninitialized -fomit-frame-pointer -funroll-loops `libpng-config --I_opts`
LDLIBS=-lpng -lz -lm

all: optar unoptar

install:
	install optar /usr/local/bin/
	install unoptar /usr/local/bin
	install pgm2ps /usr/local/bin

uninstall:
	rm /usr/local/bin/optar
	rm /usr/local/bin/unoptar
	rm /usr/local/bin/pgm2ps

clean:
	rm -f optar unoptar golay  *.o

common.o: common.c optar.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

parity.o: parity.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

optar.o: optar.c optar.h font.h parity.h common.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

golay_codes.o: golay_codes.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

golay.o: golay.c parity.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

unoptar.o: unoptar.c optar.h parity.h common.c
	$(CC) -c -I/usr/local/include/libpng $(CPPFLAGS) $(CFLAGS) -o $@ $<

optar: optar.o common.o golay_codes.o parity.o
	$(CC) $(LDFLAGS) -o $@ $^ $(CFLAGS)

golay: golay.o parity.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

unoptar: unoptar.o common.o golay_codes.o parity.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
