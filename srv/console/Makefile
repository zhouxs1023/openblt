# $Id: //depot/blt/srv/console/Makefile#4 $

include ../../make.conf

CFLAGS = $(CF) -I../../include
LIBS = -L../../lib/obj -lconsole -lblt -lc

all: console.bin

console.bin: console.o
	$(LD) -dN -Ttext $(ENTRY_USER) -o console.bin ../../lib/crt0.o console.o $(LIBS)
	$(NM) -nC console.bin > console.map
	$(ST) console.bin

clean:
	rm -f *.o *~ os.* core *.core *.s *.map *.bin
