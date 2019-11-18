/* $Id: //depot/blt/include/blt/syscall.h#4 $
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

#ifndef _SYSCALL_H_
#define _SYSCALL_H_

int   os_thread(void *addr);
void  os_terminate(int status);
void  os_console(char *string);
int   os_brk(int addr);
void  os_handle_irq(int irq);
void  os_sleep_irq(void);
void  os_debug(void);
void  os_sleep(int ticks);
int   os_identify(int rsrc); /* returns the thread_id of the owner of a resource */

int sem_create(int value);
int sem_destroy(int sem);
int sem_acquire(int sem);
int sem_release(int sem);

typedef struct {
    int flags;
    int src;
    int dst;
    int size;
    void *data;    
} msg_hdr_t;

#define msg_SEND 1
#define msg_RECV 2
#define msg_NOWAIT 3

#define PORT_OPT_NOWAIT        1
#define PORT_OPT_SETRESTRICT   2
#define PORT_OPT_SETDEFAULT    3
#define PORT_OPT_SLAVE         4

int port_create(int restrict);
int port_destroy(int port);
int port_option(int port, int opt, int arg);
int port_send(msg_hdr_t *mh);
int port_recv(msg_hdr_t *mh);

#define port_set_restrict(port, restrict) port_option(port,PORT_OPT_SETRESTRICT,restrict);
#define port_slave(master, slave) port_option(slave,PORT_OPT_SLAVE,master)


#define RIGHT_PERM_READ      0x0001   /* allow 'read' access to something         */
#define RIGHT_PERM_WRITE     0x0002   /* allow 'write' access to something        */
#define RIGHT_PERM_DESTROY   0x0004   /* allow the something to be destroyed      */
#define RIGHT_PERM_ATTACH    0x0008   /* allows other rights to be attached       */
#define RIGHT_PERM_GRANT     0x0010   /* this right may be granted to another     */
                                      /* thread by a thread that is not the owner */
#define RIGHT_MODE_INHERIT   0x0020   /* automatically granted to child           */
#define RIGHT_MODE_DISSOLVE  0x0040   /* When the owner thread terminates,        */
                                      /* the right is destroyed                   */   

int thr_create(void *addr, void *data);
int thr_resume(int thr_id);
int thr_suspend(int thr_id);
int thr_kill(int thr_id);

int right_create(int rsrc_id, int flags);
int right_destroy(int right_id);
int right_revoke(int right_id, int thread_id); 
int right_grant(int right_id, int thread_id);

/* map the boot filesystem into our address space and get its address. */
void *os_getbootfs (void);
 
#endif
