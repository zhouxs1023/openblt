# $Id: //depot/blt/srv/namer/Makefile#4 $

include ../../make.conf

CFLAGS = $(CF) -I../../include
LIBS = -L../../lib/obj -lblt -lc

all: namer.bin

namer.bin: namer.o
	$(LD) -dN -Ttext $(ENTRY_USER) -o namer.bin ../../lib/crt0.o namer.o $(LIBS)
	$(NM) -nC namer.bin > namer.map
	$(ST) namer.bin

clean:
	rm -f *.o *~ os.* core *.core *.s *.map *.bin
