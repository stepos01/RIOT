/*
 * sc_key_auth.c
 *
 *  Created on: 26 oct. 2015
 *      Author: stephane
 */



#include "key_auth.h"
#include <stdlib.h>


int _key_auth_init(int argc, char **argv){
	if(argc<4){
		 printf("usage: %s node_id serial_number node_type\n", argv[0]);
		        return 1;
	}
	int node_id = atoi(argv[1]);
	int sn = atoi(argv[2]);
	uint8_t node_type = atoi(argv[3]);
	key_auth_init(node_id,sn,node_type);
	return 0;
}
int _key_auth_add(int argc,char**argv){
	if(argc<3){
		 printf("usage: %s node_id serial_number \n", argv[0]);
		        return 1;
	}
		key_auth_add(argv[1],argv[2]);
		return 0;
}


int _key_auth_start(int argc, char **argv){
	(void)argc;
	(void)argv;
		key_auth_start();
		return 0;
}

