/*
 * sc_key_auth.c
 *
 *  Created on: 26 oct. 2015
 *      Author: stephane
 */



#include "key_auth.h"
#include <stdlib.h>


int _key_auth_init(int argc, char **argv){
	if(argc<3){
		 printf("usage: %s node_id serial_number\n", argv[0]);
		        return 1;
	}
	int node_id = atoi(argv[1]);
	int sn = atoi(argv[2]);
	key_auth_init(node_id,sn);
	return 0;
}

int _key_auth_start(int argc, char **argv){
	if(argc<3){
			 printf("usage: %s ip_v6_address port_number\n", argv[0]);
			        return 1;
		}
		int port = atoi(argv[2]);
		key_auth_start(argv[1],(uint16_t)port);
		return 0;
}

