# sbase version
VERSION = 0.0

#CC = cc
CC = musl-gcc

CPPFLAGS = -D_BSD_SOURCE
CFLAGS   = -Os -ansi -Wall -pedantic $(CPPFLAGS)
LDFLAGS  = -s -static
