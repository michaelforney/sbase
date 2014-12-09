include config.mk

.SUFFIXES:
.SUFFIXES: .o .c

HDR =\
	arg.h\
	compat.h\
	crypt.h\
	fs.h\
	md5.h\
	queue.h\
	runetypebody.h\
	sha1.h\
	sha256.h\
	sha512.h\
	text.h\
	utf.h\
	util.h

LIBUTF = libutf.a
LIBUTFSRC =\
	libutf/readrune.c\
	libutf/rune.c\
	libutf/runetype.c\
	libutf/utf.c\
	libutf/writerune.c

LIBUTIL = libutil.a
LIBUTILSRC =\
	libutil/agetcwd.c\
	libutil/apathmax.c\
	libutil/concat.c\
	libutil/cp.c\
	libutil/crypt.c\
	libutil/ealloc.c\
	libutil/enmasse.c\
	libutil/eprintf.c\
	libutil/eregcomp.c\
	libutil/estrtod.c\
	libutil/estrtol.c\
	libutil/fnck.c\
	libutil/getlines.c\
	libutil/human.c\
	libutil/md5.c\
	libutil/mode.c\
	libutil/putword.c\
	libutil/recurse.c\
	libutil/rm.c\
	libutil/sha1.c\
	libutil/sha256.c\
	libutil/sha512.c\
	libutil/strcasestr.c\
	libutil/strlcat.c\
	libutil/strlcpy.c

LIB = $(LIBUTF) $(LIBUTIL)

BIN =\
	basename\
	cal\
	cat\
	chgrp\
	chmod\
	chown\
	chroot\
	cksum\
	cmp\
	cols\
	comm\
	cp\
	crond\
	cut\
	date\
	dirname\
	du\
	echo\
	env\
	expand\
	expr\
	false\
	fold\
	grep\
	head\
	hostname\
	kill\
	link\
	ln\
	logger\
	logname\
	ls\
	md5sum\
	mkdir\
	mkfifo\
	mktemp\
	mv\
	nice\
	nl\
	nohup\
	paste\
	printenv\
	printf\
	pwd\
	readlink\
	renice\
	rm\
	rmdir\
	seq\
	setsid\
	sha1sum\
	sha256sum\
	sha512sum\
	sleep\
	sort\
	split\
	sponge\
	strings\
	sync\
	tail\
	tar\
	tee\
	test\
	touch\
	tr\
	true\
	tty\
	uname\
	unexpand\
	uniq\
	unlink\
	uudecode\
	uuencode\
	wc\
	xargs\
	yes

LIBUTFOBJ = $(LIBUTFSRC:.c=.o)
LIBUTILOBJ = $(LIBUTILSRC:.c=.o)
OBJ = $(BIN:=.o) $(LIBUTFOBJ) $(LIBUTILOBJ)
SRC = $(BIN:=.c)
MAN = $(BIN:=.1)

all: $(BIN)

$(BIN): $(LIB) $(@:=.o)

$(OBJ): $(HDR) config.mk

.o:
	$(LD) $(LDFLAGS) -o $@ $< $(LIB)

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(LIBUTF): $(LIBUTFOBJ)
	$(AR) -r -c $@ $?
	$(RANLIB) $@

$(LIBUTIL): $(LIBUTILOBJ)
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
	cp -r LICENSE Makefile README TODO config.mk $(SRC) $(MAN) libutf libutil $(HDR) sbase-$(VERSION)
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
	rm -f $(BIN) $(OBJ) $(LIB) sbase-box sbase-$(VERSION).tar.gz

.PHONY:
	all install uninstall dist sbase-box clean
