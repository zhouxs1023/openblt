# $Id: //depot/blt/kernel/Makefile#6 $

include ../make.conf

CFLAGS = $(KCF) -I../boot -I../include -DASSERT 
LIBS = -L../lib/obj -lkern -lconsole

all: kernel.bin

OBJS = stub.o kernel.o i386.o jump.o ktrace.o \
	memory.o resource.o port.o sem.o aspace.o task.o rights.o \
	fault.o syscall.o queue.o debug.o assert.o

ifeq ($(SMP),-D__SMP__)
OBJS += smp.o cpuid.o trampoline.o
endif

kernel.bin: $(OBJS)
	$(LD) -dN -Ttext $(ENTRY_KERNEL) -o kernel.bin $(OBJS)	$(LIBS)
	$(NM) -nC kernel.bin > kernel.map
	$(ST) kernel.bin

clean:
	rm -f *.o *~ os.* core *.core *.s *.map *.bin

.S.o:
	$(CC) $(KCF) -D__ASM__ -c $<

