include config.mk

SRC = basename.c cat.c date.c echo.c false.c grep.c pwd.c sleep.c tee.c touch.c true.c wc.c
OBJ = $(SRC:.c=.o) util.o
BIN = $(SRC:.c=)
MAN = $(SRC:.c=.1)

all: $(BIN)

$(OBJ): util.h
$(BIN): util.o

.o:
	@echo CC -o $@
	@$(CC) -o $@ $< util.o $(LDFLAGS)

.c.o:
	@echo CC -c $<
	@$(CC) -c $< $(CFLAGS)

dist: clean
	@echo creating dist tarball
	@mkdir -p sbase-$(VERSION)
	@cp LICENSE Makefile config.mk $(SRC) $(MAN) util.c util.h sbase-$(VERSION)
	@tar -cf sbase-$(VERSION).tar sbase-$(VERSION)
	@gzip sbase-$(VERSION).tar
	@rm -rf sbase-$(VERSION)

clean:
	@echo cleaning
	@rm -f $(BIN) $(OBJ)
