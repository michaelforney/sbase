.POSIX:
include config.mk

.SUFFIXES:
.SUFFIXES: .y .o .c

CPPFLAGS =\
	-DPREFIX=\"$(PREFIX)\" \
	-D_DEFAULT_SOURCE \
	-D_NETBSD_SOURCE \
	-D_BSD_SOURCE \
	-D_XOPEN_SOURCE=700 \
	-D_FILE_OFFSET_BITS=64

HDR =\
	arg.h\
	compat.h\
	crypt.h\
	fs.h\
	md5.h\
	queue.h\
	sha1.h\
	sha224.h\
	sha256.h\
	sha384.h\
	sha512.h\
	sha512-224.h\
	sha512-256.h\
	text.h\
	utf.h\
	util.h

LIBUTFOBJ =\
	libutf/fgetrune.o\
	libutf/fputrune.o\
	libutf/isalnumrune.o\
	libutf/isalpharune.o\
	libutf/isblankrune.o\
	libutf/iscntrlrune.o\
	libutf/isdigitrune.o\
	libutf/isgraphrune.o\
	libutf/isprintrune.o\
	libutf/ispunctrune.o\
	libutf/isspacerune.o\
	libutf/istitlerune.o\
	libutf/isxdigitrune.o\
	libutf/lowerrune.o\
	libutf/rune.o\
	libutf/runetype.o\
	libutf/upperrune.o\
	libutf/utf.o\
	libutf/utftorunestr.o

LIBUTILOBJ =\
	libutil/concat.o\
	libutil/cp.o\
	libutil/crypt.o\
	libutil/confirm.o\
	libutil/ealloc.o\
	libutil/enmasse.o\
	libutil/eprintf.o\
	libutil/eregcomp.o\
	libutil/estrtod.o\
	libutil/fnck.o\
	libutil/fshut.o\
	libutil/getlines.o\
	libutil/human.o\
	libutil/linecmp.o\
	libutil/md5.o\
	libutil/memmem.o\
	libutil/mkdirp.o\
	libutil/mode.o\
	libutil/parseoffset.o\
	libutil/putword.o\
	libutil/reallocarray.o\
	libutil/recurse.o\
	libutil/rm.o\
	libutil/sha1.o\
	libutil/sha224.o\
	libutil/sha256.o\
	libutil/sha384.o\
	libutil/sha512.o\
	libutil/sha512-224.o\
	libutil/sha512-256.o\
	libutil/strcasestr.o\
	libutil/strlcat.o\
	libutil/strlcpy.o\
	libutil/strsep.o\
	libutil/strnsubst.o\
	libutil/strtonum.o\
	libutil/unescape.o\
	libutil/writeall.o

LIB = libutf.a libutil.a

BIN =\
	basename\
	bc\
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
	cron\
	cut\
	date\
	dc\
	dd\
	dirname\
	du\
	echo\
	ed\
	env\
	expand\
	expr\
	false\
	find\
	flock\
	fold\
	getconf\
	grep\
	head\
	hostname\
	join\
	kill\
	link\
	ln\
	logger\
	logname\
	ls\
	make/make\
	md5sum\
	mkdir\
	mkfifo\
	mknod\
	mktemp\
	mv\
	nice\
	nl\
	nohup\
	od\
	paste\
	pathchk\
	printenv\
	printf\
	pwd\
	readlink\
	renice\
	rev\
	rm\
	rmdir\
	sed\
	seq\
	setsid\
	sha1sum\
	sha224sum\
	sha256sum\
	sha384sum\
	sha512sum\
	sha512-224sum\
	sha512-256sum\
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
	tftp\
	time\
	touch\
	tr\
	true\
	tsort\
	tty\
	uname\
	unexpand\
	uniq\
	unlink\
	uudecode\
	uuencode\
	wc\
	which\
	whoami\
	xargs\
	xinstall\
	yes

MAKEOBJ =\
	make/defaults.o\
	make/main.o\
	make/parser.o\
	make/posix.o\
	make/rules.o\

OBJ = $(LIBUTFOBJ) $(LIBUTILOBJ) $(MAKEOBJ)

all: scripts/make
	+@$(SMAKE) $(BIN)

scripts/make:
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ make/*.c

$(BIN): $(LIB)

$(OBJ) $(BIN): $(HDR)

.o:
	$(CC) $(LDFLAGS) -o $@ $< $(LIB)

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

.c:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIB)

bc: bc.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ bc.c $(LIB)

$(MAKEOBJ): make/make.h

make/make: $(MAKEOBJ)
	$(CC) $(LDFLAGS) -o $@ $(MAKEOBJ) $(LIB)

libutf.a: $(LIBUTFOBJ)
	$(AR) $(ARFLAGS) $@ $?
	$(RANLIB) $@

libutil.a: $(LIBUTILOBJ)
	$(AR) $(ARFLAGS) $@ $?
	$(RANLIB) $@

getconf: getconf.h

getconf.h:
	scripts/getconf.sh > $@

proto: $(BIN)
	scripts/mkproto $(DESTDIR)$(PREFIX) $(DESTDIR)$(MANPREFIX) proto

install uninstall: proto
	scripts/$@ proto

sbase-box-install: sbase-box proto
	scripts/install proto
	$(DESTDIR)$(PREFIX)/bin/sbase-box -i $(DESTDIR)$(PREFIX)/bin/

sbase-box-uninstall: sbase-box proto
	$(DESTDIR)$(PREFIX)/bin/sbase-box -d $(DESTDIR)$(PREFIX)/bin/
	scripts/uninstall proto

tests: all
	@cd $@ && $(MAKE)

dist: clean
	mkdir -p sbase
	cp LICENSE Makefile README TODO config.mk *.c *.1 *.h  sbase
	cp -R libutf libutil make scripts tests sbase
	mv sbase sbase-$(VERSION)
	tar -cf sbase-$(VERSION).tar sbase-$(VERSION)
	gzip sbase-$(VERSION).tar
	rm -rf sbase-$(VERSION)

sbase-box: $(BIN)
	scripts/mkbox
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ build/*.c $(LIB)

clean:
	@cd tests && $(MAKE) clean
	rm -f *.o
	rm -f $(BIN) $(LIB) sbase-box sbase-$(VERSION).tar.gz
	rm -f scripts/make
	rm -f getconf.h bc.c
	rm -f proto
	rm -rf build

.PHONY: all install uninstall dist sbase-box-install sbase-box-uninstall clean
