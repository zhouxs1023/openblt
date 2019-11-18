# $Id: //depot/blt/lib/Makefile#5 $

include ../../make.conf

CFLAGS = $(CF) -I../../include

all: libconsole.a

KERN = string.o snprintf.o bcmp.o bzero.o strcmp.o strcpy.o strlen.o
USER = syscall.o namer.o 
MEM = malloc.o memory.o

libconsole.a: conio.o
	rm -rf libconsole.a
	$(AR) r libconsole.a conio.o

libconsole.so: conio.o
	$(CC) -shared -o libconsole.so conio.o

clean:
	rm -f *.a *.so *.o *~ os.* core *.core *.s *.map *.bin
