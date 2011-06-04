# sbase version
VERSION = 0.0

#CC = gcc
CC = musl-gcc
LD = $(CC)
CPPFLAGS = -D_POSIX_C_SOURCE=200112L
CFLAGS   = -Os -ansi -Wall -pedantic $(CPPFLAGS)
LDFLAGS  = -static #-s

#CC = tcc
#LD = $(CC)
#CPPFLAGS = -D_POSIX_C_SOURCE=200112L
#CFLAGS   = -Os -Wall $(CPPFLAGS)
#LDFLAGS  =
