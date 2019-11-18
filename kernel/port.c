/* $Id: //depot/blt/kernel/port.c#2 $
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
#include "port.h"
#include "resource.h"
#include "queue.h"

#include "assert.h"

extern queue_t *run_queue;

int port_create(int restrict)
{
    port_t *p;

        /* create new port */
    p = kmalloc64();
    
    p->msgcount = 0;
    p->first = p->last = NULL;
    p->slaved = 0;
    p->refcount = 1;
	
	rsrc_bind(&p->rsrc, RSRC_PORT, current);
    return p->rsrc.id;
}

int port_destroy(int port)
{
    port_t *p;
    if(!(p = rsrc_find_port(port))) return ERR_RESOURCE;
    if(p->rsrc.owner != current) return ERR_PERMISSION;

    if(p->refcount == 1) {                
            /* destroy port */
		rsrc_release(&p->rsrc);
        kfree64(p); /* XXX check */
        return ERR_NONE;    
    }

        /* port is the master of one or more slaves */
    return ERR_RESOURCE;       
}

#define PORT_OPT_NOWAIT        1
#define PORT_OPT_SETRESTRICT   2
#define PORT_OPT_SETDEFAULT    3
#define PORT_OPT_SLAVE         4

uint32 port_option(uint32 port, uint32 opt, uint32 arg)
{
    port_t *p;
    
    if(!(p = rsrc_find_port(port))) return ERR_RESOURCE;
    if(p->rsrc.owner != current) return ERR_PERMISSION;

    if(opt == PORT_OPT_SETRESTRICT){
/*XXX        p->restrict = arg;        */
        return ERR_NONE;        
    }

    if(opt == PORT_OPT_SLAVE){
        port_t *master;

        if(arg){
                /* arg == 0 will simply release the old master */
            
            if(!(master = rsrc_find_port(arg))) return ERR_RESOURCE;
            if(master->rsrc.owner != current) return ERR_PERMISSION;
            
                /* indicate that our master has one more slave */
            master->refcount++;
        }
        
        if(p->slaved){
                /* change in slave status, deref our old master */
            if(!(master = rsrc_find_port(p->slaved)))
                panic("port_option(): master port not found?");
            
            master->refcount--;            
        }
        p->slaved = arg;
        return ERR_NONE;        
    }
    return ERR_PERMISSION;
}

static chain_t *msg_pool = NULL;


/*int port_send(int from, int port, void *msg, int size)*/
int port_send(msg_hdr_t *mh)
{
    int i,size;
    message_t *m;
    void *msg;
    
    port_t *f,*p;

    if(((uint32) mh) > 0x400000) return ERR_MEMORY;
    size = mh->size;    
    msg = mh->data;
    

    if(!(f = rsrc_find_port(mh->src))) return ERR_SENDPORT;
#if 0
 	if(f->rsrc.owner != current) {
        task_t *t = current->rsrc.owner; /* XXX */
        while(t){
            if(t == f->rsrc.owner) break;
            t = t->rsrc.owner;
            
        }
        if(!t) return ERR_PERMISSION;
    }
#endif
        /* insure the port exists and we may send to it */
    if(!(p = rsrc_find_port(mh->dst))) return ERR_RECVPORT;
/*    if((p->restrict) &&
       (p->restrict != mh->src)) return ERR_PERMISSION; XXX */

        /* are we slaved to a different port? */
    if(p->slaved){
        if(!(p = rsrc_find_port(p->slaved))) return ERR_RESOURCE;
        if(p->slaved) return ERR_RESOURCE;
/*        if((p->restrict) &&
           (p->restrict != mh->src)) return ERR_PERMISSION;  XXX */      
    }
        /* ignore invalid sizes/locations */
    if( (size < 1) ||
        (((uint32) msg) > 0x400000) ||
        (size > 4096)) return ERR_MEMORY;    

    m = kmalloc32();
    
        /* allocate a 4k page to carry the message. klunky... */
    if(size < 1025){
        m->data = kmallocB(size);        
    } else {
        if(msg_pool){
            m->data = (void *) msg_pool;
            msg_pool = (chain_t *) msg_pool->next;        
        } else {
            m->data = kgetpages(1,3);
        }
    }

/*    kprintf("task %X: copyin %x -> %x (%d)\n",current->rid,
            (int) msg, (int) m->data,  size);*/
    
    for(i=0;i<size;i++)
        ((unsigned char *) m->data)[i] = *((unsigned char *) msg++);

    m->from_port = mh->src;
    m->to_port = mh->dst;    
    m->size = size;
    m->next = NULL;
    if(p->last){
        p->last->next = m;
    } else {
        p->first = m;
    }
    p->last = m;
    p->msgcount++;

        /* wake our owner if he's sleeping on us */
    if(p->rsrc.owner->flags == tSLEEP_PORT &&
       p->rsrc.owner->sleeping_on == mh->dst) {
/*        kprintf("task %X: waking task %X on port %d\n",current->rid,
                p->owner->rid,mh->dst);        */
        p->rsrc.owner->flags = tREADY;
        queue_addTail(run_queue, p->rsrc.owner, 0);        
    }
    
    return size;
}

/*int port_recv(int port, void *msg, int size, int *from)*/
int port_recv(msg_hdr_t *mh)
{
    int i,size;
    message_t *m;
    void *msg;    
    port_t *p;

    if(((uint32) mh) > 0x400000) return ERR_MEMORY;
    size = mh->size;    
    msg = mh->data;

        /* insure the port exists and we may receive on it */
    if(!(p = rsrc_find_port(mh->dst))) return ERR_RECVPORT;
#if 0
	    if(p->rsrc.owner != current) return ERR_PERMISSION;
#endif
 
        /* bounds check the message... should be more careful */
    if(((uint32) msg) > 0x400000) return ERR_MEMORY;

        /* no messages -- sleep */
    while(!p->msgcount) {
        current->sleeping_on = mh->dst;
        current->flags = tSLEEP_PORT;
/*        kprintf("task %X: sleeping on port %d\n",current->rid,mh->dst);      */  
        swtch();        
/*        if(i != current->rid) panic("who am i?"); 
        kprintf("task %X: waking up\n",current->rid);        */
    }
    
    m = p->first;
/*    kprintf("task %X: copyout %x -> %x (%d/%d)\n",current->rid,
            (int) m->data, (int) msg, m->size, size);
            */  
    Assert(m->size < 4096);
    for(i=0;i<m->size && (i <size);i++){
        *((unsigned char *) msg++) = ((unsigned char *) m->data)[i];
    }
    mh->src = m->from_port;    
    mh->dst = m->to_port;
    
        /* unchain from the head of the queue */
    if(!(p->first = p->first->next)) p->last = NULL;    
    
    p->msgcount--;

        /* add to the freepool */
    if(m->size < 1025){
        kfreeB(m->size,m->data);
    } else {
        ((chain_t *) m->data)->next = msg_pool;
        msg_pool = ((chain_t *) m->data);
    }
    kfree32(m);
/*  kprintf("       : DONE\n");
 */
    return size < m->size ? size : m->size;

}

