include config.mk

.POSIX:
.SUFFIXES: .c .o

HDR = \
	arg.h    \
	crypt.h  \
	fs.h     \
	md5.h    \
	queue.h  \
	sha1.h   \
	sha256.h \
	sha512.h \
	text.h   \
	util.h

LIB = libutil.a
LIBSRC = \
	util/agetcwd.c   \
	util/agetline.c  \
	util/apathmax.c  \
	util/concat.c    \
	util/cp.c        \
	util/crypt.c     \
	util/ealloc.c    \
	util/enmasse.c   \
	util/eprintf.c   \
	util/eregcomp.c  \
	util/estrtod.c   \
	util/estrtol.c   \
	util/fnck.c      \
	util/getlines.c  \
	util/human.c     \
	util/md5.c       \
	util/mode.c      \
	util/putword.c   \
	util/recurse.c   \
	util/rm.c        \
	util/sha1.c      \
	util/sha256.c    \
	util/sha512.c    \
	util/strlcat.c   \
	util/strlcpy.c

BIN = \
	basename \
	cal      \
	cat      \
	chgrp    \
	chmod    \
	chown    \
	chroot   \
	cksum    \
	cmp      \
	col      \
	cols     \
	comm     \
	cp       \
	csplit   \
	cut      \
	date     \
	dirname  \
	du       \
	echo     \
	env      \
	expand   \
	expr     \
	false    \
	fold     \
	grep     \
	head     \
	hostname \
	kill     \
	link     \
	ln       \
	logname  \
	ls       \
	md5sum   \
	mkdir    \
	mkfifo   \
	mktemp   \
	mv       \
	nice     \
	nl       \
	nohup    \
	paste    \
	printenv \
	printf   \
	pwd      \
	readlink \
	renice   \
	rm       \
	rmdir    \
	sleep    \
	setsid   \
	sort     \
	split    \
	sponge   \
	strings  \
	sync     \
	tail     \
	tar      \
	tee      \
	test     \
	touch    \
	tr       \
	true     \
	tty      \
	uudecode \
	uuencode \
	uname    \
	unexpand \
	uniq     \
	unlink   \
	seq      \
	sha1sum  \
	sha256sum\
	sha512sum\
	wc       \
	xargs    \
	yes

LIBOBJ = $(LIBSRC:.c=.o)
OBJ = $(BIN:=.o) $(LIBOBJ)
SRC = $(BIN:=.c)
MAN = $(BIN:=.1)

all: binlib

binlib: $(LIB)
	$(MAKE) bin

bin: $(BIN)

$(OBJ): $(HDR) config.mk

.o:
	$(LD) $(LDFLAGS) -o $@ $< $(LIB)

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(LIB): $(LIBOBJ)
	$(AR) -r -c $@ $?
	$(RANLIB) $@

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin
	cd $(DESTDIR)$(PREFIX)/bin && chmod 755 $(BIN)
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	for m in $(MAN); do sed "s/VERSION/$(VERSION)/g" < "$$m" > $(DESTDIR)$(MANPREFIX)/man1/"$$m"; done
	cd $(DESTDIR)$(MANPREFIX)/man1 && chmod 644 $(MAN)

uninstall:
	cd $(DESTDIR)$(PREFIX)/bin && rm -f $(BIN)
	cd $(DESTDIR)$(MANPREFIX)/man1 && rm -f $(MAN)

dist: clean
	mkdir -p sbase-$(VERSION)
	cp -r LICENSE Makefile README TODO config.mk $(SRC) $(MAN) util $(HDR) sbase-$(VERSION)
	tar -cf sbase-$(VERSION).tar sbase-$(VERSION)
	gzip sbase-$(VERSION).tar
	rm -rf sbase-$(VERSION)

sbase-box: $(LIB) $(SRC)
	mkdir -p build
	cp $(HDR) build
	for f in $(SRC); do sed "s/^main(/`basename $$f .c`_&/" < $$f > build/$$f; done
	echo '#include <libgen.h>'  > build/$@.c
	echo '#include <stdio.h>'  >> build/$@.c
	echo '#include <stdlib.h>' >> build/$@.c
	echo '#include <string.h>' >> build/$@.c
	echo '#include "util.h"'   >> build/$@.c
	for f in $(SRC); do echo "int `basename $$f .c`_main(int, char **);" >> build/$@.c; done
	echo 'int main(int argc, char *argv[]) { char *s = basename(argv[0]); if(!strcmp(s,"sbase-box")) { argc--; argv++; s = basename(argv[0]); } if(0) ;' >> build/$@.c
	for f in $(SRC); do echo "else if(!strcmp(s, \"`basename $$f .c`\")) return `basename $$f .c`_main(argc, argv);" >> build/$@.c; done
	echo 'else {' >> build/$@.c
	for f in $(SRC); do echo "printf(\"`basename $$f .c`\"); putchar(' ');" >> build/$@.c; done
	echo "putchar(0xa); }; return 0; }" >> build/$@.c
	$(LD) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ build/*.c $(LIB)
	rm -r build

clean:
	rm -f $(BIN) $(OBJ) $(LIBOBJ) $(LIB) sbase-box sbase-$(VERSION).tar.gz

.PHONY:
	all binlib bin install uninstall dist sbase-box clean
