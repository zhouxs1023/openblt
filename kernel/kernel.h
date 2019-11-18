/* $Id: //depot/blt/kernel/kernel.h#4 $
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

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "types.h"
#include "i386.h"
#include "aspace.h"
#include "task.h"
#include "port.h"
#include "sem.h"

#include <blt/types.h>
#include <blt/error.h>

uint32 getgdtentry(void);
void *kgetpages(int count, int flags);
void *kgetpages2(int count, int flags, uint32 *phys);

void panic(char *reason);

extern int kernel_timer;

task_t *new_thread(aspace_t *a, uint32 ip, int kernel);

int brk(uint32 addr);

extern task_t *kernel, *current;
void destroy_kspace(void);

/* debugger functions */
void kprintf_init(void);
void kprintf(char *fmt, ...);
char *kgetline(char *line, int maxlen);
void krefresh(void);

void preempt(void);
void swtch(void);


/* XXX HACK */
struct _ll
{
    struct _ll *next, *prev;
    int when;
    task_t *t;
};

extern struct _ll time_first;

extern char *idt, *gdt;
extern uint32 _cr3;

#endif
