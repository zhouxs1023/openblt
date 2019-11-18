/* $Id: //depot/blt/kernel/memory.h#2 $
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

#ifndef _MEMORY_H_
#define _MEMORY_H_

#define KM16   0
#define KM32   1
#define KM64   2
#define KM128  3
#define KM256  4
#define KM512  5
#define KM1024 6

void *kmalloc(int size);
void *kmallocB(int size);
void kfree(int size, void *block);
void kfreeB(int size, void *block);
void memory_init(void);
void memory_status(void);

#define kmalloc16() kmalloc(KM16)
#define kmalloc32() kmalloc(KM32)
#define kmalloc64() kmalloc(KM64)
#define kmalloc128() kmalloc(KM128)
#define kfree16(v)  kfree(KM16,v)
#define kfree32(v)  kfree(KM32,v)
#define kfree64(v)  kfree(KM64,v)
#define kfree128(v)  kfree(KM128,v)

#endif

