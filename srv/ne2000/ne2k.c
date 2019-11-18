/* $Id: //depot/blt/srv/ne2000/ne2k.c#2 $
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

#include "net.h"

#define NULL ((void *) 0)

#include "ne2k.h"

static unsigned char *loadip = (unsigned char *) 0x20;

static snic TheSNIC;

static unsigned char prom[32];
static unsigned char IP[4] = { 10, 113, 216, 6 };
/*unsigned char IP[4] = { 192, 17, 19, 102 };*/

/* messaging */
static int port_isr = 0;
static int port_xmit = 0;
/*static int port_xmit_done = 0;*/
static int sem_xmit_done = 0;
static int port_dispatch = 0;
static int port_console = 0;
static int port_net = 0;

static msg_hdr_t kmsg;
static char kpbuf[512];
static char *kp = kpbuf;
static char *x;


int mutex = 0;
int sem_ring = 0;


int testAndSet(unsigned char *addr)
{
   unsigned char result = 1;
   asm ("xchgb %1, %0":"=m" (*addr),"=r" (result)
                      :"1" (result) :"memory");
   return result;
}

void kprintf(char *fmt,...)
{    
    va_list pvar;    
    va_start(pvar,fmt);
    va_snprintf(kp,512 - (kp-kpbuf),fmt,pvar);
    va_end(pvar);

    return;
    
    for(x = kp; *x; x++){
        if(*x == '\n'){
            *x = 0;            
            kmsg.size = strlen(kpbuf);
            kmsg.data = kpbuf;
            kmsg.src = port_isr;
            kmsg.dst = port_console;
            port_send(&kmsg);
            kp = kpbuf;
            return;            
        }
    }
    kp = x;    
}
#define printf kprintf

#define RINGSIZE 16
#define PACKETSIZE 1536

typedef struct _pbuf {
    struct _pbuf *next;  /* 4 */
    packet_buffer pb;    /* 16 */
    packet_data pd;      /* 8 */
    int n;
} pbuf;

#define pb_to_ring(x) ((pbuf *) (((char *) (x)) - 4))
#define pd_to_ring(x) ((pbuf *) (((char *) (x)) - 20))
                       
pbuf *p_next = NULL;

struct _pmap {
    struct _pmap *next;;
    int udp_port;
    int blt_port;
} pmap[10];

pbuf *p_discard;

void init_ring(void)
{
    int i;
    pbuf *p;
    p_next = NULL;
    pmap[0].udp_port = 0;
    
    for(i=0;i<RINGSIZE;i++){
        p = (pbuf *) malloc(sizeof(pbuf));        
        p->next = p_next;
        p_next = p;
        p->pb.count = 1;
        p->pb.buf = &(p->pd);
        p->pd.ptr = (char *) malloc(PACKETSIZE);
        p->n = i;
    }

    p_discard = p_next;
    p_next = p_next->next;
}


/* called by the ne2k core to get a receive buffer */
packet_data *alloc_buffer_data(uint size)
{
    pbuf *P;

    sem_acquire(sem_ring);
    if(!p_next){
        sem_release(sem_ring);
        printf("ne2000: !!! out of packet ringbuffers (recv)\n");
        return NULL;
    }
    P = p_next;
    p_next = P->next;
    sem_release(sem_ring);
    
    P->pd.len = size;
    return &(P->pd);
}


/* called by the us to sem_release a received buffer */
void free_buffer_data(packet_data *pd)
{
    pbuf *P = pd_to_ring(pd);
    sem_acquire(sem_ring);
    P->next = p_next;
    p_next = P;
    sem_release(sem_ring);
}


/* called by our send data routines to get a send buffer */
pbuf *get_pbuf(void)
{
    pbuf *P;
    sem_acquire(sem_ring);
    if(!p_next){
        sem_release(sem_ring);
        return p_discard;        
    }
    P = p_next;
    p_next = P->next;
    sem_release(sem_ring);
    return P;
}

/* called after a pbuf is xmit'd */
void free_buffer(packet_buffer *ptr)
{
    msg_hdr_t mh;
    pbuf *P = pb_to_ring(ptr);

    if(P == p_discard) return;

    sem_acquire(sem_ring);
    P->next = p_next;
    p_next = P;
    sem_release(sem_ring);

    sem_release(sem_xmit_done);
    
/*    mh.flags = 0;
    mh.src = port_isr;
    mh.dst = port_xmit_done;
    mh.size = 4;
    mh.data = &mh.flags;
    if(port_send(&mh) != 4) printf("free_buffer: blargh!"); */
}

int ticks = 0;
void idle(void)
{
    int i;
    for(i=0;i<1000;i++);
    ticks++;
}

typedef struct 
{
    net_ether ether;
    net_arp arp;    
} arp_packet;

void print_arp(unsigned char *p);


void xmit(pbuf *P)
{
    int i;
    msg_hdr_t mh;
        /* printf("xmit: P->n = %d\n",P->n);*/

    if(P == p_discard) return;
    
    mh.flags = 0;
    mh.src = port_isr;
    mh.dst = port_xmit;
    mh.size = 4;
    mh.data = &P;
    if((i = port_send(&mh)) != 4) printf("xmit: blargh! %d\n",i);
    
}

void handle_arp(arp_packet *req, pbuf *packet)
{
    if(htons(req->arp.arp_op) == ARP_OP_REQUEST){
        if(!memcmp(&(req->arp.arp_ip_target),IP,4)){
            pbuf *P;
            arp_packet *resp;
            P = get_pbuf();
            resp = (arp_packet *) P->pd.ptr;

/*            printf("handle_arp: IS the one!\n");*/
            memcpy(&(resp->ether.src),prom,6);
            memcpy(&(resp->ether.dst),&(req->ether.src),6);
            resp->ether.type = htons(0x0806);
            resp->arp.arp_hard_type = htons(1);
            resp->arp.arp_prot_type = htons(0x0800);
            resp->arp.arp_hard_size = 6;
            resp->arp.arp_prot_size = 4;            
            resp->arp.arp_op = htons(ARP_OP_REPLY);
            memcpy(&(resp->arp.arp_enet_sender),prom,6);
            memcpy(&(resp->arp.arp_ip_sender),IP,4);
            memcpy(&(resp->arp.arp_enet_target),&(req->arp.arp_enet_sender),6);
            memcpy(&(resp->arp.arp_ip_target),&(req->arp.arp_ip_sender),4);
            print_arp((unsigned char *) &(resp->arp));

            P->pd.len = sizeof(arp_packet);
            P->pb.len = sizeof(arp_packet);
            
            xmit(P);
        } else {
                /*printf("handle_arp: NOT the one.\n");            */
        }
    }
}

void print_arp(unsigned char *p)
{
    net_arp *arp = (net_arp *) p;
    unsigned char *b;    
    unsigned short t;
    
    printf("  ARP: ");
    t = htons(arp->arp_op);
    if(t == ARP_OP_REQUEST) printf("req ");
    else if(t == ARP_OP_REPLY) printf("rep ");
    else printf("??? ");

    b = (unsigned char *) &(arp->arp_enet_sender);
    printf("source:  %X:%X:%X:%X:%X:%X ",b[0],b[1],b[2],b[3],b[4],b[5]);
    b = (unsigned char *) &(arp->arp_ip_sender);
    printf("(%d.%d.%d.%d)\n",b[0],b[1],b[2],b[3]);

    printf("  ARP:     target:  ");    
    
    b = (unsigned char *) &(arp->arp_enet_target);
    printf("%X:%X:%X:%X:%X:%X ",b[0],b[1],b[2],b[3],b[4],b[5]);
    b = (unsigned char *) &(arp->arp_ip_target);
    printf("(%d.%d.%d.%d)\n",b[0],b[1],b[2],b[3]);    

}

void print_ip(unsigned char *p)
{

}

int ipchksum(void *_ip, int len)
{
    unsigned short *ip = (unsigned short *) _ip;
    unsigned long sum = 0;
    len >>= 1;
    while (len--) {
        sum += *(ip++);
        if (sum > 0xFFFF)
            sum -= 0xFFFF;
    }
    return((~sum) & 0x0000FFFF);
}


void handle_icmp(icmp_packet *icmp)
{
    pbuf *P;
    icmp_packet *resp;

    if(icmp->icmp.icmp_type == ICMP_PING_REQ){
        P = get_pbuf();
        resp = (icmp_packet *) P->pd.ptr;
        
        memcpy(&(resp->ether.src),prom,6);
        memcpy(&(resp->ether.dst),&(icmp->ether.src),6);
        memcpy(&(resp->ether.type),&(icmp->ether.type),2);

        resp->ip.ip_hdr_len = 5;
        resp->ip.ip_version = 4;
        resp->ip.ip_tos = 0;
        resp->ip.ip_len = icmp->ip.ip_len;
        resp->ip.ip_id = 0;
        resp->ip.ip_off = 0;
        resp->ip.ip_ttl = 64;
        resp->ip.ip_proto = 0x01;
        resp->ip.ip_chk = 0;
        memcpy(&(resp->ip.ip_src),IP,4);
        memcpy(&(resp->ip.ip_dst),&(icmp->ip.ip_src),4);
        resp->ip.ip_chk = ipchksum(&(resp->ip),sizeof(net_ip));
        
        resp->icmp.icmp_type = ICMP_PING_REP;
        resp->icmp.icmp_code = 0;
        resp->icmp.icmp_chk = 0;
        resp->icmp.data.ping.id = icmp->icmp.data.ping.id;
        resp->icmp.data.ping.seq = icmp->icmp.data.ping.seq;
        memcpy(resp->icmp.data.ping.data,icmp->icmp.data.ping.data,
               ntohs(icmp->ip.ip_len) - 28);

        resp->icmp.icmp_chk = ipchksum(&(resp->ip),ntohs(resp->ip.ip_len));
/*        printf("ICMP resp l = %d\n",ntohs(resp->ip.ip_len));*/

        P->pb.len = P->pd.len = ntohs(resp->ip.ip_len)+14;

        xmit(P);
    }        
}

void handle_udp(udp_packet *udp, pbuf *packet)
{
    msg_hdr_t msg;
    int i;
    
    if(pmap[0].udp_port == ntohs(udp->udp.udp_dst)){
        msg.src = port_isr;
        msg.dst = pmap[0].blt_port;
        msg.flags = 0;
        msg.size = ntohs(udp->udp.udp_len) - 8;
        msg.data = udp->data;

        if((i = port_send(&msg)) <0)
            printf("^&%$&^ %d / %d / %x\n",i,msg.size,msg.data);
        
        
    }
    
/*    printf("UDP %d.%d.%d.%d:%d -> %d.%d.%d.%d:%d\n",
           src[0],src[1],src[2],src[3],ntohs(udp->udp.udp_src),
           dst[0],dst[1],dst[2],dst[3],ntohs(udp->udp.udp_dst)
           );*/
}

void handle_ip(ip_packet *ip, pbuf *packet)
{
/*    unsigned char *src = (unsigned char *) &(ip->ip.ip_src);*/
    unsigned char *dst = (unsigned char *) &(ip->ip.ip_dst);
    static unsigned char bcip[4] = { 255, 255, 255, 255 };
    
    if(!memcmp(dst,IP,4) || (dst[3] == 0xFF)){ /*!memcmp(dst,bcip,4)){*/
/*    printf("IP %d.%d.%d.%d -> %d.%d.%d.%d\n",
      src[0],src[1],src[2],src[3],
           dst[0],dst[1],dst[2],dst[3]

           );*/
    
        switch(ip->ip.ip_proto){
        case 0x01:
            handle_icmp((icmp_packet *) ip);
            break;
            
        case 0x11:
            handle_udp((udp_packet *) ip, packet);
            break;
        }
    }
}




void receive(void *cb, packet_data *_packet)
{
    pbuf *packet = pd_to_ring(packet);
    unsigned char *b = (unsigned char *) packet->pd.ptr;
            
    if(b[12] == 0x08){
        if(b[13] == 0x00) {
            handle_ip((ip_packet *) b, packet);
        } else if(b[13] == 0x06) {
            handle_arp((arp_packet *) b, packet);
        }
    }
    free_buffer_data(&(packet->pd));
}

void xxreceive(void *cb, packet_data *packet)
{
    msg_hdr_t msg;
    int i;
    pbuf *P = pd_to_ring(packet);

    if(P == p_discard) return;
    
    msg.src = port_isr;
    msg.dst = port_dispatch;
    msg.flags = 0;
    msg.size = 4;
    msg.data = &P;

    if((i = port_send(&msg)) != 4){
        printf("receive: dispatch send failed %d\n",i);
        free_buffer_data(packet);
    }
}


#define NET_CONNECT  1
#define NET_SEND     2
#define NET_IP       3
typedef struct 
{
    int cmd; 
    int port;
    char data[0];
} net_cmd;


unsigned char bcast[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

void send_udp(int port, unsigned char *ip, void *data, int size)
{
    pbuf *P;
    udp_packet *udp;

    P = get_pbuf();
    udp = (udp_packet *) P->pd.ptr;
    
    memcpy(&(udp->ether.src),prom,6);
    memcpy(&(udp->ether.dst),bcast,6);
    udp->ether.type = ntohs(0x0800);
    
    udp->ip.ip_hdr_len = 5;
    udp->ip.ip_version = 4;
    udp->ip.ip_tos = 0;
    udp->ip.ip_len = htons(20 + 8 + size);
    udp->ip.ip_id = 0;
    udp->ip.ip_off = 0;
    udp->ip.ip_ttl = 64;
    udp->ip.ip_proto = 17;
    udp->ip.ip_chk = 0;
    memcpy(&(udp->ip.ip_src),IP,4);
    memcpy(&(udp->ip.ip_dst),bcast,4);
    udp->ip.ip_chk = ipchksum(&(udp->ip),sizeof(net_ip));
    
    udp->udp.udp_src = htons(5049);
    udp->udp.udp_dst = htons(port);
    udp->udp.udp_len = htons(size + 8);
    udp->udp.udp_chk = 0;
    
    memcpy(udp->data, data, size);
    
/*    printf("sending UDP to %d / %d\n",port,size);*/
    P->pb.len = P->pd.len = 14 + 20 + 8 + size;
    
    xmit(P);
    
}

void control(void)
{
    int nh;
    msg_hdr_t msg;
    char cbuf[1500];
    net_cmd *cmd = (net_cmd *) cbuf;
    int size;
    
    nh = namer_newhandle();
    namer_register(nh, port_net = port_create(0),"net");
    
    msg.flags=0;
    
    for(;;){
        msg.dst = port_net;
        msg.src = 0;
        msg.data = cbuf;
        msg.size = 1500;
        size = port_recv(&msg);

        if(size >= 8){
            switch(cmd->cmd){
            case NET_IP:
                memcpy(cbuf,IP,4);
                msg.size = 4;
                msg.data = cbuf;
                msg.dst = msg.src;
                msg.src = port_isr;
                port_send(&msg);
                break;
                
            case NET_CONNECT:
                pmap[0].udp_port = cmd->port;
                pmap[0].blt_port = msg.src;
                printf("ne2000: routing udp:%d -> blt:%d\n",
                       cmd->port, msg.src);
                
                break;
            case NET_SEND:
                send_udp(cmd->port, NULL, cmd->data, size-8);
                break;
            }
        }
    }

}


void sender(void)
{
    msg_hdr_t mh;
    pbuf *packet;
    
    port_xmit = port_create(port_isr);
/*    port_xmit_done = port_create(port_isr);*/
    sem_xmit_done = sem_create(0);
    
    mh.flags = 0;
    mh.src = port_isr;
    mh.size = 4;
    mh.data = &packet;
    
    for(;;){
            /* wait for a send request */
        mh.dst = port_xmit;
        if(port_recv(&mh) == 4){
                /* send the packet */
            sem_acquire(mutex);
            nic_send_packet(&TheSNIC, &(packet->pb));
            sem_release(mutex);
            
                /* wait for a done response */
/*            mh.dst = port_xmit_done;
            if(port_recv(&mh) == 4){
            }*/
            sem_acquire(sem_xmit_done);
        }    
    }
}

void dispatcher(void)
{
    pbuf *packet;
    unsigned char *b;
    msg_hdr_t msg;
    port_dispatch = port_create(port_isr);

    msg.flags = 0;
    msg.src = port_isr;
    msg.size = 4;
    msg.dst = port_dispatch;
    msg.data = &packet;
    
    for(;;){
        if(port_recv(&msg) == 4){
            b = (unsigned char *) packet->pd.ptr;
            
            if(b[12] == 0x08){
                if(b[13] == 0x00) {
                    handle_ip((ip_packet *) b, packet);
                } else if(b[13] == 0x06) {
                    handle_arp((arp_packet *) b, packet);
                }
            }
            free_buffer_data(&(packet->pd));
        }
    }
}

int main(void)
{
    int i;
    int nh;    
    int snic_irq = 5;    

    if(loadip[0]=='i' && loadip[1]=='p'){
        memcpy(IP,loadip+2,4);   
    }
    
    os_brk(RINGSIZE*PACKETSIZE*2);

    sem_ring = sem_create(1);
    mutex = sem_create(1);

        /* create our send port */
    port_isr = port_create(0);
    port_set_restrict(port_isr, port_isr);

    nh = namer_newhandle();    
/*    while((port_console = namer_find(nh,"console")) < 1)
        for(i=0;i<100000;i++);*/
    namer_register(nh, port_isr,"net_xmit");
    namer_delhandle(nh);

    os_handle_irq(snic_irq);

    init_ring();
    TheSNIC.iobase = 0;
    nic_init(&TheSNIC, 0x300, prom, NULL);

    printf("ne2000: irq %d @ 0x300 mac = %X:%X:%X:%X:%X:%X\n",
           snic_irq,prom[0],prom[1],prom[2],prom[3],prom[4],prom[5]);    

    nic_register_notify(&TheSNIC,receive,NULL);
    
    printf("ne2000: starting sender, dispatcher, and control\n");

    os_thread(sender);
    os_thread(dispatcher);
    os_thread(control);

/*    while(!(port_xmit_done && port_dispatch && port_net)) ;*/
    while(!(sem_xmit_done && port_dispatch && port_net)) ;
    
    nic_start(&TheSNIC,0);
    printf("ne2000: ready.\n");

/*    printf("TS=%X TD=%X\n",ts,td);*/
    
    for(;;){
        os_sleep_irq();
        sem_acquire(mutex);
        nic_isr(&TheSNIC);
        sem_release(mutex);
    }
    nic_stop(&TheSNIC);

    return 0;
}
