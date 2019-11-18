/* $Id: //depot/blt/lib/memory.c#4 $
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
#include <blt/types.h>
#include <blt/syscall.h>
#include "malloc.h"

void __malloc_initialize();

static unsigned int __sbrk_max;
static unsigned int __sbrk_cur;

void *sbrk(int size)
{
    if(size < 0 ){
        __sbrk_cur -= size;
        return (void *) __sbrk_cur;
    } else {
        unsigned int tmp = __sbrk_cur;
        __sbrk_cur += size;
        if(__sbrk_cur > __sbrk_max){
            __sbrk_max = __sbrk_cur;
            os_brk(__sbrk_max);
        }
        return (void *) tmp;
    }

    return (void *) __sbrk_cur;
}

int sem_malloc = 0;

void *malloc(size_t size)
{
    void *r;
    sem_acquire(sem_malloc);
    r = __malloc(size);
    sem_release(sem_malloc);
    return r;
}

void free(void *ptr)
{
    sem_acquire(sem_malloc);
    __free(ptr);
    sem_release(sem_malloc);
}

void *realloc(void *ptr, size_t size)
{
    void *r;
    sem_acquire(sem_malloc);
    r = __realloc(ptr,size);
    sem_release(sem_malloc);
    return r;
}


void * _default_morecore(long size)
{
    void *result;

    result = sbrk(size);
    if (result == (void *) -1)
        return NULL;
    return result;
}

void __libc_init_memory(unsigned int top_of_binary,
                        unsigned int start_bss)
{
    char m[80];
    unsigned char *x = (unsigned char *) start_bss;
    unsigned int tob = (top_of_binary/4096+2)*4096;
    os_brk(tob);

    sem_malloc = sem_create(1);
    
    while(x <= ((unsigned char *) top_of_binary)) {
        *x = 0;
        x++;
    }
    __sbrk_max = __sbrk_cur  = tob;
}
