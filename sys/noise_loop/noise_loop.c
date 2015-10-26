/*
 * noise_loop.c
 *
 *  Created on: Oct 2, 2015
 *      Author: stephanekamga
 */


#include "noise_loop/noise_loop.h"
#include "debug.h"
#include "msg.h"
#include "xtimer.h"
#include "net/gnrc/pktbuf.h"

#if ENABLE_DEBUG
static char _stack[NOISE_LOOP_STACK_SIZE + THREAD_EXTRA_STACKSIZE_PRINTF];
#else
static char _stack[NOISE_LOOP_STACK_SIZE];
#endif

kernel_pid_t noise_loop_pid = KERNEL_PID_UNDEF;
static void *_event_loop(void *args);
static uint8_t nfc_send(gnrc_pktsnip_t*pkt);
static uint8_t nfc_receive(uint8_t *buf,uint8_t *buflen);
static pn532_t pn532;


kernel_pid_t noise_loop_init(void){
	if(pn532_init()==NOISE_LOOP_ERROR_INIT){
		DEBUG("Error while initializing PN532! \n");
		return KERNEL_PID_UNDEF;
	}
	if (noise_loop_pid == KERNEL_PID_UNDEF) {
		noise_loop_pid = thread_create(_stack, sizeof(_stack), NOISE_LOOP_PRIO,
				CREATE_STACKTEST, _event_loop, NULL, "noise_loop");
	}
	return noise_loop_pid;
}
uint8_t pn532_init(void){
	pn532_t * dev = &pn532;
	pn532_init_master(dev, SPI_0, SPI_SPEED_1MHZ, GPIO_17 );
	DEBUG("SPI initialized.");
	DEBUG("NFC initialized.");
	pn532_begin();

	uint8_t response, ic, version, rev, support;
	pn532_get_firmware_version( &response, &ic, &version, &rev, &support);

	if( response != 0x03 ){
		DEBUG("Response byte of Get Firmware Version wrong! HALT\n");
		return NOISE_LOOP_ERROR_INIT;
	}
	if( ic != 0x32 ){
		DEBUG("Didn't find PN53x board.\n ");
		return NOISE_LOOP_ERROR_INIT;
	}
	if( version != 0x01 ){
		DEBUG("Version received not expected!\n");
	}
	//printf("Here it is the frame version:\n| %X | %X | %X | %X | %X |\n", response, ic, version, rev, support);
	return 0;
}
static void *_event_loop(void *args)
{
	msg_t msg, reply, msg_q[NOISE_LOOP_QUEUE_SIZE];


	(void)args;
	msg_init_queue(msg_q, NOISE_LOOP_QUEUE_SIZE);

	while (1) {
		DEBUG("noise_loop: waiting for incoming message.\n");
		msg_receive(&msg);
		reply.sender_pid  = noise_loop_pid;
		reply.type = NOISE_LOOP_PKT_TYPE;
		switch (msg.type) {
		case NOISE_LOOP_SEND:
			DEBUG("noise_loop: NOISE_LOOP_SEND received!.\n");
			nfc_send((gnrc_pktsnip_t*)msg.content.ptr);
			break;
		case NOISE_LOOP_RECEIVE:
			DEBUG("noise_loop: NOISE_LOOP_RECEIVE received!.\n");
			uint8_t res[255];
			uint8_t res_size =0;
			nfc_receive(res,&res_size);
			gnrc_pktsnip_t *payload;
	        payload = gnrc_pktbuf_add(NULL, res,res_size , GNRC_NETTYPE_UNDEF);
	        if (payload == NULL) {
	                    DEBUG("Error: unable to copy data to packet buffer");
	         }else{
	 	        reply.content.ptr = (void*)payload;
	        	 msg_send(&reply,msg.sender_pid);
	         }
			break;
		default:
			DEBUG("noise_loop: operation not supported\n");

			break;
		}
	}

	return NULL;
}
static uint8_t nfc_receive(uint8_t *buf,uint8_t *buflen){
	uint8_t data[255];
	uint8_t dlen = 0;

	uint8_t nad, did, atr, rats, picc;
	nad = 0x00;
	did = 0x00;
	atr = 0x00;
	rats = 0x01;
	picc = 0x01;
	if(! pn532_SAM_config() ){
		DEBUG("Configuration SAM didn't end well! HALT");
		return NOISE_LOOP_SAM_CONF_ERROR;
	}
	pn532_set_parameters( nad, did, atr, rats, picc );
	uint8_t mode = 0x02;	//DEP only
	uint8_t status_target = pn532_tg_init_as_target( mode );
	//	uint8_t mi = 0;
	if( status_target != 1) {
		printf("Initialization didn't successfully finish! HALT\n");
		noise_loop_delay(10);
	}else{
		uint8_t gt[] = { 0x8C, 0x29, 0x0E, 0x04, 0x34, 0xD7, 0x90, 0xE5, 0x61, 0xE0 };
		pn532_tg_set_general_bytes( gt, 10 );
		noise_loop_delay(300);
		pn532_tg_get_initiator_command(data, &dlen );

		for(int k=7; k < dlen ;k++ ){
			printf("	...received");
			//printf("\n received: %02x",data[k]);
			//responsetmp[k-7]=data[k];
			buf[k-7]=data[k];
		}
		*buflen=dlen;
		noise_loop_delay(150);
		pn532_tg_get_target_status();
		//puts("received. stop.");

	}
	return 1;
}
static uint8_t nfc_send(gnrc_pktsnip_t*pkt){
	uint8_t data[255];
	uint8_t dlen;
	uint8_t atr_res[255];
	uint8_t alen = 0;
	if(! pn532_SAM_config() ){
		DEBUG("Configuration SAM didn't end well! HALT");
		return NOISE_LOOP_SAM_CONF_ERROR;
	}
	noise_loop_delay(1000);
	return 0;
	DEBUG("\nInitializing PN532 as Initiator!\n");
	uint8_t actpass = 0x01;	//Active mode
	uint8_t br = 0x02;		//424 kbps
	uint8_t next = 0x00;
	uint8_t status_initiator = pn532_in_jump_for_dep( actpass, br, next, atr_res, &alen );
	if( status_initiator != 1 ) {
		printf("Something went wrong!\n");
		return NOISE_LOOP_UNKNOW_ERROR;
	}else{
		uint8_t mi = 0;
		do{
			pn532_in_data_exchange( (uint8_t*)pkt->data, (uint8_t)pkt->size, 1, &mi, data, &dlen );
		}while( mi == 1 );
		noise_loop_delay(1);
		mi = 0;
		//pn532_in_data_exchange( &pn532, message, 0, 1, &mi, data, &dlen );

		printf("	...sending");
	}
	return 1;
}



