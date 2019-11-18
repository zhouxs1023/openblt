# $Id: //depot/blt/netboot/Makefile#2 $

include ../make.conf

CFLAGS = $(CF) -I../include
LIBS = -L../lib -lconsole -lkern

all: netboot.bin netrom.hex

makerom: makerom.o
	gcc -o makerom makerom.o

netrom.hex: netboot.bin makerom
	cat netrom.bin netboot.bin > temp.bin
	./makerom temp.bin netrom.hex
	rm -f temp.bin


OBJS = crt0.o netboot.o ne2000.o

netboot.bin: $(OBJS)
	$(LD) -dN -Ttext $(ENTRY_ROM) -o netboot.bin  $(OBJS) $(LIBS)
	$(NM) -nC netboot.bin > netboot.map
	$(ST) netboot.bin

clean:
	rm -f *.o *~ os.* core *.core *.s *.map netboot.bin temp.bin netrom.hex *.boot makerom
