/*
 * key_auth.c
 *
 *  Created on: 10 oct. 2015
 *      Author: stephanekamga
 */

#include "key_auth.h"
#include "thread.h"
#include "debug.h"
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


static void *_event_loop(void *args);
static void key_auth_send_auth_request(key_auth_msg_request_t *req);

kernel_pid_t key_auth_init(uint8_t node_id, uint8_t sn) {
	key_auth_node_id = node_id;
	key_auth_node_sn = sn;

	if (key_auth_pid == KERNEL_PID_UNDEF) {
		key_auth_pid = thread_create(_stack, sizeof(_stack), KEY_AUTH_PRIO,
				CREATE_STACKTEST, _event_loop, NULL, "key_auth");

	}
	return key_auth_pid;
}
void key_auth_start(char * addr,uint16_t port){
	(void)port;
	key_auth_msg_request_t *request = NULL;
	request->id = key_auth_node_id;
				request->dest_addr = addr;
				gnrc_pktsnip_t * pkt = NULL;
				pkt = gnrc_pktbuf_add(NULL, (void*) request, sizeof(*request),
						GNRC_NETTYPE_UNDEF);
				key_auth_send_auth_request(request);
}
static void *_event_loop(void *args) {

	gnrc_netreg_entry_t entry = { NULL, PORT_TEST, key_auth_pid };
	gnrc_netreg_register(GNRC_NETTYPE_UDP, &entry);
	(void) args;
	    msg_t msg;
	    msg_t msg_queue[KEY_AUTH_QUEUE_SIZE];

	    /* setup the message queue */
	    msg_init_queue(msg_queue, KEY_AUTH_QUEUE_SIZE);

	msg_t m;

	while (1) {
		msg_receive(&m);
		switch (m.type) {
		case KEY_AUTH_MSG_TYPE_AUTH_REQUEST: {
			key_auth_msg_request_t *request = NULL;
			request->id = key_auth_node_id;
			request->dest_addr = key_auth_test_addr;
			gnrc_pktsnip_t * pkt = NULL;
			pkt = gnrc_pktbuf_add(NULL, (void*) request, sizeof(*request),
					GNRC_NETTYPE_UNDEF);
			key_auth_send_auth_request(request);
			break;
		}
		case KEY_AUTH_MSG_TYPE_AUTH_RESPONSE:{

			break;
		}
		case GNRC_NETAPI_MSG_TYPE_RCV:
		                puts("KEY_AUTH: data received:");
		                key_auth_print_packet((gnrc_pktsnip_t *)msg.content.ptr);
		                break;

		default:
			break;
		}
	}

	return NULL;
}
static void key_auth_print_packet(gnrc_pktsnip_t*pkt){
    gnrc_pktsnip_t *snip = pkt;

	 while (snip != NULL) {
	        printf("~~ SNIP %2i - size: %3u byte, type: ", (int)snip->data,
	               (unsigned int)snip->size);
            od_hex_dump(snip->data, snip->size, OD_WIDTH_DEFAULT);
	        snip = snip->next;
	    }
	    gnrc_pktbuf_release(pkt);
}
static void key_auth_send_auth_request(key_auth_msg_request_t *req) {

	ipv6_addr_t addr;

	/* parse destination address */
	if (ipv6_addr_from_str(&addr, req->dest_addr) == NULL) {
		puts("Error: unable to parse destination address");
		return;
	}

	gnrc_pktsnip_t *payload, *udp, *ip;
	/* allocate payload */
	payload = gnrc_pktbuf_add(NULL, req, sizeof(*req), GNRC_NETTYPE_UNDEF);
	if (payload == NULL) {
		puts("Error: unable to copy data to packet buffer");
		return;
	}
	/* allocate UDP header, set source port := destination port */
	uint8_t port[2];

	port[0] = (uint8_t) PORT_TEST;
	port[1] = 0;
	udp = gnrc_udp_hdr_build(payload, port, 2, port, 2);
	if (udp == NULL) {
		puts("Error: unable to allocate UDP header");
		gnrc_pktbuf_release(payload);
		return;
	}
	/* allocate IPv6 header */
	ip = gnrc_ipv6_hdr_build(udp, NULL, 0, (uint8_t *) &addr, sizeof(addr));
	if (ip == NULL) {
		puts("Error: unable to allocate IPv6 header");
		gnrc_pktbuf_release(udp);
		return;
	}
	/* send packet */
	if (!gnrc_netapi_dispatch_send(GNRC_NETTYPE_UDP, GNRC_NETREG_DEMUX_CTX_ALL,
			ip)) {
		puts("Error: unable to locate UDP thread");
		gnrc_pktbuf_release(ip);
		return;
	}

	vtimer_usleep(1000 * 100);
}

