/* $Id: //depot/blt/kernel/port.h#2 $
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

#ifndef _PORT_H_
#define _PORT_H_

#include "resource.h"
 
#include "types.h"

typedef struct __msg_hdr_t {
    int flags;
    int src;
    int dst;
    int size;
    void *data;    
} msg_hdr_t;

typedef struct __chain_t 
{
    struct __chain_t *next;
} chain_t;

typedef struct __message_t {
    struct __message_t *next;
    uint32 size;
    int from_port;
    int to_port;    
    void  *data;    
} message_t;

struct __port_t 
{
	resource_t rsrc;
	
    int msgcount;          /* number of messages waiting in the queue */
    int slaved;            /* deliver my messages to a master port */     
    message_t *first;      /* head of the queue */
    message_t *last;       /* tail of the queue */
    int refcount;          /* counts owner and any slaved ports */
};


int port_create(int restrict);
int port_destroy(int port);
uint32 port_option(uint32 port, uint32 opt, uint32 arg);
int port_send(msg_hdr_t *mh);
int port_recv(msg_hdr_t *mh);

#endif
