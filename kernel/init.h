/* $Id: //depot/blt/kernel/init.h#1 $
**
** Copyright 1998 Sidney Cammeresi
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

#ifndef _INIT_H_
#define _INIT_H_

/*
 * Behold macros to place functions or variables in special elf sections.  To
 * use these jewels, write your functions like this
 *
 *         void __init__ foo_init (int a, int b)
 *         {
 *         }
 *
 * and your function prototypes like this
 *
 *         extern void foo_init (int a, int b) __init__;
 *
 * and your data like this
 *
 *         static char *foo_stuff __initdata__ = "Foo rules the world.";
 *
 * At some point in the future, the .text.init and .data.init sections of the
 * kernel may be discarded at the end of the kernel's initialisation to free
 * up a bit of memory.
 */

#define __init__            __attribute__ ((__section__ (".text.init")))
#define __initdata__        __attribute__ ((__section__ (".data.init")))

#endif

