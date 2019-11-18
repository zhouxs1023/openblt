/* $Id: //depot/blt/include/blt/syscall_id.h#5 $
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
#ifndef _SYSCALL_ID_H
#define _SYSCALL_ID_H

/* syscall id #'s */
#define BLT_SYS_os_thread      0
#define BLT_SYS_os_terminate   1
#define BLT_SYS_os_console     2
#define BLT_SYS_os_brk         3
#define BLT_SYS_os_handle_irq  4
#define BLT_SYS_os_sleep_irq   5
#define BLT_SYS_os_debug       6
#define BLT_SYS_os_sleep       7
#define BLT_SYS_sem_create     8
#define BLT_SYS_sem_destroy    9
#define BLT_SYS_sem_acquire   10
#define BLT_SYS_sem_release   11
#define BLT_SYS_port_create   12
#define BLT_SYS_port_destroy  13
#define BLT_SYS_port_option   14
#define BLT_SYS_port_send     15
#define BLT_SYS_port_recv     16
#define BLT_SYS_os_identify   17
#define BLT_SYS_right_create  18
#define BLT_SYS_right_destroy 19
#define BLT_SYS_right_revoke  20
#define BLT_SYS_right_attach  21
#define BLT_SYS_right_grant   22
#define BLT_SYS_get_bootfs    23
#define BLT_SYS_thr_create    24
#define BLT_SYS_thr_resume    25
#define BLT_SYS_thr_suspend   26
#define BLT_SYS_thr_kill      27
#endif
