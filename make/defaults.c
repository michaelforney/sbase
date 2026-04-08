char defaults[] = 
	".SUFFIXES: .o .c .y .l .a .sh .f\n"

	"AR = ar\n"
	"ARFLAGS = -rv\n"
	"CC = c99\n"
	"CFLAGS = -O\n"
	"FC = fort77\n"
	"FFLAGS = -O 1\n"
	"LDFLAGS =\n"
	"LEX = lex\n"
	"LFLAGS =\n"
	"YACC = yacc\n"
	"YFLAGS =\n"
	"SHELL = /bin/sh\n"

	".c:\n"
	"\t${CC} ${CFLAGS} ${LDFLAGS} -o $@ $<\n"

	".f:\n"
	"\t${FC} ${FFLAGS} ${LDFLAGS} -o $@ $<\n"

	".sh:\n"
	"\tcp $< $@\n"
	"\tchmod a+x $@\n"

	".c.o:\n"
	"\t${CC} ${CFLAGS} -c $<\n"

	".f.o:\n"
	"\t${FC} ${FFLAGS} -c $<\n"

	".y.o:\n"
	"\t${YACC} ${YFLAGS} $<\n"
	"\t${CC} ${CFLAGS} -c y.tab.c\n"
	"\trm -f y.tab.c\n"
	"\tmv y.tab.o $@\n"

	".l.o:\n"
	"\t${LEX} ${LFLAGS} $<\n"
	"\t${CC} ${CFLAGS} -c lex.yy.c\n"
	"\trm -f lex.yy.c\n"
	"\tmv lex.yy.o $@\n"

	".y.c:\n"
	"\t${YACC} ${YFLAGS} $<\n"
	"\tmv y.tab.c $@\n"

	".l.c:\n"
	"\t${LEX} ${LFLAGS} $<\n"
	"\tmv lex.yy.c $@\n"

	".c.a:\n"
	"\t${CC} -c ${CFLAGS} $<\n"
	"\t${AR} ${ARFLAGS} $@ $*.o\n"
	"\trm -f $*.o\n"

	".f.a:\n"
	"\t${FC} -c ${FFLAGS} $<\n"
	"\t${AR} ${ARFLAGS} $@ $*.o\n"
	"\trm -f $*.o\n";
