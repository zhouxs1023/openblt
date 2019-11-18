/* $Id: //depot/blt/kernel/kernel.c#8 $
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
#include "boot.h"
#include "port.h"

#include <i386/io.h>
#include <string.h>
#include "resource.h"
#include "aspace.h"
#include "task.h"
#include "queue.h"
#include "smp.h"

#include "assert.h"

void restore_idt(void);
void init_idt(char *idt);

unsigned char IP[4];

aspace_t *flat;
boot_dir *bdir;

char *idt;
char *gdt;
char *gdt2;
char *toppage;
task_t *current;
task_t *kernel;
uint32 gdt2len;
uint32 entry_ebp;
uint32 _cr3;
task_t *idle_task;
int live_tasks = 0;

queue_t *run_queue;

uint32 memsize; /* pages */
uint32 memtotal;

static uint32 nextentry = 9*8;
static uint32 nextmem = 0x80400000 - 3*4096;
static int uberportnum = 0;

uint32 getgdtentry(void)
{
    nextentry += 8;    
    return nextentry;
}

/* return phys page number of first page of allocated group */
uint32 getpages(int count)
{
    memsize -= count;
    Assert(memsize > 512);
    
    return memsize;
}

/* alloc count physical pages, map them into kernel space, return virtaddr AND phys */
void *kgetpages2(int count, int flags, uint32 *phys)
{
    nextmem -= 4096*count;
    *phys = getpages(count);
    aspace_maphi(flat, *phys, nextmem/0x1000, count, flags);
    *phys *= 4096;
    return (void *) nextmem;
}

/* alloc count physical pages, map them into kernel space, return virtaddr */
void *kgetpages(int count, int flags)
{
    nextmem -= 4096*count;
    aspace_maphi(flat, getpages(count), nextmem/0x1000, count, flags); 
    return (void *) nextmem;
}

/* map specific physical pages into kernel space, return virtaddr */
void *kmappages(int phys, int count, int flags)
{
    nextmem -= 4096*count;
    aspace_maphi(flat, phys, nextmem/0x1000, count, flags); 
    return (void *) nextmem;
}

void destroy_kspace(void)
{
/*  memory_status();*/
    
    asm("mov %0, %%cr3"::"r" (_cr3));   /* insure we're in kernel map */
    restore_idt();
    
    i386lgdt((uint32)gdt2,gdt2len);    
    asm ("mov %0, %%esp; ret": :"r" (entry_ebp + 4));
}

void panic(char *reason)
{
    kprintf("");
    kprintf("PANIC: %s",reason);    
    asm("hlt");    
}

void idler(void)
{    
    for(;;){
        asm("hlt");        
    }    
}

static aspace_t _flat;

void init_kspace(void)
{
    TSS *ktss;
	uint32 *raw;
	
    /* there's an existing aspace at 0xB03FD000 that was initialized
       by the 2nd stage loader */
	raw = (uint32 *) 0x803FD000;
    flat = &_flat;
	flat->pdir = raw;
	flat->ptab = &raw[1024];
	flat->high = &raw[2048];
	memtotal = memsize;
	
    memsize -= 3; /* three pages already in use */
    
    toppage = (char *) kgetpages(3,3);
    gdt = (char *) kgetpages(1,3);
    rsrc_init(kgetpages(6,3),4096*6);
    memory_init();
    

	kernel = (task_t *) kmalloc128();
	kernel->tss = (ktss = (TSS *) kgetpages(1,3));
    kernel->flags = tKERNEL;
	kernel->rsrc.id = 0;
	kernel->rsrc.owner = NULL;
	kernel->rsrc.rights = NULL;
    current = kernel;

    init_idt(idt = kgetpages(1,3));      /* init the new idt, save the old */
    gdt2 = (void *) i386sgdt(&gdt2len);        /* save the old gdt */

    i386SetSegment(gdt + SEL_KCODE,      /* #01 - 32bit DPL0 CODE */
                   0, 0xFFFFF,
                   i386rPRESENT | i386rDPL0 | i386rCODE,
                   i386g4K | i386g32BIT);

    i386SetSegment(gdt + SEL_KDATA,      /* #02 - 32bit DPL0 DATA */
                   0, 0xFFFFF,
                   i386rPRESENT | i386rDPL0 | i386rDATA,
                   i386g4K | i386g32BIT);

    i386SetSegment(gdt + SEL_KDATA2,        /* #02 - 32bit DPL0 DATA */
                   0, 0xFFFFF,
                   i386rPRESENT | i386rDPL0 | i386rDATA,
                   i386g4K | i386g32BIT);

    i386SetSegment(gdt + SEL_UCODE,        /* #03 - 32bit DPL3 CODE */
                   0, 0xFFFFF,
                   i386rPRESENT | i386rDPL3 | i386rCODE,
                   i386g4K | i386g32BIT);

    i386SetSegment(gdt + SEL_UDATA,        /* #04 - 32bit DPL3 DATA */
                   0, 0xFFFFF,
                   i386rPRESENT | i386rDPL3 | i386rDATA,
                   i386g4K | i386g32BIT);

    i386SetSegment(gdt + SEL_KTSS,        /* #05 - DPL0 TSS (Kernel) */
                   (uint32) ktss, 104,
                   i386rPRESENT | i386rDPL0 | i386rTSS,
                   0);
    
    i386lgdt((uint32) gdt, 1024/8);

    asm("mov %%cr3, %0":"=r" (_cr3));
    
        /* setup the kernel TSS to insure that it's happy */
    ktss->cs = SEL_KCODE;
    ktss->ds = ktss->es = ktss->ss = ktss->fs = ktss->gs = SEL_KDATA;
    ktss->ldts = ktss->debugtrap = ktss->iomapbase = 0;
    ktss->cr3 = _cr3;
    i386ltr(SEL_KTSS);

    run_queue = queue_new(0);    
}


void preempt(void)
{
    current->flags = tREADY;

    current = queue_peekHead(run_queue);
    current->flags = tRUNNING;
    
    i386SetSegment(gdt + SEL_UTSS /*current->tid */,    /* XXX - YUCK */
                   (uint32) current->tss, 104,
                   i386rPRESENT | i386rDPL3 | i386rTSS,
                   0);
    task_call(current);
    
        /* clear the NT bit */
    asm("pushf ; pop %%eax ; andl $0xBFFF, %%eax ; push %%eax ;; popf":::"ax");
}

void swtch(void)
{
    int x,old;    

#ifdef __SMP__
		if (smp_my_cpu ())
			return;
#endif

    current = queue_removeHeadT(run_queue, &x, task_t*);
    
        /*kprintf("swtch(): %d -> ",current->rid);*/
    
    old = current->rsrc.id;
    
    if((current->flags == tRUNNING) || (current->flags == tREADY)) {
        current->flags = tREADY;
        if(current != idle_task) queue_addTail(run_queue, current, 0);        
    }

    current = queue_peekHead(run_queue);

    if(!current) {
        if(live_tasks){
            current = idle_task;
            queue_addTail(run_queue, current, 0);
        } else {
            kprintf("");
            kprintf("No runnable tasks.  Exiting.");
            destroy_kspace();
        }
    }
    
/*    kprintf("%d\n",current->rid);*/
    
    current->flags = tRUNNING;
    
    i386SetSegment(gdt + SEL_UTSS /* current->tid */,    /* XXX - YUCK */
        (uint32) current->tss, 104, i386rPRESENT | ((current==idle_task) ?
        i386rDPL0 : i386rDPL3) | i386rTSS, 0);
    
/*    kprintf("swtch() A %X -> %X\n",old,current->rid);*/
    if(old != current->rsrc.id){
        task_call(current);
            /* clear the NT bit */
        asm("pushf ; pop %%eax ; andl $0xBFFF, %%eax ; push %%eax ;; popf"
            :::"ax");
    }
/*    kprintf("swtch() B -> %X\n",current->rid);    */
}

task_t *new_thread(aspace_t *a, uint32 ip, int kernelspace)
{
    task_t *t;
    int i;

    for(i=1023;i>0;i--){
        if(!a->ptab[i]) break;
    }
    if(!i) {
            /* XXX kprintf("\nTask died in birth...\n"); */
    }
    aspace_map(a, getpages(1), i, 1, 7);  /* map in a 4k stack */
    t = task_create(a, ip, i*4096+4092, kernelspace);         
	rsrc_bind(&t->rsrc, RSRC_TASK, kernel);
    t->flags = tREADY;
    if(!kernelspace) {
        queue_addTail(run_queue, t, 0);
        live_tasks++;
    }
    
    return t;
}

int brk(uint32 addr)
{
    int i;
    aspace_t *a = current->addr;

    addr /= 4096;
    
    if(addr > 512) return -1;
    for(i=0;i<=addr;i++){
        if(!(a->ptab[i])){
            aspace_map(a, getpages(1), i, 1, 7);            
        }
    }

    return 0; /* XXX */
}

/*
  0 = directory
  1 = bootstrap
  2 = kernel
*/

void go_kernel(void)
{
    task_t *t;
    int i,n;    
    aspace_t *a;
    
    port_t *uberport;    
    uberport = rsrc_find_port(uberportnum = port_create(0));    

    kprintf("uberport allocated with rid = %d",uberportnum);
	
    for(i=3;bdir->bd_entry[i].be_type;i++){
        a = aspace_create();        
        aspace_map(a, bdir->bd_entry[i].be_offset+0x100, 0,
                   bdir->bd_entry[i].be_size, 7);
        aspace_map(a, getpages(2), bdir->bd_entry[i].be_size, 2, 7);
        
		aspace_map(a, 0xB0, 0xB0, 2, 7);            /* map in video ram */ 
		aspace_map(a, 0xB8, 0xB8, 2, 7);            /* map in video ram */ 
        aspace_map(a, 0xA0, 0xA0, 16, 7);           /* EXTREME */
        
        t = new_thread(a, bdir->bd_entry[i].be_code_ventr, 0);
		/* make the thread own it's addressspace */
		rsrc_set_owner(&a->rsrc, t);
        if(i == 3) {
			rsrc_set_owner(&uberport->rsrc, t);
		}
		
        for(n=0;(t->name[n] = bdir->bd_entry[i].be_name[n]);n++);

        kprintf("task %X @ 0x%x, size = 0x%x (%s)",t->rsrc.id,
                bdir->bd_entry[i].be_offset*4096+0x100000,
                bdir->bd_entry[i].be_size*4096,
                t->name);

        if(!strcmp(bdir->bd_entry[i].be_name,"ne2000")){
            unsigned char *x =
                (unsigned char *) (bdir->bd_entry[i].be_offset*4096+0x100020);
            x[0] = 'i';
            x[1] = 'p';
            x[2] = IP[0];
            x[3] = IP[1];
            x[4] = IP[2];
            x[5] = IP[3];            
        }
    }

    kprintf("creating idle task...");    
    a = aspace_create();	
    idle_task = new_thread(a, (int) idler, 1);
    rsrc_set_owner(&a->rsrc, idle_task);
	strcpy(idle_task->name,"idler");
	strcpy(kernel->name,"openblt kernel");

#ifdef __SMP__
    smp_init ();
#endif

    k_debugger();
    kprintf("starting scheduler...");    

#ifdef __SMP__
		if (smp_configured)
			{
				smp_final_setup ();
				kprintf ("smp: signaling other processors");
				smp_begin ();
			}
#endif

		/*
		 * when the new vm stuffas are done, we can at this point discard any
		 * complete pages in the .text.init and .data.init sections of the kernel
		 * by zeroing them and adding them to the free physical page pool.
		 */
	
    swtch();

    kprintf("panic: returned from scheduler?");
    asm("hlt");
}

struct _kinfo
{
    uint32 memsize;
    uint32 entry_ebp;
    boot_dir *bd;
    unsigned char *params;
} *kinfo = (struct _kinfo *) 0x80000000;

void kmain(void)
{
    int n;
    memsize = (kinfo->memsize) / 4096;
    entry_ebp = kinfo->entry_ebp;
    bdir = kinfo->bd;

    init_timer();
    init_kspace();
    
    kprintf_init();

    kprintf("OpenBLT Release I (built %s, %s)", __DATE__, __TIME__);
    kprintf("    Copyright (c) 1998 The OpenBLT Dev Team.  All rights reserved.");
    kprintf("");
    kprintf("system memory 0x%x",memsize*4096);

    if(kinfo->params[0] == 'i' && kinfo->params[1] == 'p' &&
       kinfo->params[2] == '=') {
        kprintf("ip = %d.%d.%d.%d",
                kinfo->params[3],kinfo->params[4],
                kinfo->params[5],kinfo->params[6]
                );
        IP[0] = kinfo->params[3];
        IP[1] = kinfo->params[4];
        IP[2] = kinfo->params[5];
        IP[3] = kinfo->params[6];        
    }

    n = ((uint32) toppage) + 4080;
    
    asm("mov %0, %%esp"::"r" (n) );
    
    kprintf("kernel space initialized");    
    go_kernel();
}

