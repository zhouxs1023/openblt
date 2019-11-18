/* $Id: //depot/blt/srv/video/video.c#2 $
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
#include "string.h"

#include <blt/namer.h>
#include <blt/syscall.h>
#include <blt/conio.h>

#include <stdlib.h>

#include "vga.h"
#include "blit.h"

int port_video;

unsigned char *vram = 0xA0000;


unsigned char *fish = 
    ".........XXXXXX.....XXX."
    ".....XXXXYYYYYYXX..XYYYX"
    "...XXYYYYYYYYYYYYXXYYYYX"
    ".XXYYYYYYYYYYYYYYYYYYYX."
    "...XXYYYYYYYYYYYYYXXYYYX"
    ".....XXXYYYYYYYXXX..XXYX"
    "........XXXYYYX.......XX"
    "...........XXXXX........"
;




#define pixel(x,y,c) (vram[(y)*320+(x)] = c)


void fill(int w, int h, int x, int y, int c)
{
    int tx,ty;
    
    for(ty = y; ty < y+h; ty++){
        for(tx = x; tx < x+w; tx++){
            pixel(tx,ty,c);
        }
    }

}

void fill2(int w, int h, int x, int y)
{
    int tx,ty;
    
    for(ty = y; ty < y+h; ty++){
        for(tx = x; tx < x+w; tx++){
            pixel(tx,ty,128 + ty/4);
        }
    }

}

#define WAIT 50000
/*000*/
/*000*/

#ifdef EXP
typedef struct _sprite {
    struct _sprite *next;
    int x,y,dx,dy;
    void *bitmap;
    char *text;
} sprite;


sprite *first = NULL;

int testAndSet(unsigned char *addr)
{
   unsigned char result = 1;
   asm ("xchgb %1, %0":"=m" (*addr),"=r" (result)
                      :"1" (result) :"memory");
   return result;
}

#define acquire(n) {while(testAndSet(n));}
#define release(n) {(*((unsigned char*)n)=0);}

void animate()
{
    sprite *s,*x;
    os_sleep(10);

    for(s=first;s;s=s->next){
        if(
    }
}
#endif
void bounce(int ix,int iy, char *xx)
{
    int x = ix;
    int y = iy;
    int vx = 1;
    int vy = 1;
    int t;
    int l = strlen(xx)*5;
    
    for(;;){

/*	blit_str("OpenBLT -- more asskicking for your dollar!",20,180);*/
        if(x + vx < 0 || x + vx > 320 - 25) vx = -vx;
        if((y + vy) < 9 || (y + vy) > 200 - 9) vy = -vy;
        for(t=0;t<WAIT;t++);
	os_sleep(10);

        fill2(l,8,x,y-8);
        fill2(24,16,x,y);
        
        x += vx;
        y += vy;
        if(vx < 0) {
            blit_trans(fish,x,y,24,8);
        } else {
            blit_trans_r(fish,x,y,24,8);
        }
        blit_str(xx,x,y-8);
        
    }

}


void t1()
{
    bounce(160,100,"23");
}

void t2()
{
    bounce(40,50,"17");
}

void t3()
{
    bounce(100,160,"1");
    
}

char name[80];

void tn()
{
    char x[80];
    strcpy(x,name);
    
    bounce(100,160,x);
}


int main(void)
{
    msg_hdr_t msg;
    
    int i;
    int nh;    
 
    nh = namer_newhandle();
    port_video = port_create(0);
    namer_register(nh, port_video, "video");
    namer_delhandle(nh);

    os_console("video driver is alive");

    os_handle_irq(0);


    setVideoMode(320,200,8);

    setPalette('.',0,0,255);
    setPalette('X',255,0,0);
    setPalette('Y',255,255,0);
    setPalette('#',0,255,0);
    
    fill(320,200,0,0,'.');
    for(i=0;i<50;i++){
        setPalette(128+i,0,0,12+i);
        fill(320,4,0,4*i,128+i);
    }


    os_thread(t1);
    os_thread(t2);
    os_thread(t3);

    
    for(;;){
        msg.src = 0;
        msg.dst = port_video;
        msg.size = 16;
        msg.data = &name;
        msg.flags = 0;

        i = port_recv(&msg);
        name[i] = 0;
        
        os_thread(tn);
    }
    
    return(0);
}
