/* $Id: //depot/blt/kernel/ktrace.c#6 $
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

/* serial tracing */

#include <stdarg.h>
#include "memory.h"
#include <i386/io.h>
#include <blt/conio.h>
#include "kernel.h"

#define NULL ((void *) 0)

#define com1 0x3f8 
#define com2 0x2f8

#define combase com1

char *kprintf_lock = 0;

void va_snprintf(char *b, int l, char *fmt, va_list pvar);

#ifdef USE_SERIAL
void krefresh(void)
{
}

void kprintf_init(void)
{
    outb(0, combase + 4);
    outb(0, combase + 0);
    outb(0x83, combase + 3);
    outb(6, combase);                           /* 9600 bps, 8-N-1 */
    outb(0, combase+1);
    outb(0x03, combase + 3);
}

static int ser_getc(void)
{
    while (!(inb(combase + 5) & 0x01));
    return inb(combase);
}

static void ser_putc(int ch)
{
    while (!(inb(combase + 5) & 0x20));
    outb((unsigned char) ch, combase);
}

static void ser_puts(char *s)
{
    int t;
    while(*s){
        ser_putc(*s);
        s++;
    }
}


char *kgetline(char *line, int len)
{
    char c;
    int pos = 0;

    ser_puts(": ");
    
    for(;;){
        switch(c = ser_getc()){
        case 10:
        case 13:
            line[pos]=0;
            ser_puts("\r\n");
            return line;
            
        case 8:
            if(pos) {
                pos--;
                ser_puts("\b \b");
            }
            break;
            
        case 27:
            while(pos) {
                pos--;
                ser_puts("\b \b");
            }
            break;

        default:
            if((c >= ' ') && (c < 0x7f) && (pos < len-1)){
                line[pos] = c;
                pos++;
                ser_putc(c);
            }
        }
    }
}

void kprintf(char *fmt, ...)
{
    static char line[128];
    va_list pvar;    
    va_start(pvar,fmt);
		p (&kprintf_lock);
    va_snprintf(line,128,fmt,pvar);
    line[127]=0;
    va_end(pvar);
    ser_puts(line);
    ser_puts("\r\n");
		v (&kprintf_lock);
}

#else

#define ESC 27
#define BS 8
#define TAB 9
#define CR 13
char ScanTable [] =  {' ', ESC, '1', '2', '3', '4', '5', '6', '7', '8',
                      '9', '0', '-', '=', BS,  TAB, 'q', 'w', 'e', 'r',
                      't', 'y', 'u', 'i', 'o', 'p', '[', ']', CR,  ' ',
                      'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
                      '\'', '~', ' ', '\\', 'z', 'x', 'c', 'v', 'b', 'n',
                      'm', ',', '.', '/', ' ', ' ', ' ', ' ', ' '};
char ShiftTable [] = {' ', ESC, '!', '@', '#', '$', '%', '^', '&', '*',
                      '(', ')', '_', '+', ' ', ' ', 'Q', 'W', 'E', 'R',
                      'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', CR,  ' ',
                      'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
                      '\"', '~', ' ', '|', 'Z', 'X', 'C', 'V', 'B', 'N',
                      'M', '<', '>', '?', ' ', ' ', ' ', ' ', ' '};
#define LSHIFT 42
#define RSHIFT 54

# if OLD
typedef struct _kp_ring
{
    struct _kp_ring *next, *prev;
    char line[80];
} kp_ring;

static kp_ring *kr_cur;

void kprintf_init(void)
{
    kp_ring *r,*t,*b;
    int i;

    b = t = (kp_ring *) kmalloc(KM128);
    t->line[0] = 0;
    t->next = NULL;
	memset(t->line,0,80);
    for(i=0;i<32;i++){
        r = (kp_ring *) kmalloc(KM128);
        memset(r->line,0,80);	
        t->prev = r;
        r->next = t;
        t = r;
    }
    b->next = t;
    t->prev = b;    
    kr_cur = t;

    con_start(
#ifdef MONO
        0xB0000
#else
        0xB8000
#endif            
        );
    con_attr(CON_YELLOW|0x08);
    con_clear();
}

void krefresh(void)
{
    kp_ring *r = kr_cur;
    int i;
    con_clear();
    for(i=0;i<24;i++) {
        r = r->prev;
        con_goto(0,23-i);
        con_puts(r->line);
    }
    con_goto(0,24);
}
#else 

static unsigned char *screen = NULL;
static unsigned char vscreen[80*25*2];

void *kmappages(int phys, int count, int flags);

void kprintf_init()
{
	screen = (unsigned char *) kmappages(
#ifdef MONO
										 0xB0, 
#else 
										 0xB8, 
#endif
										 2, 7);
	
    con_start((uint32) vscreen);
    con_attr(CON_YELLOW|0x08);
    con_clear();
}

void krefresh(void)
{
	memcpy(screen,vscreen,80*25*2);
}

void kprintf(char *fmt, ...)
{
    va_list pvar;    
	char line[80];
    va_start(pvar,fmt);
		p (&kprintf_lock);
    va_snprintf(line,80,fmt,pvar);
    line[79] = 0;
    va_end(pvar);
    con_goto(0,24);
    con_puts(line);
    con_puts("\n");
	memcpy(screen,vscreen,80*25*2);
		v (&kprintf_lock);
}

#endif

void movecursor (int x, int y)
{
    int offset;

    offset = 80 * y + x;
    outb (0xe, 0x3d4);
    outb (offset / 256, 0x3d5);
    outb (0xf, 0x3d4);
    outb (offset % 256, 0x3d5);
}   

char *kgetline(char *line, int len)
{
    int i,lp,key;
    int shift = 0;
    if(len > 80) len = 80;
    
  restart:
    for(i=1;i<len-1;i++) line[i] = ' ';
    line[0] = ':';
    line[1] = ' ';
    line[len-1] = 0;
    lp = 2;
    
    for(;;){
        con_goto(0,24);
        con_puts(line);
		movecursor(lp,24);
		
#ifndef OLD
		memcpy(screen + (80*24*2), vscreen + (80*24*2), 160);
#endif
        while(!(inb(0x64) & 0x01));
        key = inb(0x60);
        switch(key){
        case LSHIFT:
        case RSHIFT:
            shift = 1;
            break;
        case LSHIFT | 0x80:
        case RSHIFT | 0x80:
            shift = 0;
            break;
        default:
            if(key & 0x80){
                    /* break */
            } else {
                if(key < 59){
                    key = shift ? ShiftTable[key] : ScanTable[key];

                    switch(key){
                    case CR:
                        line[lp] = 0;
                        kprintf(line);
                        return line + 2;
                    case ESC:                        
                        goto restart;
                    case BS:
                        if(lp > 2){
                            lp--;
                            line[lp]=' ';
                        }
                        break;
                    default:
                        if(lp < len-1);
                        line[lp] = key;
                        lp++;
                    }
                    
                }
            }
        }
    }
}

#if OLD
void kprintf(char *fmt, ...)
{
    va_list pvar;    
    va_start(pvar,fmt);
		p (&kprintf_lock);
    va_snprintf(kr_cur->line,80,fmt,pvar);
    kr_cur->line[79] = 0;
    va_end(pvar);
    con_goto(0,24);
    con_puts(kr_cur->line);
    con_puts("\n");
    kr_cur = kr_cur->next;    
    con_puts(kr_cur->line);
		v (&kprintf_lock);
}
#endif

#endif
