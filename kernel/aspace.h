/* $Id: //depot/blt/kernel/aspace.h#2 $
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

#ifndef _ASPACE_H_
#define _ASPACE_H_

#include "resource.h"
 
#define SEL_KCODE (1*8)
#define SEL_KDATA (2*8)
#define SEL_KDATA2 (3*8)
#define SEL_UCODE (4*8)
#define SEL_UDATA (5*8)
#define SEL_KTSS  (6*8)
#define SEL_UTSS  (7*8)


struct __aspace_t {
	resource_t rsrc;
	
    uint32 *pdir;  /* page directory -- 4k */
    uint32 *ptab;  /* page table     -- 4k */
    uint32 *high;  /* high page table -- 4k (0x8000000 -> ) */
};

aspace_t *aspace_create(void);
void aspace_map(aspace_t *a, uint32 phys, uint32 virt,
                uint32 len, uint32 flags);
void aspace_maphi(aspace_t *a, uint32 phys, uint32 virt,
                  uint32 len, uint32 flags);
void aspace_clr(aspace_t *a, uint32 virt, uint32 len);
void aspace_print(aspace_t *a);
void aspace_portect(aspace_t *a, uint32 virt, uint32 flags);

#endif
