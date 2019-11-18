/* $Id: //depot/blt/kernel/aspace.c#2 $
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
#include "aspace.h"

extern uint32 _cr3;
extern aspace_t *flat;

void aspace_pprint(uint32 *page, uint32 vbase)
{
    int i,j;

    char buf[80];
    
    for(i=0;i<1024;i++){
        if(page[i]){
            j = page[i];
            snprintf(buf,80,"%s %s %s ",
                    (j & 4) ? "USR" : "SYS",
                    (j & 2) ? "RW" : "RO",
                    (j & 1) ? "P" : "X");

            snprintf(buf+9,80-9,"%xv  %xp", i*4096+vbase, j&0xFFFFFF00);
            
            for(j=1;(i+j < 1024) &&
                    ( (0xFFFFFF00 & page[i+j]) ==
                      (0xFFFFFF00 & page[i+j-1])+0x1000 );j++);
            
            if(j>1) { 
                i += j-1;
                snprintf(buf+29,80-29," - %xp (%x)",
                        0x0FFF + (page[i]&0xFFFFFF00), j);
            }
            kprintf(buf);
        }
    }
}

void aspace_kprint(aspace_t *a)
{
    aspace_pprint(a->high,0x80000000);       
}

void aspace_print(aspace_t *a)
{
    aspace_pprint(a->ptab,0);        
}

aspace_t *aspace_create(void) 
{
    int i;
    uint32 phys;
	uint32 *raw;
    
    aspace_t *a = kmalloc32();
	raw = kgetpages2(2,3,&phys);
	a->pdir = raw;
	a->ptab = &raw[1024];
	a->high = flat->high;
	
    for(i=0;i<1024;i++){
        a->pdir[i] = 0;
        a->ptab[i] = 0;
    }
    a->pdir[0] = (phys + 4096) | 7;
    a->pdir[512] = (_cr3 + 2*4096) | 3;
	rsrc_bind(&a->rsrc, RSRC_ASPACE, NULL);
    return a;
}

void aspace_protect(aspace_t *a, uint32 virt, uint32 flags)
{
    a->ptab[virt] = ((a->ptab[virt] & 0xFFFFF000) | (flags & 0x00000FFF));
    
}

void aspace_map(aspace_t *a, uint32 phys, uint32 virt, uint32 len, uint32 flags)
{
    int i;
    for(i=0;i<len;i++)
        a->ptab[virt+i] = (((phys+i)*4096) & 0xFFFFF000) |
            (flags & 0x00000FFF);
}

void aspace_maphi(aspace_t *a, uint32 phys, uint32 virt, uint32 len, uint32 flags)
{
    int i;
    for(i=0;i<len;i++)
        a->high[(virt&0x3FF)+i] = (phys+i)*4096 | flags;
}

void aspace_clr(aspace_t *a, uint32 virt, uint32 len)
{
    int i;
    for(i=0;i<len;i++)
        a->ptab[virt+i] = 0;
}


