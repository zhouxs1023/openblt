# $Id: //depot/blt/srv/ne2000/Makefile#4 $

include ../../make.conf

CFLAGS = $(CF) -I../../include
LIBS = -L../../lib/obj -lconsole -lblt -lc

all: ne2000.bin

OBJS = ne2k.o ne2000.o


ne2000.bin: $(OBJS)
	$(LD) -dN -Ttext $(ENTRY_USER) -o ne2000.bin ../../lib/crt0.o $(OBJS) $(LIBS)
	$(NM) -nC ne2000.bin > ne2000.map
	$(ST) ne2000.bin

clean:
	rm -f *.o *~ os.* core *.core *.s *.map *.bin
