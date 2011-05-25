# sbase version
VERSION = 0.0

#CC = cc
#CC = musl-gcc

CPPFLAGS = -D_POSIX_C_SOURCE=200112L
CFLAGS   = -Os -ansi -Wall -pedantic $(CPPFLAGS)
LDFLAGS  = -s -static
