/*
 * rijndael.h
 *
 *  Created on: Nov 10, 2015
 *      Author: stephanekamga
 */

#ifndef SYS_KEY_AUTH_RIJNDAEL_H_
#define SYS_KEY_AUTH_RIJNDAEL_H_

#ifdef __cplusplus
extern "C" {
#endif

void RijndaelKeySchedule( u8 key[16] );
void RijndaelEncrypt( u8 input[16], u8 output[16] );


#ifdef __cplusplus
}
#endif
#endif /* SYS_KEY_AUTH_RIJNDAEL_H_ */
