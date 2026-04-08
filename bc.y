%{
#include <libgen.h>
#include <unistd.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arg.h"
#include "util.h"

#define DIGITS   "0123456789ABCDEF"
#define NESTED_MAX 32

#define funid(f) ((f)[0] - 'a' + 1)

int yydebug;

typedef struct macro Macro;

struct macro {
	int op;
	int id;
	char *name;
	int flowid;
	int nested;
};

static int yyerror(char *);
static int yylex(void);

static void quit(void);
static char *code(char *, ...);
static char *forcode(Macro *, char *, char *, char *, char *);
static char *whilecode(Macro *, char *, char *);
static char *ifcode(Macro *, char *, char *);
static char *funcode(Macro *, char *, char *, char *);
static char *param(char *, char *), *local(char *, char *);
static Macro *define(char *, char *);
static char *retcode(char *);
static char *brkcode(void);
static Macro *macro(int);

static char *ftn(char *);
static char *var(char *);
static char *ary(char *);
static void writeout(char *);

static char *yytext, *buff, *unwind;
static char *filename;
static FILE *filep;
static int lineno, nerr, flowid;
static jmp_buf recover;
static int nested, inhome;
static Macro macros[NESTED_MAX];
int cflag, dflag, lflag, sflag;

static char *dcprog = "dc";

%}

%union {
	char *str;
	char id[2];
	Macro *macro;
}

%token <id> ID
%token <str> STRING NUMBER
%token <str> EQOP '+' '-' '*' '/' '%' '^' INCDEC
%token HOME LOOP
%token DOT
%token EQ
%token LE
%token GE
%token NE
%token DEF
%token BREAK
%token QUIT
%token LENGTH
%token RETURN
%token FOR
%token IF
%token WHILE
%token SQRT
%token SCALE
%token IBASE
%token OBASE
%token AUTO PARAM
%token PRINT

%type <str> item statlst scolonlst
%type <str> function assign nexpr expr exprstat rel stat ary cond
%type <str> autolst arglst parlst
%type <str> params param locals local
%type <macro> def if for while

%right	'=' EQOP
%left	'+' '-'
%left	'*' '/' '%'
%right	'^'

%start    program

%%

program  :
         | item program
         ;

item     : scolonlst '\n'       {writeout($1);}
         | function             {writeout($1);}
         ;

function : def parlst '{' '\n' autolst statlst '}' {$$ = funcode($1, $2, $5, $6);}
         ;

scolonlst:                      {$$ = code("");}
         | stat
         | scolonlst ';' stat   {$$ = code("%s%s", $1, $3);}
         | scolonlst ';'
         ;

statlst :                       {$$ = code("");}
        | stat
        | statlst '\n' stat     {$$ = code("%s%s", $1, $3);}
        | statlst ';' stat      {$$ = code("%s%s", $1, $3);}
        | statlst '\n'
        | statlst ';'
        ;

stat    : exprstat
        | PRINT expr            {$$ = code("%sps.", $2);}
        | PRINT STRING          {$$ = code("[%s]P", $2);}
        | PRINT STRING ',' expr {$$ = code("[%s]P%sps.", $2, $4);}
        | STRING                {$$ = code("[%s]P", $1);}
        | BREAK                 {$$ = brkcode();}
        | QUIT                  {quit();}
        | RETURN                {$$ = retcode(code(" 0"));}
        | RETURN '(' expr ')'   {$$ = retcode($3);}
        | RETURN '(' ')'        {$$ = retcode(code(" 0"));}
        | while cond stat       {$$ = whilecode($1, $2, $3);}
        | if cond stat          {$$ = ifcode($1, $2, $3);}
        | '{' statlst '}'       {$$ = $2;}
        | for '(' expr ';' rel ';' expr ')' stat  {$$ = forcode($1, $3, $5, $7, $9);}
        ;

while   : WHILE                 {$$ = macro(LOOP);}
        ;

if      : IF                    {$$ = macro(IF);}
        ;

for     : FOR                   {$$ = macro(LOOP);}
        ;

def     : DEF ID                {$$ = macro(DEF);}
        ;

parlst  : '(' ')'               {$$ = code("");}
        | '(' params ')'        {$$ = $2;}
        ;

params  : param                 {$$ = param(NULL, $1);}
        | params ',' param      {$$ = param($1, $3);}
        ;

param   : ID                    {$$ = var($1);}
        | ID '[' ']'            {$$ = ary($1);}
        ;

autolst :                       {$$ = code("");}
        | AUTO locals '\n'      {$$ = $2;}
        | AUTO locals ';'       {$$ = $2;}
        ;

locals  : local                 {$$ = local(NULL, $1);}
        | locals ',' local      {$$ = local($1, $3);}
        ;

local   : ID                    {$$ = var($1);}
        | ID '[' ']'            {$$ = ary($1);}
        ;

arglst  : expr
        | ID '[' ']'            {$$ = code("%s", ary($1));}
        | expr ',' arglst       {$$ = code("%s%s", $1, $3);}
        | ID '[' ']' ',' arglst {$$ = code("%s%s", ary($1), $5);}
        ;

cond    : '(' rel ')'           {$$ = $2;}
        ;

rel     : expr                  {$$ = code("%s 0!=", $1);}
        | expr EQ expr          {$$ = code("%s%s=", $1, $3);}
        | expr LE expr          {$$ = code("%s%s!<", $1, $3);}
        | expr GE expr          {$$ = code("%s%s!>", $1, $3);}
        | expr NE expr          {$$ = code("%s%s!=", $1, $3);}
        | expr '<' expr         {$$ = code("%s%s>", $1, $3);}
        | expr '>' expr         {$$ = code("%s%s<", $1, $3);}
        ;

exprstat: nexpr                 {$$ = code("%s%ss.", $1, code(sflag ? "" : "p"));}
        | assign                {$$ = code("%ss.", $1);}
        ;

expr    : nexpr
        | assign
        ;

nexpr   : NUMBER                {$$ = code(" %s", code($1));}
        | ID                    {$$ = code("l%s", var($1));}
        | DOT                   {$$ = code("l.");}
        | SCALE                 {$$ = code("K");}
        | IBASE                 {$$ = code("I");}
        | OBASE                 {$$ = code("O");}
        | ID ary                {$$ = code("%s;%s", $2, ary($1));}
        | '(' expr ')'          {$$ = $2;}
        | ID '(' arglst ')'     {$$ = code("%sl%sx", $3, ftn($1));}
        | ID '(' ')'            {$$ = code("l%sx", ftn($1));}
        | '-' expr              {$$ = code("0%s-", $2);}
        | expr '+' expr         {$$ = code("%s%s+", $1, $3);}
        | expr '-' expr         {$$ = code("%s%s-", $1, $3);}
        | expr '*' expr         {$$ = code("%s%s*", $1, $3);}
        | expr '/' expr         {$$ = code("%s%s/", $1, $3);}
        | expr '%' expr         {$$ = code("%s%s%%", $1, $3);}
        | expr '^' expr         {$$ = code("%s%s^", $1, $3);}
        | LENGTH '(' expr ')'   {$$ = code("%sZ", $3);}
        | SQRT '(' expr ')'     {$$ = code("%sv", $3);}
        | SCALE '(' expr ')'    {$$ = code("%sX", $3);}
        | INCDEC ID             {$$ = code("l%s1%sds%s", var($2), code($1), var($2));}
        | INCDEC SCALE          {$$ = code("K1%sk", code($1));}
        | INCDEC IBASE          {$$ = code("I1%sdi", code($1));}
        | INCDEC OBASE          {$$ = code("O1%sdo", code($1));}
        | INCDEC ID ary         {$$ = code("%sdS_;%s1%sdL_:%s", $3, ary($2), code($1), ary($2));}
        | ID INCDEC             {$$ = code("l%sd1%ss%s", var($1), code($2), var($1));}
        | SCALE INCDEC          {$$ = code("Kd1%sk", code($2));}
        | IBASE INCDEC          {$$ = code("Id1%si", code($2));}
        | OBASE INCDEC          {$$ = code("Od1%so", code($2));}
        | ID ary INCDEC         {$$ = code("%sds.;%sd1%sl.:%s", $2, ary($1), code($3), ary($1));}
        ;

assign  : ID '=' expr           {$$ = code("%sds%s", $3, var($1));}
        | SCALE '=' expr        {$$ = code("%sdk", $3);}
        | IBASE '=' expr        {$$ = code("%sdi", $3);}
        | OBASE '=' expr        {$$ = code("%sdo", $3);}
        | ID ary '=' expr       {$$ = code("%sd%s:%s", $4, $2, ary($1));}
        | ID EQOP expr          {$$ = code("%sl%s%sds%s", $3, var($1), code($2), var($1));}
        | SCALE EQOP expr       {$$ = code("%sK%sdk", $3, code($2));}
        | IBASE EQOP expr       {$$ = code("%sI%sdi", $3, code($2));}
        | OBASE EQOP expr       {$$ = code("%sO%sdo", $3, code($2));}
        | ID ary EQOP expr      {$$ = code("%s%sds.;%s%sdl.:s", $4, $2, ary($1), code($3), ary($1));}
        ;

ary     : '[' expr ']'          {$$ = $2;}
        ;

%%
static int
yyerror(char *s)
{
	fprintf(stderr, "bc: %s:%d: %s\n", filename, lineno, s);
	nerr++;
	longjmp(recover, 1);
}

static void
writeout(char *s)
{
	if (write(1, s, strlen(s)) < 0)
		goto err;
	if (write(1, "\n", 1) < 0)
		goto err;
	free(s);
	return;
	
err:
	eprintf("writing to dc:");
}

static char *
code(char *fmt, ...)
{
	char *s, *t;
	va_list ap;
	int c, len, room;

	va_start(ap, fmt);
	room = BUFSIZ;
	for (s = buff; *fmt; s += len) {
		len = 1;
		if ((c = *fmt++) != '%')
			goto append;

		switch (*fmt++) {
		case 'd':
			c = va_arg(ap, int);
			len = snprintf(s, room, "%d", c);
			if (len < 0 || len >= room)
				goto err;
			break;
		case 'c':
			c = va_arg(ap, int);
			goto append;
		case 's':
			t = va_arg(ap, void *);
			len = strlen(t);
			if (len >= room)
				goto err;
			memcpy(s, t, len);
			free(t);
			break;
		case '%':
		append:
			if (room <= 1)
				goto err;
			*s = c;
			break;
		default:
			abort();
		}

		room -= len;
	}
	va_end(ap);

	*s = '\0';
	return estrdup(buff);

err:
	eprintf("unable to code requested operation\n");
	return NULL;
}

static Macro *
macro(int op)
{
	int preop;
	Macro *d, *p;

	if (nested == NESTED_MAX)
		yyerror("too much nesting");

	d = &macros[nested];
	d->op = op;
	d->nested = nested++;
	d->name = NULL;

	switch (op) {
	case HOME:
		d->id = 0;
		d->flowid = flowid;
		inhome = 1;
		break;
	case DEF:
		unwind = estrdup("");
		inhome = 0;
		d->id = funid(yytext);
		d->name = estrdup(yytext);
		d->flowid = macros[0].flowid;
		break;
	default:
		assert(nested > 1);
		preop = d[-1].op;
		d->flowid = d[-1].flowid;
		if (preop != HOME && preop != DEF) {
			if (d->flowid == 255)
				eprintf("too many control flow structures");
			d->flowid++;
		}
		d->id = d->flowid;
		if (!inhome) {
			/* populate reserved id */
			flowid = d->flowid;
			for (p = d; p != macros; --p)
				p[-1].flowid++;
		}
		break;
	}

	return d;
}

static char *
decl(int type, char *list, char *id)
{
	char *i1, *i2;

	i1 = estrdup(id);
	i2 = estrdup(id);
	free(id);

	if (!list)
		list = estrdup("");

	unwind = code("%sL%ss.", unwind, i1);

	return code((type == AUTO) ? "0S%s%s" : "S%s%s", i2, list);
}

static char *
param(char *list, char *id)
{
	return decl(PARAM, list, id);
}

static char *
local(char *list, char *id)
{
	return decl(AUTO, list, id);
}

static char *
funcode(Macro *d, char *params, char *vars, char *body)
{
	char *s;

	if (strlen(d->name) > 1) {
		s = code("[%s%s%s%s]s\"()%s\"",
			 vars, params,
			 body,
			 retcode(code(" 0")),
			 d->name);
	} else {
		s = code(sflag ? "[%s%s%s%s]s<%d>" : "[%s%s%s%s]s%c",
			 vars, params,
			 body,
			 retcode(code(" 0")),
			 d->id);
		free(d->name);
	}

	free(unwind);
	unwind = NULL;
	nested--;
	inhome = 0;

	return s;
}

static char *
brkcode(void)
{
	Macro *d;

	for (d = &macros[nested-1]; d->op != HOME && d->op != LOOP; --d)
		;
	if (d->op == HOME)
		yyerror("break not in for or while");
	return code(" %dQ", nested  - d->nested);
}

static char *
forcode(Macro *d, char *init, char *cmp, char *inc, char *body)
{
	char *s;

	s = code(sflag ? "[%s%ss.%s<%d>]s<%d>" : "[%s%ss.%s%c]s%c",
	         body,
	         inc,
	         estrdup(cmp),
	         d->id, d->id);
	writeout(s);

	s = code(sflag ? "%ss.%s<%d> " : "%ss.%s%c ",
	         init,
	         cmp,
	         d->id);
	nested--;

	return s;
}

static char *
whilecode(Macro *d, char *cmp, char *body)
{
	char *s;

	s = code(sflag ? "[%s%s<%d>]s<%d>" : "[%s%s%c]s%c",
	         body,
	         estrdup(cmp),
	         d->id, d->id);
	writeout(s);

	s = code(sflag ? "%s<%d> " : "%s%c ",
	         cmp, d->id);
	nested--;

	return s;
}

static char *
ifcode(Macro *d, char *cmp, char *body)
{
	char *s;

	s = code(sflag ? "[%s]s<%d>" : "[%s]s%c",
	         body, d->id);
	writeout(s);

	s = code(sflag ? "%s<%d> " : "%s%c ",
	         cmp, d->id);
	nested--;

	return s;
}

static char *
retcode(char *expr)
{
	char *s;

	if (nested < 2 || macros[1].op != DEF)
		yyerror("return must be in a function");
	return code("%s %s %dQ", expr, estrdup(unwind), nested - 1);
}

static char *
ary(char *s)
{
	if (strlen(s) == 1)
		return code("%c", toupper(s[0]));
	return code("\"[]%s\"", estrdup(s));
}

static char *
ftn(char *s)
{
	if (strlen(s) == 1)
		return code(sflag ? "<%d>" : "%c", funid(s));
	return code("\"()%s\"", estrdup(s));
}

static char *
var(char *s)
{
	if (strlen(s) == 1)
		return code(s);
	return code("\"%s\"", estrdup(s));
}

static void
quit(void)
{
	exit(nerr > 0 ? 1 : 0);
}

static void
skipspaces(void)
{
	int ch;

	while (isascii(ch = getc(filep)) && isspace(ch)) {
		if (ch == '\n') {
			lineno++;
			break;
		}
	}
	ungetc(ch, filep);
}

static int
iden(int ch)
{
	static struct keyword {
		char *str;
		int token;
	} keywords[] = {
		{"define", DEF},
		{"break", BREAK},
		{"quit", QUIT},
		{"length", LENGTH},
		{"return", RETURN},
		{"for", FOR},
		{"if", IF},
		{"while", WHILE},
		{"sqrt", SQRT},
		{"scale", SCALE},
		{"ibase", IBASE},
		{"obase", OBASE},
		{"auto", AUTO},
		{"print", PRINT},
		{NULL}
	};
	struct keyword *p;
	char *bp;

	ungetc(ch, filep);
	for (bp = yytext; bp < &yytext[BUFSIZ]; ++bp) {
		ch = getc(filep);
		if (!isascii(ch) || !islower(ch))
			break;
		*bp = ch;
	}

	if (bp == &yytext[BUFSIZ])
		yyerror("too long token");
	*bp = '\0';
	ungetc(ch, filep);

	if (strlen(yytext) == 1) {
		strcpy(yylval.id, yytext);
		return ID;
	}

	for (p = keywords; p->str && strcmp(p->str, yytext); ++p)
		;
	if (p->str)
		return p->token;

	if (!sflag)
		yyerror("invalid keyword");
	strcpy(yylval.id, yytext);
	return ID;
}

static char *
digits(char *bp)
{
	int ch;
	char *digits = DIGITS, *p;

	while (bp < &yytext[BUFSIZ]) {
		ch = getc(filep);
		p = strchr(digits, ch);
		if (!p)
			break;
		*bp++ = ch;
	}

	if (bp == &yytext[BUFSIZ])
		return NULL;
	ungetc(ch, filep);

	return bp;
}

static int
number(int ch)
{
	int d;
	char *bp;

	ungetc(ch, filep);
	if ((bp = digits(yytext)) == NULL)
		goto toolong;

	if ((ch = getc(filep)) != '.') {
		ungetc(ch, filep);
		goto end;
	}
	*bp++ = '.';

	if ((bp = digits(bp)) == NULL)
		goto toolong;

end:
	if (bp ==  &yytext[BUFSIZ])
		goto toolong;
	*bp = '\0';
	yylval.str = yytext;

	return NUMBER;

toolong:
	yyerror("too long number");
	return 0;
}

static int
string(int ch)
{
	char *bp;

	for (bp = yytext; bp < &yytext[BUFSIZ]; ++bp) {
		if ((ch = getc(filep)) == '"')
			break;
		*bp = ch;
	}

	if (bp == &yytext[BUFSIZ])
		yyerror("too long string");
	*bp = '\0';
	yylval.str = estrdup(yytext);

	return STRING;
}

static int
follow(int next, int yes, int no)
{
	int ch;

	ch = getc(filep);
	if (ch == next)
		return yes;
	ungetc(ch, filep);
	return no;
}

static int
operand(int ch)
{
	int peekc;

	switch (ch) {
	case '\n':
	case '{':
	case '}':
	case '[':
	case ']':
	case '(':
	case ')':
	case ',':
	case ';':
		return ch;
	case '.':
		peekc = ungetc(getc(filep), filep);
		if (strchr(DIGITS, peekc))
			return number(ch);
		return DOT;
	case '"':
		return string(ch);
	case '*':
		yylval.str = "*";
		return follow('=', EQOP, '*');
	case '/':
		yylval.str = "/";
		return follow('=', EQOP, '/');
	case '%':
		yylval.str = "%";
		return follow('=', EQOP, '%');
	case '=':
		return follow('=', EQ, '=');
	case '+':
	case '-':
		yylval.str = (ch == '+') ? "+" : "-";
		if (follow('=', EQOP, ch) != ch)
			return EQOP;
		return follow(ch, INCDEC, ch);
	case '^':
		yylval.str = "^";
		return follow('=', EQOP, '^');
	case '<':
		return follow('=', LE, '<');
	case '>':
		return follow('=', GE, '>');
	case '!':
		if (getc(filep) == '=')
			return NE;
	default:
		yyerror("invalid operand");
		return 0;
	}
}

static void
comment(void)
{
	int c;

	for (;;) {
		while ((c = getc(filep)) != '*') {
			if (c == '\n')
				lineno++;
		}
		if ((c = getc(filep)) == '/')
			break;
		ungetc(c, filep);
	}
}

static int
yylex(void)
{
	int peekc, ch;

repeat:
	skipspaces();

	ch = getc(filep);
	if (ch == EOF) {
		return EOF;
	} else if (!isascii(ch)) {
		yyerror("invalid input character");
	} else if (islower(ch)) {
		return iden(ch);
	} else if (strchr(DIGITS, ch)) {
		return number(ch);
	} else {
		if (ch == '/') {
			peekc = getc(filep);
			if (peekc == '*') {
				comment();
				goto repeat;
			}
			ungetc(peekc, filep);
		}
		return operand(ch);
	}

	return 0;
}

static void
spawn(void)
{
	int fds[2];
	char *par = sflag ? "-i" : NULL;
	char errmsg[] = "bc:error execing dc\n";

	if (pipe(fds) < 0)
		eprintf("creating pipe:");

	switch (fork()) {
	case -1:
		eprintf("forking dc:");
	case 0:
		close(1);
		dup(fds[1]);
		close(fds[0]);
		close(fds[1]);
		break;
	default:
		close(0);
		dup(fds[0]);
		close(fds[0]);
		close(fds[1]);
		execlp(dcprog, "dc", par, (char *) NULL);

		/* it shouldn't happen */
		write(3, errmsg, sizeof(errmsg)-1);
		_Exit(2);
	}
}

static void
run(void)
{
	if (setjmp(recover)) {
		if (ferror(filep))
			eprintf("%s:", filename);
		if (feof(filep))
			return;
	}
	yyparse();
}

static void
bc(char *fname)
{
	Macro *d;

	lineno = 1;
	nested = 0;

	macro(HOME);
	if (!fname) {
		filename = "<stdin>";
		filep = stdin;
	} else {
		filename = fname;
		if ((filep = fopen(fname, "r")) == NULL)
			eprintf("%s:", fname);
	}

	run();
	fclose(filep);
}

static void
usage(void)
{
	eprintf("usage: %s [-p dc][-cdls]\n", argv0);
}

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	case 'p':
		dcprog = EARGF(usage());
		break;
	case 'c':
		cflag = 1;
		break;
	case 'd':
		dflag = 1;
		yydebug = 3;
		break;
	case 'l':
		lflag = 1;
		break;
	case 's':
		sflag = 1;
		break;
	default:
		usage();
	} ARGEND

	yytext = malloc(BUFSIZ);
	buff = malloc(BUFSIZ);
	if (!yytext || !buff)
		eprintf("out of memory\n");
	flowid = 128;

	if (!cflag)
		spawn();
	if (lflag)
		bc(PREFIX "/share/misc/bc.library");

	while (*argv)
		bc(*argv++);
	bc(NULL);

	quit();
}
