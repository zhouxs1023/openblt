/* $Id: //depot/blt/kernel/queue.h#2 $
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

#ifndef _QUEUE_H_
#define _QUEUE_H_

typedef struct _qnode_t 
{
    struct _qnode_t *next;
    struct _qnode_t *prev;
    void *data;
    int dsize;
} qnode_t;

typedef struct _queue_t 
{
    int count;
    int limit;    
    struct _qnode_t *head;
    struct _qnode_t *tail;
} queue_t;

queue_t *queue_new(int limit);
void queue_del(queue_t *q);

void queue_addHead(queue_t *q, void *data, int dsize);
void queue_addTail(queue_t *q, void *data, int dsize);

void *queue_removeHead(queue_t *q, int *dsize);
void *queue_removeTail(queue_t *q, int *dsize);

#define queue_removeHeadT(q,s,t)  ((t) queue_removeHead(q,s))
#define queue_removeTailT(q,s,t)  ((t) queue_removeTail(q,s))

#define queue_peekHead(q) ( q->head ? q->head->data : NULL )
#define queue_peekTail(q) ( q->head ? q->head->data : NULL )

#define queue_peekHeadT(q,t) ((t) ( q->head ? q->head->data : NULL ))
#define queue_peekTailT(q,t) ((t) ( q->head ? q->head->data : NULL ))

#define queue_count(q) ((q)->count)
#define queue_limit(q) ((q)->count)

#endif
