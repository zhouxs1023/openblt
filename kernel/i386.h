/* $Id: //depot/blt/kernel/i386.h#3 $
**
** Copyright 1998 Brian J. Swetland
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions, and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions, and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _I386_H
#define _I386_H

typedef struct
{
    uint16 link, __unused0;
    uint32 esp0;
    uint16 ss0,  __unused1;
    uint32 esp1;
    uint16 ss1,  __unused2;
    uint32 esp2;
    uint16 ss2,  __unused3;
    uint32 cr3,eip,eflags,eax,ecx,edx,ebx,esp,ebp,esi,edi;
    uint16 es,   __unused4;
    uint16 cs,   __unused5;
    uint16 ss,   __unused6;
    uint16 ds,   __unused7;
    uint16 fs,   __unused8;
    uint16 gs,   __unused9;
    uint16 ldts, __unused10;
    uint16 debugtrap, iomapbase;
} TSS;

/* rights bits options */
#define i386rPRESENT 0x80   /* segment is present */

#define i386rDPL0    0x00
#define i386rDPL1    0x20
#define i386rDPL2    0x40
#define i386rDPL3    0x60

/* for Data/Code/TSS segments */
#define i386rDATA    0x12   /* segment is data, read/write */
#define i386rDATAro  0x10   /* segment is data, read only */
#define i386rCODE    0x1A   /* segment is code, read/exec */
#define i386rCODExo  0x18   /* segment is code, read only */

#define i386rTSS     0x09   /* segment is an i386 TSS */

/* gran bits options */
#define i386g4K      0x80   /* segment gran is 4K (else 1B) */
#define i386g32BIT   0x40   /* segment default is 32bit (else 16bit) */
#define i386gAVL     0x10   /* segment AVL is set */

void i386SetSegment(void *entry,
                    uint32 base, uint32 limit,
                    uint8 rights, uint8 gran);
void i386SetTaskGate(void *entry, uint16 selector, uint8 rights);

void i386ltr(uint32 selector);
void i386lidt(uint32 base, uint32 limit);
void i386lgdt(uint32 base, uint32 limit);
uint32 *i386sgdt(uint32 *limit);

#define I386_PAGING_ON() asm("push %eax; mov %cr0, %eax; orl $0x80000000,%eax ; mov %eax, %cr0 ; pop %eax");
#define I386_PAGING_OFF() asm("push %eax; mov %cr0, %eax; andl $0x7FFFFFFF,%eax ; mov %eax, %cr0 ; pop %eax");

void unmap_irqs(void);
void remap_irqs(void);
void unmask_irq(int irq);
void mask_irq(int irq);
void init_timer(void);

void cli (void);
void sti (void);
void local_flush_tlb (void);
void local_flush_pte (unsigned int addr);

#endif /* _I386_H */

