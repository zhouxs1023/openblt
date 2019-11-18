# $Id: //depot/blt/lib/Makefile#8 $

include ../make.conf

CFLAGS = $(CF) -I../include

all: obj crt0.o libc libblt libconsole

obj:
	mkdir obj
	ln -fs ../libc/libc.a obj
	ln -fs ../libc/libkern.a obj
	ln -fs ../libblt/libblt.a obj
	ln -fs ../libconsole/libconsole.a obj

libc::
	@cd libc ; $(MAKE)

libconsole::
	@cd libconsole ; $(MAKE)

libblt::
	@cd libblt ; $(MAKE)

clean:
	rm -rf obj
	rm -f *.a *.so *.o *~ os.* core *.core *.s *.map *.bin
	@cd libc ; $(MAKE) clean
	@cd libblt ; $(MAKE) clean
	@cd libconsole ; $(MAKE) clean
