/* $Id: //depot/blt/kernel/queue.c#2 $
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
#include "queue.h"
#include "memory.h"

queue_t *queue_new(int limit)
{
    queue_t *q = kmalloc(KM16);
    q->limit = limit;
    q->count = 0;
    q->head = q->tail = NULL;
    return q;
}

void queue_del(queue_t *q)
{
    kfree(KM16,q);    
}

void queue_addHead(queue_t *q, void *data, int dsize)
{
    qnode_t *qn = kmalloc(KM16);
    qn->data = data;
    qn->dsize = dsize;

    if(q->head) {
        qn->next = q->head;
        qn->prev = NULL;
        q->head = q->head->prev = qn;        
    } else {
        q->head = q->tail = qn;
        qn->next = qn->prev = NULL;        
    }

    q->count++;    
}

void queue_addTail(queue_t *q, void *data, int dsize)
{
    qnode_t *qn = kmalloc(KM16);
    qn->data = data;
    qn->dsize = dsize;

    if(q->head) {
        qn->prev = q->head;
        qn->next = NULL;
        q->tail = q->tail->next = qn;        
    } else {
        q->head = q->tail = qn;
        qn->next = qn->prev = NULL;        
    }

    q->count++;    
}

void *queue_removeHead(queue_t *q, int *dsize)
{
    qnode_t *qn;    
    void *block;
    
    if(q->head){
        qn = q->head;
        q->count--;
        
        if(q->tail == q->head){
            q->head = q->tail = NULL;            
        } else {
            qn->next->prev = NULL;
            q->head = qn->next;            
        }
        
        if(dsize) *dsize = qn->dsize;
        block = qn->data;
        kfree(KM16,qn);
        return block;        
    } else {
        return NULL;        
    }
}

void *queue_removeTail(queue_t *q, int *dsize)
{
    qnode_t *qn;    
    void *block;
    
    if(q->head){
        qn = q->head;
        q->count--;
        
        if(q->tail == q->head){
            q->head = q->tail = NULL;            
        } else {
            qn->prev->next = NULL;
            q->tail = qn->prev;            
        }
        
        if(dsize) *dsize = qn->dsize;
        block = qn->data;
        kfree(KM16,qn);
        return block;        
    } else {
        return NULL;        
    }
}


