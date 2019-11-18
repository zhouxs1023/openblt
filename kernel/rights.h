/* $Id: //depot/blt/kernel/rights.h#2 $
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

#ifndef _RIGHTS_H_
#define _RIGHTS_H_

#include "resource.h"
 
#define RIGHT_PERM_READ      0x0001   /* allow 'read' access to something         */
#define RIGHT_PERM_WRITE     0x0002   /* allow 'write' access to something        */
#define RIGHT_PERM_DESTROY   0x0004   /* allow the something to be destroyed      */
#define RIGHT_PERM_ATTACH    0x0008   /* allows other rights to be attached       */
#define RIGHT_PERM_GRANT     0x0010   /* this right may be granted to another     */
                                      /* thread by a thread that is not the owner */
#define RIGHT_MODE_INHERIT   0x0020   /* automatically granted to child           */
#define RIGHT_MODE_DISSOLVE  0x0040   /* When the owner thread terminates,        */
                                      /* the right is destroyed                   */   
					   
int right_create(uint32 rsrc_id, uint32 flags);
int right_destroy(uint32 right_id);
int right_revoke(uint32 right_id, uint32 thread_id); 
int right_grant(uint32 right_id, uint32 thread_id);

struct __right_t
{
	resource_t rsrc;
	
	resource_t *attached;
	uint32 flags;
	uint32 refcount;
	struct __tnode_t *holders;
};



#endif