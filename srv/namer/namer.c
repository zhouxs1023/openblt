/* $Id: //depot/blt/srv/namer/namer.c#2 $
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
#include <blt/namer.h>
#include <blt/error.h>
#include <string.h>

static char names[32][32];
static int ports[32];
static int count = 0;

static int port_register(int port, char *name)
{
    int i;
    for(i=0;i<count;i++){
        if(ports[i] == port) return ERR_PERMISSION;        
    }
    if(count == 32) return ERR_MEMORY;

    ports[count] = port;
    strcpy(names[count],name);
    count++;

    return ERR_NONE;    
}

static int port_find(char *name)
{
    int i;
    
    for(i=0;i<count;i++){
        if(!strcmp(name,names[i])) return ports[i];
    }
    return 0;    
}

int main(void)
{
    msg_hdr_t mh;
    namer_message_t msg;    
    int l;

    port_register(1,"namer");
    
    for(;;){        
        mh.dst = 1;
        mh.data = &msg;
        mh.size = sizeof(namer_message_t);        
        l = port_recv(&mh);
        
        if(l < 0) {
            os_console("namer: can't get port");
            os_terminate(-1);            
        }

        msg.text[NAMER_TEXT_MAX-1] = 0;        
        if(msg.number){
            msg.number = port_register(msg.number,msg.text);            
        } else {
            msg.number = port_find(msg.text);            
        }
        mh.data = &msg;
        mh.size = 4;
        mh.dst = mh.src;
        mh.src = 1;
        l = port_send(&mh);

/*        if(l == ERR_RESOURCE) os_console("namer: ERR_RESOURCE");
        if(l == ERR_PERMISSION) os_console("namer: ERR_PERMISSION");
        if(l == ERR_MEMORY) os_console("namer: ERR_MEMORY");*/
    }
    return 0;
}
