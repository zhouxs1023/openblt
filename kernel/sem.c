/* $Id: //depot/blt/kernel/sem.c#2 $
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
#include "resource.h"
#include "memory.h"

extern queue_t *run_queue;

int sem_create(int count) 
{
    sem_t *s = (sem_t *) kmalloc32();
    s->queue = queue_new(0);
    s->count = count;
	rsrc_bind(&s->rsrc, RSRC_SEM, current);
	
    return s->rsrc.id;
}

int sem_destroy(int sem)
{

}

int sem_acquire(int id) 
{
    sem_t *s;
    
    if(!(s = rsrc_find_sem(id))) {
        return ERR_RESOURCE;
    }
    
    if(s->count > 0 ){
        s->count--;
    } else {
        s->count--;
        queue_addTail(s->queue,current,0);
        current->sleeping_on = id;
        current->flags = tSLEEP_SEM;
        swtch();
    }
    return ERR_NONE;
}

int sem_release(int id) 
{
    int x;
    sem_t *s;
    task_t *t;
    
    if(!(s = rsrc_find_sem(id))) {
        return ERR_RESOURCE;
    }

    s->count++;

    if(t = queue_removeHeadT(s->queue,&x,task_t*)){
        t->flags = tREADY;
        queue_addTail(run_queue, t, 0);            
        preempt();             
    }

    return ERR_NONE;
}
