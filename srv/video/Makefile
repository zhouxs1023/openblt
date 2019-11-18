# $Id: //depot/blt/srv/video/Makefile#4 $

include ../../make.conf

CFLAGS = $(CF) -I../../include
LIBS = -L../../lib/obj -lconsole -lblt -lc 

all: video.bin

OBJS = video.o vga.o blit.o

video.bin: $(OBJS)
	$(LD) -dN -Ttext $(ENTRY_USER) -o video.bin ../../lib/crt0.o $(OBJS) $(LIBS)
	$(NM) -nC video.bin > video.map
	$(ST) video.bin

clean:
	rm -f *.o *~ os.* core *.core *.s *.map *.bin
