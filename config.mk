# sbase version
VERSION = 0.0

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

#CC = gcc
#CC = musl-gcc
LD = $(CC)
CPPFLAGS = -D_BSD_SOURCE -D_GNU_SOURCE
CFLAGS   = -g -ansi -Wall -Wno-long-long -pedantic -fPIC $(CPPFLAGS)
LDFLAGS  = -g

#CC = tcc
#LD = $(CC)
#CPPFLAGS = -D_POSIX_C_SOURCE=200112L
#CFLAGS   = -Os -Wall $(CPPFLAGS)
#LDFLAGS  =
