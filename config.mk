# sbase version
VERSION = 0.0

#CC = cc
#CC = musl-gcc

AR = ar

CPPFLAGS = -D_BSD_SOURCE -D_POSIX_C_SOURCE=2
CFLAGS   = -Os -ansi -Wall -pedantic $(CPPFLAGS)
LDFLAGS  = -s -static
