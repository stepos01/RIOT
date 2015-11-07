/*
 * key_auth.c
 *
 *  Created on: 10 oct. 2015
 *      Author: stephanekamga
 */

#include "key_auth.h"
#include "thread.h"
#include "net/gnrc/pktbuf.h"
#include "od.h"
#include "net/gnrc.h"
#include "net/gnrc/ipv6.h"
#include "net/gnrc/udp.h"
#if ENABLE_DEBUG
static char _stack[KEY_AUTH_STACK_SIZE + THREAD_EXTRA_STACKSIZE_PRINTF];
#else
static char _stack[KEY_AUTH_STACK_SIZE];
#endif


static uint8_t key_auth_node_id = 0;
static uint8_t key_auth_node_sn = 0;
static char * key_auth_test_addr = "0xfe" ;
static volatile kernel_pid_t key_auth_pid = KERNEL_PID_UNDEF;
static void key_auth_print_packet(gnrc_pktsnip_t*p);
static gnrc_netreg_entry_t server = { NULL, GNRC_NETREG_DEMUX_CTX_ALL, KERNEL_PID_UNDEF };


static void *_event_loop(void *args);
static void key_auth_send_auth_request(key_auth_msg_request_t *req);

kernel_pid_t key_auth_init(uint8_t node_id, uint8_t sn) {
	key_auth_node_id = node_id;
	key_auth_node_sn = sn;

	if (key_auth_pid == KERNEL_PID_UNDEF) {
		key_auth_pid = thread_create(_stack, sizeof(_stack), KEY_AUTH_PRIO,
				CREATE_STACKTEST, _event_loop, NULL, "key_auth");
		server.pid = key_auth_pid;
			    	              server.demux_ctx = (uint32_t)1;
			    	              gnrc_netreg_register(GNRC_NETTYPE_UDP, &server);

	}
	return key_auth_pid;
}
void key_auth_start(char * addr,char* port){
	(void)port;
	key_auth_msg_request_t request;
	request.id = key_auth_node_id;
				request.dest_addr = addr;
				request.port =port;

				key_auth_send_auth_request(&request);
}
static void *_event_loop(void *args) {


	(void) args;
	    msg_t m;
	    msg_t msg_queue[KEY_AUTH_QUEUE_SIZE];

	    /* setup the message queue */
	    msg_init_queue(msg_queue, KEY_AUTH_QUEUE_SIZE);



	while (1) {
		msg_receive(&m);
		switch (m.type) {
		case KEY_AUTH_MSG_TYPE_AUTH_REQUEST: {
			key_auth_msg_request_t request;
			request.id = key_auth_node_id;
			request.dest_addr = key_auth_test_addr;

			key_auth_send_auth_request(&request);
			break;
		}
		case KEY_AUTH_MSG_TYPE_AUTH_RESPONSE:{

			break;
		}
		case GNRC_NETAPI_MSG_TYPE_RCV:
		                puts("KEY_AUTH: data received:");
		                key_auth_print_packet((gnrc_pktsnip_t *)m.content.ptr);
		                break;

		default:
			break;
		}
	}

	return NULL;
}
static void key_auth_print_packet(gnrc_pktsnip_t*pkt){
    gnrc_pktsnip_t *snip = pkt;

    key_auth_msg_request_t * data = (key_auth_msg_request_t*)snip->data;
	printf("node ID: %d",data->id);
	    gnrc_pktbuf_release(pkt);
}
static void key_auth_send_auth_request(key_auth_msg_request_t *req) {

	uint8_t port[2];
	    uint16_t tmp;
	    ipv6_addr_t addr;

	    /* parse destination address */
	    if (ipv6_addr_from_str(&addr, req->dest_addr) == NULL) {
	        puts("Error: unable to parse destination address");
	        return;
	    }
	    /* parse port */
	    tmp = (uint16_t)atoi(req->port);
	    if (tmp == 0) {
	        puts("Error: unable to parse destination port");
	        return;
	    }
	    port[0] = (uint8_t)tmp;
	    port[1] = tmp >> 8;

	        gnrc_pktsnip_t *payload, *udp, *ip;
	        /* allocate payload */
	        payload = gnrc_pktbuf_add(NULL, req, sizeof(*req), GNRC_NETTYPE_UNDEF);
	        if (payload == NULL) {
	            puts("Error: unable to copy data to packet buffer");
	            return;
	        }
	        /* allocate UDP header, set source port := destination port */
	        udp = gnrc_udp_hdr_build(payload, port, 2, port, 2);
	        if (udp == NULL) {
	            puts("Error: unable to allocate UDP header");
	            gnrc_pktbuf_release(payload);
	            return;
	        }
	        /* allocate IPv6 header */
	        ip = gnrc_ipv6_hdr_build(udp, NULL, 0, (uint8_t *)&addr, sizeof(addr));
	        if (ip == NULL) {
	            puts("Error: unable to allocate IPv6 header");
	            gnrc_pktbuf_release(udp);
	            return;
	        }
	        /* send packet */
	        if (!gnrc_netapi_dispatch_send(GNRC_NETTYPE_UDP, GNRC_NETREG_DEMUX_CTX_ALL, ip)) {
	            puts("Error: unable to locate UDP thread");
	            gnrc_pktbuf_release(ip);
	            return;
	        }
	        printf("Success: send %u byte to [%s]:%u\n", (unsigned)payload->size,
	               req->dest_addr, tmp);
	        vtimer_usleep(1000*1000);
}

