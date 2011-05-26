include config.mk

LIB = util/afgets.o util/agetcwd.o util/enmasse.o util/eprintf.o util/recurse.o
SRC = basename.c cat.c chown.c date.c dirname.c echo.c false.c grep.c head.c \
      ln.c ls.c mkfifo.c pwd.c rm.c sleep.c tee.c touch.c true.c wc.c
OBJ = $(SRC:.c=.o) $(LIB)
BIN = $(SRC:.c=)
MAN = $(SRC:.c=.1)

all: $(BIN)

$(OBJ): util.h
$(BIN): util.a
grep.o: text.h

.o:
	@echo CC -o $@
	@$(CC) -o $@ $< util.a $(LDFLAGS)

.c.o:
	@echo CC -c $<
	@$(CC) -c -o $@ $< $(CFLAGS)

util.a: $(LIB)
	@echo AR rc $@
	@$(AR) rc $@ $(LIB)

dist: clean
	@echo creating dist tarball
	@mkdir -p sbase-$(VERSION)
	@cp LICENSE Makefile config.mk $(SRC) $(MAN) util.c util.h sbase-$(VERSION)
	@tar -cf sbase-$(VERSION).tar sbase-$(VERSION)
	@gzip sbase-$(VERSION).tar
	@rm -rf sbase-$(VERSION)

clean:
	@echo cleaning
	@rm -f $(BIN) $(OBJ) $(LIB) util.a
