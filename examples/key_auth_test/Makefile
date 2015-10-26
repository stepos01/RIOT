# name of your application
APPLICATION = key_auth_test

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1


USEMODULE += xtimer
USEMODULE += shell
USEMODULE += shell_commands

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netif_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for IPv6 and UDP
USEMODULE += gnrc_ipv6
USEMODULE += gnrc_udp
USEMODULE += gnrc_pktbuf_static
USEMODULE += inet_csum
USEMODULE += od
# Add a routing protocol
# This application dumps received packets to STDIO using the pktdump module
# Additional networking modules that can be dropped if not needed
# Add also the shell, some shell commands
USEMODULE += gnrc_ipv6_hdr
USEMODULE += gnrc_netreg
USEMODULE += gnrc_netapi
USEMODULE += key_auth
include $(RIOTBASE)/Makefile.include
