/* $Id: //depot/blt/kernel/resource.c#2 $
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
#include "resource.h"

typedef struct _rtab {
	resource_t *resource;
	int next;
} rtab;

static resource_t null_resource;
	
static rtab *rmap;
static uint32 rmax = 0;
static uint32 rfree = 0;

resnode_t *resource_list = NULL;


void rsrc_init(void *map, int size)
{
    int i;

	null_resource.id = 0;
	null_resource.type = RSRC_NONE;
	null_resource.owner = NULL;
	null_resource.rights = NULL;
	    
    rfree = 1;    
    rmax = size / sizeof(resource_t);
    rmap = (rtab *) map;
    for(i = 0; i < rmax; i++) {
        rmap[i].resource = &null_resource;
        rmap[i].next = i+1;        
    }
    rmap[rmax-1].next = 0;            
}

void *rsrc_find(int type, int id)
{    
    if((id < rmax) && (rmap[id].resource->type == type)) return rmap[id].resource;
    return NULL;    
}

void rsrc_set_owner(resource_t *r, task_t *owner) 
{
	resnode_t *rn;
	
	if(r->owner){
		/* unchain it from the owner */
		for(rn = r->owner->resources; rn; rn=rn->next){
			if(rn->resource == r){
				if(rn->prev) {
					rn->prev->next = rn->next;
				} else {
					r->owner->resources = rn->next;
				}
				if(rn->next) {
					rn->next->prev = rn->prev;
				}
				kfree16(rn);
				break;
			}
		}		
	}
	
	r->owner = owner;

	if(owner){
		rn = (resnode_t *) kmalloc16();
		rn->resource = r;
		rn->prev = NULL;
		rn->next = owner->resources;
		owner->resources = rn;	
	}
}


int rsrc_identify(uint32 id) 
{
	if((id >= rmax) || (rmap[id].resource->type == RSRC_NONE)) return 0;
	return rmap[id].resource->owner->rsrc.id; 
}


void rsrc_bind(resource_t *rsrc, rsrc_type type, task_t *owner)
{
    uint32 id;
    resnode_t *rn;
	
    if(rfree){
        id = rfree;
        rfree = rmap[rfree].next;
    } else {
        panic("resource exhaustion");
    }

    rmap[id].resource = rsrc;
	rsrc->id = id;
	rsrc->type = type;
	rsrc->owner = owner;
	rsrc->rights = NULL;
	
	if(owner){
		rn = (resnode_t *) kmalloc16();
		rn->resource = rsrc;
		rn->prev = NULL;
		rn->next = owner->resources;
		owner->resources = rn;	
	}
	rn = (resnode_t *) kmalloc16();
	rn->resource = rsrc;
	rn->prev = NULL;
	rn->next = resource_list;
	resource_list = rn;
}

void rsrc_release(resource_t *r)
{
	uint32 id = r->id;
	resnode_t *rn;
	
	/* unchain it from the global pool */
	for(rn = resource_list; rn; rn=rn->next){
		if(rn->resource == r){
			if(rn->prev) {
				rn->prev->next = rn->next;
			} else {
				resource_list = rn->next;
			}
			if(rn->next) {
				rn->next->prev = rn->prev;
			}
			kfree16(rn);
			break;
		}
	}
	
	if(r->owner){
		/* unchain it from the owner */
		for(rn = r->owner->resources; rn; rn=rn->next){
			if(rn->resource == r){
				if(rn->prev) {
					rn->prev->next = rn->next;
				} else {
					r->owner->resources = rn->next;
				}
				if(rn->next) {
					rn->next->prev = rn->prev;
				}
				kfree16(rn);
				break;
			}
		}	
	}
	
	
	r->type = RSRC_NONE;
	r->id = 0;
	rmap[id].resource = &null_resource;
	rmap[id].next = rfree;
	rfree = id;
}


