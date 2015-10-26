/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief      	noise loop key exchange application
 *
 * @author
 * @author
 *
 * @}
 */

#include <stdio.h>
#include "noise_loop/noise_loop.h"
#include "thread.h"
#include "net/gnrc/pkt.h"
#include "net/gnrc/pktbuf.h"

static char _stack[THREAD_STACKSIZE_DEFAULT + THREAD_EXTRA_STACKSIZE_PRINTF];
static void *_event_loop(void *args);

#define TARGET 		0
#define INITIATOR 		1
int main(void)
{
    puts("noise_loop_test!");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);
    kernel_pid_t noise_loop_pid = noise_loop_init();

    if(KERNEL_PID_UNDEF!=noise_loop_pid)puts("\nNoise loop init!\n");
    else return 0;
    kernel_pid_t noise_loop_dump = thread_create(_stack, sizeof(_stack), (THREAD_PRIORITY_MAIN - 1),
    				CREATE_STACKTEST, _event_loop, NULL, "noise_loop_pkt_dump");
    if (noise_loop_dump != KERNEL_PID_UNDEF) {
          puts("PKTDUMP thread not initialized!\n");
            return 0;
        }
#ifdef INITIATOR
    gnrc_pktsnip_t *pkt;
    int data[2];
    data[0]=atoi("N");
    data[1]=atoi("L");
    pkt = gnrc_pktbuf_add(NULL,data,2*sizeof(int),GNRC_NETTYPE_UNDEF);
    if(pkt==NULL)puts("Could not created the pkt!\n");
    else{
    	msg_t msg;
    	msg.sender_pid = noise_loop_dump;
    	msg.content.ptr =(void*)pkt;
    	msg.type = NOISE_LOOP_SEND;
    	msg_send(&msg,noise_loop_pid);

    	//wait 4 ACK
    	msg_t msg2;
    	msg2.sender_pid = noise_loop_dump;
    	msg2.type = NOISE_LOOP_RECEIVE;
    	msg_send(&msg2,noise_loop_pid);

    }
#else
    msg_t msg2;
        	msg2.sender_pid = noise_loop_dump;
        	msg2.type = NOISE_LOOP_RECEIVE;
        	msg_send(&msg2, noise_loop_pid);

#endif
    return 0;
}
static void *_event_loop(void *args){
	 (void) args;

	    msg_t m;

	    while (1) {
	        msg_receive(&m);
	        printf("2nd: Got msg from %" PRIkernel_pid "\n", m.sender_pid);
	        gnrc_pktsnip_t * pkt = (gnrc_pktsnip_t*)(m.content.ptr);
	        int data[2]={0};
	        if(pkt->size!=2*sizeof(int))puts("ERROR: received wrong data size!\n");
	        memcpy(data,pkt->data,pkt->size);
	        if(data[0]!=atoi("N") ||  data[1]!=atoi("L") )
	        	puts("ERROR: received wrong data size!\n");
	    }

	    return NULL;
}
