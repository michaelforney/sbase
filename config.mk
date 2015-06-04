# sbase version
VERSION = 0.0

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CC = cc
LD = $(CC)
AR = ar
RANLIB = ranlib

# for NetBSD add -D_NETBSD_SOURCE
# for glibc on 32bit add -D_FILE_OFFSET_BITS=64 or use something sane
# -lrt might be needed on some systems
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700
CFLAGS   = -std=c99 -Wall -pedantic
LDFLAGS  = -s
