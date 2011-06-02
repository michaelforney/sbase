include config.mk

HDR = text.h util.h
LIB = util/afgets.o util/agetcwd.o util/concat.o util/enmasse.o util/eprintf.o \
      util/recurse.o

SRC = basename.c cat.c chmod.c chown.c date.c dirname.c echo.c false.c grep.c \
      head.c ln.c ls.c mkdir.c mkfifo.c nl.c pwd.c rm.c sleep.c sort.c tail.c \
      tee.c touch.c true.c uname.c wc.c
OBJ = $(SRC:.c=.o) $(LIB)
BIN = $(SRC:.c=)
MAN = $(SRC:.c=.1)

all: $(BIN)

$(OBJ): util.h config.mk
$(BIN): util.a
cat.o grep.o tail.o: text.h

.o:
	@echo LD -o $@
	@$(LD) -o $@ $< util.a $(LDFLAGS)

.c.o:
	@echo CC -c $<
	@$(CC) -c -o $@ $< $(CFLAGS)

util.a: $(LIB)
	@echo AR -r $@
	@$(AR) -r -c $@ $(LIB)

dist: clean
	@echo creating dist tarball
	@mkdir -p sbase-$(VERSION)
	@cp -r LICENSE Makefile config.mk $(SRC) $(MAN) util $(HDR) sbase-$(VERSION)
	@tar -cf sbase-$(VERSION).tar sbase-$(VERSION)
	@gzip sbase-$(VERSION).tar
	@rm -rf sbase-$(VERSION)

clean:
	@echo cleaning
	@rm -f $(BIN) $(OBJ) $(LIB) util.a
