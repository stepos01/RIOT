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

#define KEY_AUTH_STACK_SIZE        (THREAD_STACKSIZE_DEFAULT)
#define KEY_AUTH_PRIO              (THREAD_PRIORITY_MAIN - 1)



#define KEY_AUTH_QUEUE_SIZE    (8U)
#define KEY_AUTH_MSG_TYPE_AUTH_REQUEST 0x01
#define KEY_AUTH_MSG_TYPE_AUTH_TRANSFERT 0x02
#define KEY_AUTH_MSG_TYPE_AUTH_RESPONSE 0x03
#define KEY_AUTH_MSG_TYPE_CONFIRM_REQUEST 0x04
#define KEY_AUTH_MSG_TYPE_CONFIRM_RESPONSE 0x05

#define KEY_AUTH_NODE_TYPE_SIMPLE 1U
#define KEY_AUTH_NODE_TYPE_BROOKER 2U

#define KEY_AUTH_ALGORITHM_TYPE_NONE 0X01
#define KEY_AUTH_ALGORITHM_TYPE_MILENAGE 0X02


#ifdef KEY_AUTH_TEST_NATIVE
#define PORT_TEST 1
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t key_auth_node_type_t;
typedef uint8_t key_auth_algorithm_type_t;
typedef uint8_t key_auth_msg_type_t;
typedef struct key_auth_msg{
	key_auth_msg_type_t msg_type;
#ifdef KEY_AUTH_TEST_NATIVE
	char * dest_addr;
	char * port;
#endif
	uint8_t id;
}key_auth_msg_t;
typedef struct key_auth_msg_request{
	key_auth_msg_t msg;
	//todo remove ID from here and use the one in key_auth_msg_t
	uint8_t id;
	key_auth_algorithm_type_t algorithm_type;


}key_auth_msg_request_t;
typedef struct key_auth_msg_response{
	key_auth_msg_t msg;
	uint8_t autn[16];
	uint8_t rand[16];

}key_auth_msg_response_t;

typedef struct key_auth_msg_confirm_t{
	key_auth_msg_t msg;
	uint8_t res[8];
	uint8_t success;
}key_auth_msg_confirm_t;
typedef struct key_auth_msg_confirm_response_t{
	key_auth_msg_t msg;
	uint8_t success;

}key_auth_msg_confirm_response_t;
typedef struct key_auth_dictionary{
	uint8_t id;
	uint8_t sn;
	struct key_auth_dictionary * next;
	uint8_t res[8];
	uint8_t kasme[20];
}key_auth_dictionary_t;

kernel_pid_t key_auth_init(uint8_t node_id,uint8_t key_auth_node_sn,key_auth_node_type_t node_type );
void key_auth_start(void);
void key_auth_add(char * sn,char *id);


#ifdef __cplusplus
}
#endif


#endif /* KEY_AUTH_H_ */
