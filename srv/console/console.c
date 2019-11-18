/* $Id: //depot/blt/srv/console/console.c#3 $
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

#include <blt/syscall.h>
#include <blt/error.h>
#include <blt/namer.h>
#include <blt/conio.h>
#include <i386/io.h>
#include <string.h>

int console_port;
int send_port;

int y = 2;
char command[80];

#define WITH_KBD

#define MONOx

#ifdef MONO
#define SCREEN 0xB0000
#else
#define SCREEN 0xB8000
#endif


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

static char kpbuf[256];
static char *kp = kpbuf;    

void movecursor (int x, int y);

void printf(char *fmt,...) 
{
    va_list pvar;    
    char *x;
    static msg_hdr_t msg;    
    
    va_start(pvar,fmt);
    va_snprintf(kp,256 - (kp-kpbuf),fmt,pvar);
    va_end(pvar);

    for(x = kp; *x; x++){
        if(*x == '\n'){
            *x = 0;            
            msg.size = strlen(kpbuf);
            msg.data = kpbuf;
            msg.src = send_port;
            msg.dst = console_port;
            port_send(&msg);
            kp = kpbuf;
            return;            
        }
    }
    kp = x;
    
}

void keypress(int key);

int keyboard_nh;

void keyboard_irq_thread(void)
{
    char x[128];
    int i;
    
    int shift = 0;    
    int key;

    send_port = port_create(console_port);
    keyboard_nh = namer_newhandle();

    printf("keyboard: ready\n");

    os_handle_irq(1);
		movecursor (y, 24);

    x[2] = 0;
    i = 0;
    
    for(;;) {
        os_sleep_irq();
#ifdef MULTI
        while(inb(0x64) & 0x01) {
#endif
            key = inb(0x60);
            if(key == 1) {
                os_debug();
                continue;
            }


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
                        keypress(key);                        
                    }
                }
            }
#ifdef MULTI
        }
#endif
    }

}

#define NULL ((void *) 0)

typedef struct _tline 
{
    char raw[160];    
    struct _tline *prev, *next;    
} tline;

typedef struct _tpane
{    
    tline *top, *bottom;
    int visible;
    int height;
    int y0,y1;    
} tpane;


tpane commandline;
tpane console;
tline lines[128];
tline *nextline = NULL;

tline *newline(tline *next, tline *prev)
{
    tline *l = nextline;
    if(!l) os_terminate(0);    
    nextline = nextline->next;
    l->next = next;
    l->prev = prev;    
    return l;    
}

void delline(tline *line)
{
    line->next = nextline;
    nextline = line;    
}

void wipeline(tline *line, int attr)
{
    int i = 0;    
    unsigned char *x = line->raw;
    for(i=0;i<160;i++){
        *x++ = ' ';
        *x++ = attr;
    }
}


void init(tpane *pane, int attr)
{
    int i;
    pane->bottom = pane->top = newline(NULL,NULL);
    wipeline(pane->bottom, attr);
    
    for(i=1;i<pane->height;i++){
        pane->bottom->next = newline(NULL,pane->bottom);
        wipeline(pane->bottom->next, attr);
        pane->bottom = pane->bottom->next;
    }
/*    wipeline(pane->bottom, attr);
    wipeline(pane->bottom->prev, attr);*/
    
}

void clr(tpane *pane, int attr)
{
    int i;
    
    tline *l = pane->top;

    for(i=0;i<pane->height;i++){
        wipeline(l,attr);
        l = l->next;
    }
}

void addline(tpane *pane, char *text)
{
    int i;    

    tline *l = pane->top;
    pane->top = pane->top->next;
    pane->top->prev = NULL;
    pane->bottom->next = l;
    l->prev = pane->bottom;    
    l->next = NULL;
    pane->bottom = l;
    for(i=0;i<80;i++){
        if(*text >= ' ') l->raw[i*2] = *text++;
        else l->raw[i*2] = ' ';
    }
}

void paint(tpane *pane)
{
    int y;
    tline *l;    
    int *a,*b;
    int i;

    for(l=pane->top,y=pane->y0;y<=pane->y1;y++){
        a = (int *) (l->raw);
        b = (int *) (SCREEN + 160*y);
        for(i=0;i<40;i++) *b++ = *a++;
        l = l->next;        
    }
}

void execute(char *command)
{
    int l;

    if(!strncmp(command,"find",4)){
        if(command[4]){
            l = namer_find(keyboard_nh,command+5);            
            printf("console: found \"%s\" at port %d\n",command+5,l);
        }
        return;        
    }
    if(!strncmp(command,"send",4)){
        char *serv;
        char *text;

        if(command[4] == ' '){
            serv = text = command+5;
            while(*text){
                if(*text == ' '){
                    *text = 0;
                    text++;
                    if(*text){
                        msg_hdr_t msg;        
                        msg.dst = namer_find(keyboard_nh,serv);
                        if(msg.dst < 1){
                            printf("console: unknown service \"%s\"\n",serv);
                            return;
                        }
                        msg.src = keyboard_nh;
                        msg.size = strlen(text);
                        msg.data = text;
                        port_send(&msg);
                        return;                        
                    }
                    printf("console: no message?\n");
                    return;                    
                }
                text++;                
            }
            printf("console: no message?\n");
            return;                    
        }
        printf("console: no server or message?\n");        
        return;                            
    }
    if(!strcmp(command,"exit")){
        os_terminate(0x00420023);        
    }
    if(!strcmp(command,"quit")){
        os_terminate(0x00420023);        
    }

    printf("console: bad command\n");
}

void movecursor (int x, int y)
{
	int offset;

	offset = 80 * y + x;
	outb (0xe, 0x3d4);
	outb (offset / 256, 0x3d5);
	outb (0xf, 0x3d4);
	outb (offset % 256, 0x3d5);
}

void keypress(int key)
{
    if(key == BS){
        if(y > 2){
            y--;        
            commandline.bottom->raw[y*2] = ' ';
            paint(&commandline);
        }
				movecursor (y, 24);
        return;        
    }
    if(y == 79 || key == 13){
        if(key == 13){
            command[y-2] = 0;
            execute(command);        
        }
        for(y=2;y<79;y++) commandline.bottom->raw[y*2] = ' ';
        y = 2;
    }
    if(key != 13){        
        commandline.bottom->raw[y*2] = key;
        command[y-2] = key;        
        y++;
    } 
    paint(&commandline);
    movecursor (y, 24);
}


char *divider =
"### OpenBLT System Console #######################################################";


void console_thread(void)
{
    int i;
    int l;    
    char data[81];
    msg_hdr_t msg;    
    msg.data = data;
    msg.dst = console_port;
    msg.size = 80;
    
    for(i=0;i<128;i++) delline(&lines[i]);
    
    console.y0 = 0;
    console.y1 = 22;
    console.height = 23;
    
    commandline.y0 = 23;
    commandline.y1 = 24;
    commandline.height = 2;    

    init(&console, CON_WHITE);
    init(&commandline, CON_BLUE << 4 | CON_WHITE);

    for(i=0;i<80;i++) commandline.top->raw[i*2] = divider[i];
    commandline.bottom->raw[0] = '>';    
        

/*    clr(&console, CON_WHITE);*/
    
    paint(&console);
    paint(&commandline);

    while((l = port_recv(&msg)) > 0){
        data[l]=0;        
        addline(&console,data);
        paint(&console);        
    }
    addline(&console,"console: ready.");
    os_terminate(0);
    
}


int main(void)
{
    int nh;
    
    os_brk(2*32768);

    
    console_port = port_create(0);

    nh = namer_newhandle();    
    namer_register(nh,console_port,"console");
    namer_delhandle(nh);    
    
#ifdef WITH_KBD
    os_thread(keyboard_irq_thread);
#endif
    console_thread();

    return 0;
}


