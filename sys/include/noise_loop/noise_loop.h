/*
 * noise_loop.h
 *
 *  Created on: Oct 2, 2015
 *      Author: stephanekamga
 */

#ifndef SYS_NOISE_LOOP_INCLUDE_NOISE_LOOP_H_
#define SYS_NOISE_LOOP_INCLUDE_NOISE_LOOP_H_

#include "kernel.h"
#include "pn532.h"
#define NOISE_LOOP_STACK_SIZE        (THREAD_STACKSIZE_DEFAULT)
#define NOISE_LOOP_PRIO              (THREAD_PRIORITY_MAIN - 1)
#define NOISE_LOOP_QUEUE_SIZE    (8U)
#define NOISE_LOOP_ERROR_INIT 	0x01
#define NOISE_LOOP_SAM_CONF_ERROR 	0x02
#define NOISE_LOOP_UNKNOW_ERROR 	0x03
#define NOISE_LOOP_SEND 0x04
#define NOISE_LOOP_RECEIVE 0x05

#define NOISE_LOOP_PKT_TYPE 	0x06

#define noise_loop_delay(X)    (xtimer_usleep(1000*X))


#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 */
kernel_pid_t noise_loop_init(void);
uint8_t pn532_init(void);

#ifdef __cplusplus
}
#endif
#endif /* SYS_NOISE_LOOP_INCLUDE_NOISE_LOOP_H_ */
