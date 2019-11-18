# $Id: //depot/blt/srv/vfs/Makefile#2 $

include ../../make.conf

CFLAGS = $(CF) -I../../include
LIBS = -L../../lib/obj -lblt -lc

all: vfs.bin

vfs.bin: vfs.o
	$(LD) -dN -Ttext $(ENTRY_USER) -o vfs.bin ../../lib/crt0.o vfs.o $(LIBS)
	$(NM) -nC vfs.bin > vfs.map
	$(ST) vfs.bin

clean:
	rm -f *.o *~ os.* core *.core *.s *.map *.bin
