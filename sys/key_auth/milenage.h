/*
 * milenage.h
 *
 *  Created on: Nov 10, 2015
 *      Author: stephanekamga
 */

#ifndef SYS_KEY_AUTH_MILENAGE_H_
#define SYS_KEY_AUTH_MILENAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char u8;

void f1    ( u8 k[16], u8 rand[16], u8 sqn[6], u8 amf[2],
        u8 mac_a[8], u8 op[16] );

void f2345 ( u8 k[16], u8 rand[16],
        u8 res[8], u8 ck[16], u8 ik[16], u8 ak[6], u8 op[16] );

void f1star( u8 k[16], u8 rand[16], u8 sqn[6], u8 amf[2],
        u8 mac_s[8], u8 op[16] );

void f5star( u8 k[16], u8 rand[16],
        u8 ak[6], u8 op[16] );

void ComputeOPc( u8 op_c[16], u8 op[16] );


#ifdef __cplusplus
}
#endif

#endif /* SYS_KEY_AUTH_MILENAGE_H_ */
