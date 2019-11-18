# $Id: //depot/blt/lib/libblt/Makefile#1 $

include ../../make.conf

CFLAGS = $(CF) -I../../include

all: libblt.a

OBJS = namer.o syscalls.o 

syscalls.o: syscalls.S
	$(CC) $(CFLAGS) -c -o syscalls.o syscalls.S
 
libblt.a: $(OBJS)
	rm -f libblt.a
	$(AR) r libblt.a $(OBJS)

libblt.so: $(OBJS)
	$(CC) -shared -o libblt.so $(OBJS)

clean:
	rm -f *.a *.so *.o *~ os.* core *.core *.s *.map *.bin
