/* $Id: //depot/blt/kernel/memory.c#2 $
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


static unsigned char size_map[1025] = {
    KM16,
    
    KM16, KM16, KM16, KM16, KM16, KM16, KM16, KM16,
    KM16, KM16, KM16, KM16, KM16, KM16, KM16, KM16,

    KM32, KM32, KM32, KM32, KM32, KM32, KM32, KM32,
    KM32, KM32, KM32, KM32, KM32, KM32, KM32, KM32,

    KM64, KM64, KM64, KM64, KM64, KM64, KM64, KM64,
    KM64, KM64, KM64, KM64, KM64, KM64, KM64, KM64,
    KM64, KM64, KM64, KM64, KM64, KM64, KM64, KM64,
    KM64, KM64, KM64, KM64, KM64, KM64, KM64, KM64,

    KM128, KM128, KM128, KM128, KM128, KM128, KM128, KM128,
    KM128, KM128, KM128, KM128, KM128, KM128, KM128, KM128,
    KM128, KM128, KM128, KM128, KM128, KM128, KM128, KM128,
    KM128, KM128, KM128, KM128, KM128, KM128, KM128, KM128,
    KM128, KM128, KM128, KM128, KM128, KM128, KM128, KM128,
    KM128, KM128, KM128, KM128, KM128, KM128, KM128, KM128,
    KM128, KM128, KM128, KM128, KM128, KM128, KM128, KM128,
    KM128, KM128, KM128, KM128, KM128, KM128, KM128, KM128,
    
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,    
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,
    KM256, KM256, KM256, KM256, KM256, KM256, KM256, KM256,

    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,    
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,    
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,
    KM512, KM512, KM512, KM512, KM512, KM512, KM512, KM512,

    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,    
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,    
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,    
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,    
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024,
    KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024, KM1024    
};


void *kmallocB(int size)
{
    if(size < 1025){
        return kmalloc(size_map[size /* X? -1 */] );        
    } else {
        panic("illegal size in kmallocB");        
    }
    return NULL; /* for paranoid compilers */
}

void kfreeB(int size, void *block)
{
    if(size < 1025){
        kfree(size_map[size /* X? -1 */],block);        
    } else {
        panic("illegal size in kmallocB");        
    }
}



struct km_mnode 
{
    struct km_mnode *next;    
};

static struct _km_map
{
    int size;
    int used_count;
    int fresh_count;
    int free_count;
    struct km_mnode *free_chain;
    void *fresh_chain;    
} km_map[7] = {
    {   16,   0, 256, 0, NULL, NULL },
    {   32,   0, 128, 0, NULL, NULL },
    {   64,   0,  64, 0, NULL, NULL },
    {  128,   0,  32, 0, NULL, NULL },
    {  256,   0,  16, 0, NULL, NULL },
    {  512,   0,   8, 0, NULL, NULL },
    { 1024,   0,   4, 0, NULL, NULL }
};        

extern uint32 memsize;
extern uint32 memtotal;

void memory_status(void)
{
    int i;
	int inuse = 0, allocated = 0;
	
    kprintf("");
    kprintf("size used free fresh");    
    for(i=0;i<7;i++){
        kprintf("%U %U %U %U", km_map[i].size, km_map[i].used_count,
                km_map[i].free_count, km_map[i].fresh_count);
		inuse += km_map[i].size * km_map[i].used_count;
		allocated += km_map[i].size * 
		    (km_map[i].free_count+km_map[i].used_count+km_map[i].fresh_count);
		
    }
	inuse /= 1024;
	allocated /= 1024;
	kprintf("");
	kprintf("%Ukb allocated, %Ukb in use",allocated,inuse);
	kprintf("%U (of %U) pages in use",memtotal-memsize,memtotal);
	 
}

void memory_init(void) 
{
    int i;

    for(i=0;i<7;i++)
        km_map[i].fresh_chain = kgetpages(1,3);    
}


void *kmalloc(int size)
{
    struct _km_map *km;    
    void *block;    
    if(size > KM1024) panic("illegal size in kmalloc()");
    km = &km_map[size];

    km->used_count++;

    if(km->free_chain) {
            /* recycle free'd blocks if available */
        km->free_count--;
        block = (void *) km->free_chain;
        km->free_chain = km->free_chain->next;
    } else {
            /* shave a new block off of the fresh page if
               we can't recycle */
        km->fresh_count--;
        block = km->fresh_chain;

        if(km->fresh_count){
                /* still some left, just bump us to the next chunk */
            km->fresh_chain = (void *)
                (((char *) km->fresh_chain) + km->size);        
        } else {
                /* gotta grab a new page */
            km->fresh_count = 4096 / km->size;        
            km->fresh_chain = kgetpages(1,3);            
        }
    }
    
    return block;    
}

void kfree(int size, void *block)
{
    struct _km_map *km;    
    if(size > KM1024) panic("illegal size in kmalloc()");
    km = &km_map[size];
    
    km->free_count++;    
    km->used_count--;
    ((struct km_mnode *) block)->next = km->free_chain;    
    km->free_chain = (struct km_mnode *) block;    
}

