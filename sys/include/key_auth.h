/*
 * key_auth.h
 *
 *  Created on: 10 oct. 2015
 *      Author: stephanekamga
 */

#ifndef KEY_AUTH_H_
#define KEY_AUTH_H_
#define KEY_AUTH_TEST_NATIVE 1


#include "kernel.h"
#include "net/ipv6/addr.h"
#ifdef KEY_AUTH_TEST_NATIVE
#else
#include "pn532.h"
#endif
#include "debug.h"

#define KEY_AUTH_STACK_SIZE        (THREAD_STACKSIZE_DEFAULT)
#define KEY_AUTH_PRIO              (THREAD_PRIORITY_MAIN - 1)
#define KEY_AUTH_QUEUE_SIZE    (8U)
#define KEY_AUTH_MSG_TYPE_AUTH_REQUEST 0x01
#define KEY_AUTH_MSG_TYPE_AUTH_TRANSFERT 0x02
#define KEY_AUTH_MSG_TYPE_AUTH_RESPONSE 0x03
#define KEY_AUTH_MSG_TYPE_CONFIRM_REQUEST 0x04
#define KEY_AUTH_MSG_TYPE_CONFIRM_RESPONSE 0x05

#ifdef KEY_AUTH_TEST_NATIVE
#define PORT_TEST 1
#endif

typedef struct key_auth_msg_request{
	uint8_t id;
#ifdef KEY_AUTH_TEST_NATIVE
	char * dest_addr;
#endif
}key_auth_msg_request_t;

#ifdef __cplusplus
extern "C" {
#endif


kernel_pid_t key_auth_init(uint8_t node_id,uint8_t key_auth_node_sn);
void key_auth_start(char * addr,uint16_t port);

#ifdef __cplusplus
}
#endif


#endif /* KEY_AUTH_H_ */
