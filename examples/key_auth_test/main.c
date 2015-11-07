/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Hello World application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Ludwig Ortmann <ludwig.ortmann@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include "shell.h"
#include "msg.h"
#include "net/gnrc.h"
#include "net/gnrc/ipv6.h"
#include "net/gnrc/udp.h"
#include "net/gnrc/pktdump.h"
#include "timex.h"
//static gnrc_netreg_entry_t server = { NULL, GNRC_NETREG_DEMUX_CTX_ALL, KERNEL_PID_UNDEF };

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

static const shell_command_t shell_commands[] = {
    { NULL, NULL, NULL }
};
int main(void)
{
    puts("Hello key_auth_test!");

    /* we need a message queue for the thread running the shell in order to
        * receive potentially fast incoming networking packets */
       msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
//       server.pid = gnrc_pktdump_getpid();
//          server.demux_ctx = (uint32_t)1;
//          gnrc_netreg_register(GNRC_NETTYPE_UDP, &server);
       /* start shell */
       puts("All up, running the shell now");
       char line_buf[SHELL_DEFAULT_BUFSIZE];
       shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

       /* should be never reached */
       return 0;

}
