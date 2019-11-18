/* $Id: //depot/blt/kernel/syscall.c#6 $
**
** Copyright 1998 Brian J. Swetland
** All rights reserved.
** Copyright 1998 Sidney Cammeresi
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
#include <i386/io.h>
#include "kernel.h"
#include "queue.h"
#include "memory.h"
#include "resource.h"
#include "boot.h"
#include "aspace.h"

#include "smp.h"

#include <blt/syscall_id.h>

extern task_t *irq_task_map[16];

typedef struct { uint32 edi, esi, ebp, esp, ebx, edx, ecx, eax; } regs;
void k_debugger(regs *r, uint32 eip, uint32 cs, uint32 eflags);

extern int live_tasks;

extern boot_dir *bdir;

void terminate(void)
{
    task_t *t = current;
    
    kprintf("Task %X terminated.",current->rsrc.id);
    live_tasks--;    
    current->flags = tDEAD;
    swtch();
    kprintf("panic: HUH? Back from the dead? %x / %x",t,current);
    asm("hlt");
}



struct _ll time_first = { NULL, NULL, 0, NULL };


void sleep(int ticks)
{
    struct _ll *l,*n;
    int when = ticks + kernel_timer;

    if(when > kernel_timer){
        n = kmalloc16();
        n->t = current;
        n->when = when;
        if(!time_first.next){
            time_first.next = n;
            n->next = NULL;
            n->prev = NULL;
        } else { 
            for(l = time_first.next;l;l=l->next){
                if(when < l->when){
                    n->prev = l->prev;
                    n->next = l;
                    l->prev = l;
                    l->prev->next = n;
                    break;
                }
                if(!(l->next)){
                    l->next = n;
                    n->prev = l;
                    n->next = NULL;
                }
            }
        }
        current->flags = tSLEEP_TIMER;
/*    kprintf("sleeping %d / %d -> %d\n",current->rid, kernel_timer, when);
 */
    }
    swtch();
}

#define p_uint32(n) (esp[n])
#define p_voidptr(n) ((void *) esp[n])
#define p_charptr(n) ((char *) esp[n])
#define res r.eax

void syscall(regs r, uint32 eip, uint32 cs, uint32 eflags, uint32 *esp)
{
		int i, len, bdir_base;
    unsigned int config;

    switch(r.eax){

    case BLT_SYS_os_thread :
    {
        int i;    
        task_t *t;
        
        t = new_thread(current->addr, p_uint32(1), 0);
        for(i=0;current->name[i] && i<31;i++) t->name[i] = current->name[i];
        if(i<30){
            t->name[i++] = '+';
        }
        t->name[i] = 0;    
        t->rsrc.owner = current;
        
        res = t->rsrc.id;
    }
    break;

    case BLT_SYS_os_terminate :
        kprintf("task %X: os_terminate(%x)",current->rsrc.id,r.eax);
        if(p_uint32(1) == 0x00420023) {
            kprintf("");
            kprintf("Kernel terminated by user process");
            destroy_kspace();
        }
        terminate();    
        break;

    case BLT_SYS_os_console :
        kprintf("task %X> %s",current->rsrc.id,p_charptr(1));
        break;

    case BLT_SYS_os_brk :
        res = brk(p_uint32(1));
        break;

    case BLT_SYS_os_handle_irq :
            /* thread wants to listen to irq in eax */
        if(p_uint32(1) > 0 && p_uint32(1) < 16) {
            current->irq = p_uint32(1);
            irq_task_map[p_uint32(1)] = current;
        };
        
        current->tss->eflags |= 2<<12 | 2<<13;    
        eflags |= 2<<12 | 2<<13;
        
        swtch();
        break;

    case BLT_SYS_os_sleep_irq :
        if(current->irq){
                /* sleep */
            current->flags = tSLEEP_IRQ;
            unmask_irq(current->irq);
        }
        swtch();
        break;

    case BLT_SYS_os_debug :
#ifdef __SMP__
        if (smp_configured)
          {
            config = apic_read (APIC_LVTT);
            apic_write (APIC_LVTT, config | 0x10000);
            smp_begun = 0;
            ipi_all_but_self (IPI_STOP);
          }
#endif
        k_debugger(&r, eip, cs, eflags);
#ifdef __SMP__
				if (smp_configured)
          {
            smp_begin ();
            apic_write (APIC_LVTT, config);
          }
#endif
        break;

    case BLT_SYS_os_sleep :
        sleep(p_uint32(1));
        break;
	
	case BLT_SYS_os_identify :
		res = rsrc_identify(p_uint32(1));
		break;
		
    case BLT_SYS_sem_create :
        res = sem_create(p_uint32(1));
        break;

    case BLT_SYS_sem_destroy :
        res = sem_destroy(p_uint32(1));
        break;

    case BLT_SYS_sem_acquire :
        res = sem_acquire(p_uint32(1));
        break;

    case BLT_SYS_sem_release :
        res = sem_release(p_uint32(1));
        break;

    case BLT_SYS_port_create :
        res = port_create(p_uint32(1));
        break;

    case BLT_SYS_port_destroy :
        res = port_destroy(p_uint32(1));    
        break;

    case BLT_SYS_port_option :
        res = port_option(p_uint32(1), p_uint32(2), p_uint32(3));
        break;

    case BLT_SYS_port_send :
        res = port_send(p_voidptr(1));
        break;

    case BLT_SYS_port_recv :
        res = port_recv(p_voidptr(1));
        break;        
		
	case BLT_SYS_right_create :
		res = right_create(p_uint32(1), p_uint32(1));
		break;
		
	case BLT_SYS_right_destroy :
		res = right_destroy(p_uint32(1));
		break;
		
	case BLT_SYS_right_revoke :
		res = right_revoke(p_uint32(1), p_uint32(2));
		break;
		
	case BLT_SYS_right_grant :
		res = right_grant(p_uint32(1), p_uint32(2));
		break;		

	case BLT_SYS_get_bootfs :
		bdir_base = (unsigned int) bdir >> 12;
		aspace_map (current->addr, bdir_base, bdir_base, 1, 5);
		for (i = 0, len = 0; (bdir->bd_entry[i].be_type != BE_TYPE_NONE); i++)
			len += bdir->bd_entry[i].be_size;
		aspace_map (current->addr, bdir_base + 1, bdir_base + 1, len, 5);
		res = (unsigned int) bdir;
		break;
    }
}
