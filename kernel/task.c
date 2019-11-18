/* $Id: //depot/blt/kernel/task.c#2 $
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

#include "kernel.h"
#include "memory.h"
#include "task.h"

extern char *gdt;

/* create a new task, complete with int stack */
task_t *task_create(aspace_t *a, uint32 ip, uint32 sp, int kernel) 
{
    task_t *t = kmalloc128();
	
	t->tss = (TSS *) kgetpages(1,7);
    
    t->tss->esp0 = (uint32) ( ((char *) t->tss) + 4092 );
    t->tss->ss0  = SEL_KDATA;
    t->tss->eax = 0xDEADBEEF;
    t->tss->ebx = 0xDEADBEEF;
    t->tss->ecx = 0xDEADBEEF;
    t->tss->edx = 0xDEADBEEF;
    t->tss->esp1 = t->tss->esp2 = t->tss->ss1 = t->tss->ss2 = 0;
    t->tss->cr3  = (((uint32) a->pdir[0]) & 0xFFFFFF00) - 4096;
    t->tss->eip  = (uint32) ip;
    t->tss->eflags =/* EXTREME!  0x4202 */ 0x7202;
    t->tss->esp  = (uint32) sp;
    t->tss->cs   = ( kernel ? (SEL_KCODE) : (SEL_UCODE | 3) );
    t->tss->ds = t->tss->es = t->tss->ss =
        t->tss->fs = t->tss->gs = (kernel ? SEL_KDATA : (SEL_UDATA | 3) );
    t->tss->ldts = t->tss->debugtrap = t->tss->iomapbase = 0;
	t->resources = NULL;
    t->addr = a;
    
/*    i386SetSegment(gdt + t->tid,        
                   (uint32) t, 104,
                   i386rPRESENT | (kernel ? i386rDPL0 : i386rDPL3) | i386rTSS,
                   0);
                   */
    
    t->name[0] = 0;
    t->irq = 0;
    
    return t;
}

void task_call(task_t *t)
{
    uint32 sel[2]; 
    sel[1] = SEL_UTSS /*t->tid*/;    
    __asm__("lcall %0; clts"::"m" (*sel));
}

