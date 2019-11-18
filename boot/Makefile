# $Id: //depot/blt/boot/Makefile#3 $

include ../make.conf

CFLAGS = $(CF) -I../kernel -I../include

all: boot.bin

boot.bin: bootstub.o boot.o 
	$(LD) -dN -Ttext $(ENTRY_BOOTSTRAP) -o boot.bin bootstub.o boot.o ../lib/obj/libconsole.a
	$(NM) -nC boot.bin > boot.map
	$(ST) boot.bin

clean:
	rm -f *.o *~ os.* core *.core *.s *.map *.bin

