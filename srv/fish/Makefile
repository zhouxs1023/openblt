# $Id: //depot/blt/srv/fish/Makefile#4 $

include ../../make.conf

CFLAGS = $(CF) -I../../include
LIBS = -L../../lib/obj -lconsole -lblt -lc 

all: fish.bin

OBJS = fish.o vga.o

fish.bin: $(OBJS)
	$(LD) -dN -Ttext $(ENTRY_USER) -o fish.bin ../../lib/crt0.o $(OBJS) $(LIBS)
	$(NM) -nC fish.bin > fish.map
	$(ST) fish.bin

clean:
	rm -f *.o *~ os.* core *.core *.s *.map *.bin
