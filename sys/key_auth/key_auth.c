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
#include "milenage.h"
#include "utlist.h"
#include "net/gnrc.h"
#include "net/gnrc/ipv6.h"
#include "net/gnrc/udp.h"
#include "random.h"
#include "sha1.h"

#if ENABLE_DEBUG
static char _stack[KEY_AUTH_STACK_SIZE + THREAD_EXTRA_STACKSIZE_PRINTF];
#else
static char _stack[KEY_AUTH_STACK_SIZE];
#endif


static uint8_t key_auth_node_id = 0;
static uint8_t key_auth_node_sn = 0;
static uint32_t seq_number = 1;			//should be of size sizeof(char)*6
static key_auth_node_type_t node_type = 0;
static char * key_auth_test_addr = "ff02::1" ;
static char * key_auth_test_port = "1";
static volatile kernel_pid_t key_auth_pid = KERNEL_PID_UNDEF;
static void _rcv(gnrc_pktsnip_t*p);
static void _toArray(uint32_t src,u8 * dest,uint8_t length);
static void _gen_key(key_auth_dictionary_t * dic,key_auth_msg_request_t * request,key_auth_msg_response_t * req);
static gnrc_netreg_entry_t server = { NULL, GNRC_NETREG_DEMUX_CTX_ALL, KERNEL_PID_UNDEF };

static key_auth_dictionary_t * head = NULL;

static void *_event_loop(void *args);
static void _send_auth_request(key_auth_msg_t *req,size_t size);

kernel_pid_t key_auth_init(uint8_t node_id, uint8_t sn,key_auth_node_type_t type) {
	key_auth_node_id = node_id;
	key_auth_node_sn = sn;
	node_type = type;

	if (key_auth_pid == KERNEL_PID_UNDEF) {
		key_auth_pid = thread_create(_stack, sizeof(_stack), KEY_AUTH_PRIO,
				CREATE_STACKTEST, _event_loop, NULL, "key_auth");
		server.pid = key_auth_pid;
		server.demux_ctx = (uint32_t)1;
		gnrc_netreg_register(GNRC_NETTYPE_UDP, &server);

	}
	return key_auth_pid;
}
void key_auth_start(void){
	key_auth_msg_request_t request;
	request.id = key_auth_node_id;
	request.algorithm_type = KEY_AUTH_ALGORITHM_TYPE_MILENAGE;
	request.msg.dest_addr =key_auth_test_addr;
	request.msg.port = key_auth_test_port;
	request.msg.msg_type = KEY_AUTH_MSG_TYPE_AUTH_REQUEST;
	_send_auth_request((key_auth_msg_t*)(&request),sizeof(request));
}
void key_auth_add(char * charSN,char *charId){
	uint8_t id = (uint8_t)(atoi(charId));
	uint8_t sn = (uint8_t)(atoi(charSN));
	key_auth_dictionary_t * search;
	LL_SEARCH_SCALAR(head,search,id,id);
	if(search && search->id==id){
		puts("ID already registered!");
		return;
	}
	key_auth_dictionary_t * entry = malloc(sizeof(key_auth_dictionary_t));
	entry->id = id;entry->sn = sn;
	LL_APPEND(head,entry);
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
		case GNRC_NETAPI_MSG_TYPE_RCV:
			_rcv((gnrc_pktsnip_t *)m.content.ptr);
			break;

		default:
			break;
		}
	}

	return NULL;
}
static void _gen_key(key_auth_dictionary_t * dic,key_auth_msg_request_t * request,key_auth_msg_response_t * response){
	//uint32_t rand = genrand_uint32();
	(void)dic;
	(void)request;
	switch (request->algorithm_type) {
	case KEY_AUTH_ALGORITHM_TYPE_MILENAGE:
	{
		u8 rand[16] = {0},key[16] = {0},op[16] = {0},ck[16] = {0},ik[16] = {0};
		u8 sqn[6] = {0},ak[6]={0};u8 amf[2]={0};
		u8 mac[8] = {0},res[8] =  {0};
		uint32_t a = genrand_uint32();
		_toArray(a,rand,4);
		a = genrand_uint32();
		_toArray(a,rand+4,4);
		a = genrand_uint32();
		_toArray(a,rand+8,4);
		a = genrand_uint32();
		_toArray(a,rand+12,4);

		for(int i=0;i<16;i++)
		{
			key[i]=request->id;
		}
		seq_number ++;
		_toArray(seq_number,sqn,6);
		f1(key,rand,sqn,amf,mac,op);
		f2345(key,rand,res,ck,ik,ak,op);

		u8 sha_key[32]={0};
		u8 sqnXORak[6]={0};
		for(int i=0;i<6;i++)
				{
			sqnXORak[i]=ak[i]^sqn[i];
				}
		memcpy(sha_key,ck,16);
		memcpy(sha_key+16,ik,16);
		u8 sha_text[12]= {0};
		sha_text[0]=0x01;

		memcpy(sha_text+4,sqnXORak,sizeof(sqnXORak));



		uint8_t kasme[20];
		hmac_sha1(sha_text,sizeof(sha_text),sha_key,sizeof(sha_key),kasme);


		memcpy(response->autn,sqnXORak,6);
		memcpy(response->autn+6,amf,2);
		memcpy(response->autn+8,mac,8);
		memcpy(response->rand,rand,16);

		memcpy(dic->kasme,kasme,sizeof(kasme));
		memcpy(dic->res,res,sizeof(res));

		printf("autn %d\n",response->autn[0]);
		printf("rand %d\n",response->rand[0]);
		break;
	}
	case KEY_AUTH_ALGORITHM_TYPE_NONE:


		break;
	default:
		break;
	}


}

static void _toArray(uint32_t src,u8 * dest,uint8_t length)
{
	for(int i = 0;i<length;i++)
	{
		dest[i]=src >> (i*8);
	}
}
static void _rcv(gnrc_pktsnip_t*pkt){
	gnrc_pktsnip_t *snip = pkt;
	key_auth_msg_t * msg = (key_auth_msg_t*)snip->data;
	switch (msg->msg_type) {
	case KEY_AUTH_MSG_TYPE_AUTH_REQUEST:
	{
		if(node_type==KEY_AUTH_NODE_TYPE_BROOKER)
		{
			key_auth_msg_request_t * req = (key_auth_msg_request_t*)snip->data;
			puts("KEY_AUTH_MSG_TYPE_AUTH_REQUEST received");
			key_auth_dictionary_t * entry;
			LL_SEARCH_SCALAR(head,entry,id,req->id);
			if(entry && entry->id==req->id){
				//found
				printf("SN found:id: %d => sn: %d\n",entry->id,entry->sn);
				key_auth_msg_response_t response;
				_gen_key(entry,req,&response);
				response.msg.msg_type = KEY_AUTH_MSG_TYPE_AUTH_RESPONSE;
				response.msg.port = key_auth_test_port;
				response.msg.dest_addr = key_auth_test_addr;
				_send_auth_request((key_auth_msg_t*)(&response),sizeof(response));


			}else{
				printf("No node with id %d found\n",req->id);
			}

		}else{
			puts("Not supported!");
		}
		break;
	}
	case KEY_AUTH_MSG_TYPE_AUTH_RESPONSE:
	{
		if(node_type==KEY_AUTH_NODE_TYPE_BROOKER)
		{
			key_auth_msg_response_t * response = (key_auth_msg_response_t*)snip->data;
			puts("KEY_AUTH_MSG_TYPE_AUTH_RESPONSE received");




		}else{
			puts("Not supported!");
		}
		break;
	}
	default:
		break;
	}
	gnrc_pktbuf_release(pkt);
}



static void _send_auth_request(key_auth_msg_t *req,size_t size) {

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
	payload = gnrc_pktbuf_add(NULL, req, size, GNRC_NETTYPE_UNDEF);
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

