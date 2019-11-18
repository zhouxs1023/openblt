# $Id: //depot/blt/lib/Makefile#5 $

include ../../make.conf

CFLAGS = $(CF) -I../../include

all: libc.a libkern.a

MINIMAL = string.o snprintf.o bcmp.o bzero.o strcmp.o strcpy.o strlen.o
EXTRA = malloc.o memory.o

libc.a: $(MINIMAL) $(EXTRA)
	rm -f libc.a
	$(AR) r libc.a $(MINIMAL) $(EXTRA)

libc.so: $(MINIMAL) $(EXTRA)
	$(CC) -shared -o libc.so $(MINIMAL) $(EXTRA)

libkern.a: $(MINIMAL)
	rm -f libkern.a
	$(AR) r libkern.a $(MINIMAL)

clean:
	rm -f *.a *.so *.o *~ os.* core *.core *.s *.map *.bin
