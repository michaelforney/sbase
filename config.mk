# sbase version
VERSION = 0.0

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

#CC = musl-gcc
LD = $(CC)
RANLIB = ranlib

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_GNU_SOURCE
CFLAGS   = -std=c99 -Wall -pedantic
LDFLAGS  = -s
