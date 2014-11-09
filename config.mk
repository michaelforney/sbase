# sbase version
VERSION = 0.0

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CPPFLAGS = -D_BSD_SOURCE -D_GNU_SOURCE
CFLAGS   = -g -std=c99 -Wall -pedantic $(CPPFLAGS)
LDFLAGS  = -g

#CC = gcc
#CC = tcc
#CC = musl-gcc
CC = cc
LD = $(CC)

#CPPFLAGS = -D_POSIX_C_SOURCE=200112L
#CFLAGS   = -Os -Wall $(CPPFLAGS) -D_GNU_SOURCE
#LDFLAGS  =
