# name of your application
APPLICATION = noise_loop_test

# If no BOARD is found in the environment, use this default:
BOARD ?= udoo

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include

USEMODULE += xtimer
USEMODULE += noise_loop
USEMODULE += pn532
USEMODULE+=	gnrc_pktbuf_static
