/* See LICENSE file for copyright and license details. */
#include <dirent.h>
#include <fnmatch.h>
#include <grp.h>
#include <libgen.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/wait.h>

#include "util.h"

/* because putting integers in pointers is undefined by the standard */
typedef union {
	void    *p;
	intptr_t i;
} Extra;

/* Argument passed into a primary's function */
typedef struct {
	char        *path;
	struct stat *st;
	Extra        extra;
} Arg;

/* Information about each primary, for lookup table */
typedef struct {
	char  *name;
	int    (*func)(Arg *arg);
	char **(*getarg)(char **argv, Extra *extra);
	void   (*freearg)(Extra extra);
} Pri_info;

/* Information about operators, for lookup table */
typedef struct {
	char *name;   /* string representation of op           */
	char  type;   /* from Tok.type                         */
	char  prec;   /* precedence                            */
	char  nargs;  /* number of arguments (unary or binary) */
	char  lassoc; /* left associative                      */
} Op_info;

/* Token when lexing/parsing
 * (although also used for the expression tree) */
typedef struct Tok Tok;
struct Tok {
	Tok  *left, *right; /* if (type == NOT) left = NULL */
	Extra extra;
	union {
		Pri_info *pinfo; /* if (type == PRIM) */
		Op_info  *oinfo;
	} u;
	enum {
		PRIM = 0, LPAR, RPAR, NOT, AND, OR, END
	} type;
};

/* structures used for Arg.extra.p and Tok.extra.p */
typedef struct {
	mode_t mode;
	char   exact;
} Permarg;

typedef struct {
	char ***braces;
	char **argv;
} Okarg;

/* for all arguments that take a number
 * +n, n, -n mean > n, == n, < n respectively */
typedef struct {
	enum {
		GT, EQ, LT
	} cmp;
	int n;
} Narg;

typedef struct {
	Narg n;
	char bytes; /* size is in bytes, not 512 byte sectors */
} Sizearg;

typedef struct {
	union {
		struct {
			char ***braces; /* NULL terminated list of pointers into argv where {} were */
		} s; /* semicolon */
		struct {
			size_t arglen;  /* number of bytes in argv before files are added */
			size_t filelen; /* numer of bytes in file names added to argv     */
			size_t first;   /* index one past last arg, where first file goes */
			size_t next;    /* index where next file goes                     */
			size_t cap;     /* capacity of argv                               */
		} p; /* plus */
	} u;
	char **argv; /* NULL terminated list of arguments (allocated if isplus) */
	char   isplus; /* -exec + instead of -exec ; */
} Execarg;

/* used to find loops while recursing through directory structure */
typedef struct Findhist Findhist;
struct Findhist {
	Findhist *next;
	char     *path;
	dev_t     dev;
	ino_t     ino;
};

/* Primaries */
static int pri_name   (Arg *arg);
static int pri_path   (Arg *arg);
static int pri_nouser (Arg *arg);
static int pri_nogroup(Arg *arg);
static int pri_xdev   (Arg *arg);
static int pri_prune  (Arg *arg);
static int pri_perm   (Arg *arg);
static int pri_type   (Arg *arg);
static int pri_links  (Arg *arg);
static int pri_user   (Arg *arg);
static int pri_group  (Arg *arg);
static int pri_size   (Arg *arg);
static int pri_atime  (Arg *arg);
static int pri_ctime  (Arg *arg);
static int pri_mtime  (Arg *arg);
static int pri_exec   (Arg *arg);
static int pri_ok     (Arg *arg);
static int pri_print  (Arg *arg);
static int pri_newer  (Arg *arg);
static int pri_depth  (Arg *arg);

/* Getargs */
static char **get_name_arg (char **argv, Extra *extra);
static char **get_path_arg (char **argv, Extra *extra);
static char **get_perm_arg (char **argv, Extra *extra);
static char **get_type_arg (char **argv, Extra *extra);
static char **get_n_arg    (char **argv, Extra *extra);
static char **get_user_arg (char **argv, Extra *extra);
static char **get_group_arg(char **argv, Extra *extra);
static char **get_size_arg (char **argv, Extra *extra);
static char **get_exec_arg (char **argv, Extra *extra);
static char **get_ok_arg   (char **argv, Extra *extra);
static char **get_newer_arg(char **argv, Extra *extra);

/* Freeargs */
static void free_extra   (Extra extra);
static void free_exec_arg(Extra extra);
static void free_ok_arg  (Extra extra);

/* Parsing/Building/Running */
static Pri_info *find_primary(char *name);
static Op_info *find_op(char *name);
static void parse(int argc, char **argv);
static int eval(Tok *tok, Arg *arg);
static void find(char *path, Findhist *hist);
static void usage(void);

/* for comparisons with Narg */
static int cmp_gt(int a, int b) { return a >  b; }
static int cmp_eq(int a, int b) { return a == b; }
static int cmp_lt(int a, int b) { return a <  b; }

static int (*cmps[])(int, int) = {
	[GT] = cmp_gt,
	[EQ] = cmp_eq,
	[LT] = cmp_lt,
};

/* order from find(1p), may want to alphabetize */
static Pri_info primaries[] = {
	{ "-name"   , pri_name   , get_name_arg , NULL          },
	{ "-path"   , pri_path   , get_path_arg , NULL          },
	{ "-nouser" , pri_nouser , NULL         , NULL          },
	{ "-nogroup", pri_nogroup, NULL         , NULL          },
	{ "-xdev"   , pri_xdev   , NULL         , NULL          },
	{ "-prune"  , pri_prune  , NULL         , NULL          },
	{ "-perm"   , pri_perm   , get_perm_arg , free_extra    },
	{ "-type"   , pri_type   , get_type_arg , NULL          },
	{ "-links"  , pri_links  , get_n_arg    , free_extra    },
	{ "-user"   , pri_user   , get_user_arg , NULL          },
	{ "-group"  , pri_group  , get_group_arg, NULL          },
	{ "-size"   , pri_size   , get_size_arg , free_extra    },
	{ "-atime"  , pri_atime  , get_n_arg    , free_extra    },
	{ "-ctime"  , pri_ctime  , get_n_arg    , free_extra    },
	{ "-mtime"  , pri_mtime  , get_n_arg    , free_extra    },
	{ "-exec"   , pri_exec   , get_exec_arg , free_exec_arg },
	{ "-ok"     , pri_ok     , get_ok_arg   , free_ok_arg   },
	{ "-print"  , pri_print  , NULL         , NULL          },
	{ "-newer"  , pri_newer  , get_newer_arg, NULL          },
	{ "-depth"  , pri_depth  , NULL         , NULL          },

	{ NULL, NULL, NULL, NULL }
};

static Op_info ops[] = {
	{ "(" , LPAR, 0, 0, 0 }, /* parens are handled specially */
	{ ")" , RPAR, 0, 0, 0 },
	{ "!" , NOT , 3, 1, 0 },
	{ "-a", AND , 2, 2, 1 },
	{ "-o", OR  , 1, 2, 1 },

	{ NULL, 0, 0, 0, 0 }
};

extern char **environ;

static Tok *toks; /* holds allocated array of all Toks created while parsing */
static Tok *root; /* points to root of expression tree, inside toks array */

static struct timespec start; /* time find was started, used for -[acm]time */

static size_t envlen; /* number of bytes in environ, used to calculate against ARG_MAX */
static size_t argmax; /* value of ARG_MAX retrieved using sysconf(3p) */

static struct {
	char ret  ; /* return value from main                             */
	char depth; /* -depth, directory contents before directory itself */
	char h    ; /* -H, follow symlinks on command line                */
	char l    ; /* -L, follow all symlinks (command line and search)  */
	char prune; /* hit -prune                                         */
	char xdev ; /* -xdev, prune directories on different devices      */
} gflags;

/*
 * Primaries
 */
static int
pri_name(Arg *arg)
{
	return !fnmatch((char *)arg->extra.p, basename(arg->path), 0);
}

static int
pri_path(Arg *arg)
{
	return !fnmatch((char *)arg->extra.p, arg->path, 0);
}

/* FIXME: what about errors? find(1p) literally just says
 * "for which the getpwuid() function ... returns NULL" */
static int
pri_nouser(Arg *arg)
{
	return !getpwuid(arg->st->st_uid);
}

static int
pri_nogroup(Arg *arg)
{
	return !getgrgid(arg->st->st_gid);
}

static int
pri_xdev(Arg *arg)
{
	return 1;
}

static int
pri_prune(Arg *arg)
{
	return gflags.prune = 1;
}

static int
pri_perm(Arg *arg)
{
	Permarg *p = (Permarg *)arg->extra.p;

	return (arg->st->st_mode & 07777 & (p->exact ? -1U : p->mode)) == p->mode;
}

static int
pri_type(Arg *arg)
{
	switch ((char)arg->extra.i) {
	default : return 0; /* impossible, but placate warnings */
	case 'b': return S_ISBLK (arg->st->st_mode);
	case 'c': return S_ISCHR (arg->st->st_mode);
	case 'd': return S_ISDIR (arg->st->st_mode);
	case 'l': return S_ISLNK (arg->st->st_mode);
	case 'p': return S_ISFIFO(arg->st->st_mode);
	case 'f': return S_ISREG (arg->st->st_mode);
	case 's': return S_ISSOCK(arg->st->st_mode);
	}
}

static int
pri_links(Arg *arg)
{
	Narg *n = arg->extra.p;
	return cmps[n->cmp](arg->st->st_nlink, n->n);
}

static int
pri_user(Arg *arg)
{
	return arg->st->st_uid == (uid_t)arg->extra.i;
}

static int
pri_group(Arg *arg)
{
	return arg->st->st_gid == (gid_t)arg->extra.i;
}

static int
pri_size(Arg *arg)
{
	Sizearg *s = arg->extra.p;
	off_t size = arg->st->st_size;

	if (!s->bytes)
		size = size / 512 + !!(size % 512);

	return cmps[s->n.cmp](size, s->n.n);
}

/* FIXME: ignoring nanoseconds in atime, ctime, mtime */
static int
pri_atime(Arg *arg)
{
	Narg *n = arg->extra.p;
	time_t time = (n->n - start.tv_sec) / 86400;
	return cmps[n->cmp](time, n->n);
}

static int
pri_ctime(Arg *arg)
{
	Narg *n = arg->extra.p;
	time_t time = (n->n - start.tv_sec) / 86400;
	return cmps[n->cmp](time, n->n);
}

static int
pri_mtime(Arg *arg)
{
	Narg *n = arg->extra.p;
	time_t time = (n->n - start.tv_sec) / 86400;
	return cmps[n->cmp](time, n->n);
}

static int
pri_exec(Arg *arg)
{
	Execarg *e = arg->extra.p;

	if (e->isplus) {
		size_t len = strlen(arg->path) + 1;

		/* if we've reached ARG_MAX, fork, exec, wait, free file names, reset
		 * list */
		if (len + e->u.p.arglen + e->u.p.filelen + envlen > argmax) {
			char **arg;
			int status;
			pid_t pid;

			e->argv[e->u.p.next] = NULL;

			if (!(pid = fork())) { /* child */
				execvp(*e->argv, e->argv);
				eprintf("exec %s failed:", *e->argv);
			}
			waitpid(pid, &status, 0);
			gflags.ret |= status;

			for (arg = e->argv + e->u.p.first; *arg; arg++)
				free(*arg);

			e->u.p.next = e->u.p.first;
			e->u.p.filelen = 0;
		}

		/* if we have too many filenames, realloc (with space for NULL
		 * termination) */
		if (e->u.p.next + 1 == e->u.p.cap)
			e->argv = erealloc(e->argv, (e->u.p.cap *= 2) * sizeof(*e->argv));

		/* FIXME: we can do better than strdup and free for every single file
		 *        name. do we care? use strlacat/strlacpy from sed? */
		e->argv[e->u.p.next++] = estrdup(arg->path);
		e->u.p.filelen += len + sizeof(arg->path);

		return 1;
	} else {
		int status;
		char ***brace;
		pid_t pid;

		/* insert path everywhere user gave us {} */
		for (brace = e->u.s.braces; *brace; brace++)
			**brace = arg->path;

		if (!(pid = fork())) { /* child */
			execvp(*e->argv, e->argv);
			eprintf("exec %s failed:", *e->argv);
		}
		/* FIXME: propper course of action for all waitpid() on EINTR? */
		waitpid(pid, &status, 0);
		return !!status;
	}
}

static int
pri_ok(Arg *arg)
{
	int status;
	char ***brace, reply, buf[256];
	pid_t pid;
	Okarg *o = arg->extra.p;

	fprintf(stderr, "%s: %s ?", *o->argv, arg->path);
	reply = fgetc(stdin);

	/* throw away rest of line */
	while (fgets(buf, sizeof(buf), stdin) && *buf && buf[strlen(buf) - 1] == '\n')
		/* FIXME: what if the first character of the rest of the line is a null
		 * byte? probably shouldn't juse fgets() */
		;

	if (feof(stdin)) /* ferror()? */
		clearerr(stdin);

	if (reply != 'y' && reply != 'Y')
		return 0;

	/* insert filename everywhere use gave us {} */
	for (brace = o->braces; *brace; brace++)
		**brace = arg->path;

	if (!(pid = fork())) { /* child */
		execvp(*o->argv, o->argv);
		eprintf("exec %s failed:", *o->argv);
	}
	waitpid(pid, &status, 0);
	return !!status;
}

static int
pri_print(Arg *arg)
{
	puts(arg->path);
	return 1;
}

/* FIXME: ignoring nanoseconds */
static int
pri_newer(Arg *arg)
{
	return arg->st->st_mtime > (time_t)arg->extra.i;
}

static int
pri_depth(Arg *arg)
{
	return 1;
}

/*
 * Getargs
 * consume any arguments for given primary and fill extra
 * return pointer to last argument, the pointer will be incremented in parse()
 */
static char **
get_name_arg(char **argv, Extra *extra)
{
	extra->p = *argv;
	return argv;
}

static char **
get_path_arg(char **argv, Extra *extra)
{
	extra->p = *argv;
	return argv;
}

static char **
get_perm_arg(char **argv, Extra *extra)
{
	Permarg *p = emalloc(sizeof(*p));

	if (**argv == '-')
		(*argv)++;
	else
		p->exact = 1;

	p->mode = parsemode(*argv, 0, 0);
	extra->p = p;

	return argv;
}

static char **
get_type_arg(char **argv, Extra *extra)
{
	if (!strchr("bcdlpfs", **argv))
		eprintf("invalid type %c for -type primary\n", **argv);

	extra->i = **argv;
	return argv;
}

static char **
get_n_arg(char **argv, Extra *extra)
{
	Narg *n = emalloc(sizeof(*n));
	char *end;

	switch (**argv) {
	case '+': n->cmp = GT; (*argv)++; break;
	case '-': n->cmp = LT; (*argv)++; break;
	default : n->cmp = EQ;            break;
	}

	n->n = strtol(*argv, &end, 10);
	if (end == *argv || *end)
		eprintf("bad number '%s'\n", *argv);

	extra->p = n;
	return argv;
}

static char **
get_user_arg(char **argv, Extra *extra)
{
	char *end;
	struct passwd *p = getpwnam(*argv);

	if (p) {
		extra->i = p->pw_uid;
	} else {
		extra->i = strtol(*argv, &end, 10);
		if (end == *argv || *end)
			eprintf("unknown user '%s'\n", *argv);
	}
	return argv;
}

static char **
get_group_arg(char **argv, Extra *extra)
{
	char *end;
	struct group *g = getgrnam(*argv);

	if (g) {
		extra->i = g->gr_gid;
	} else {
		extra->i = strtol(*argv, &end, 10);
		if (end == *argv || *end)
			eprintf("unknown group '%s'\n", *argv);
	}
	return argv;
}

static char **
get_size_arg(char **argv, Extra *extra)
{
	char *end;
	char *p = *argv + strlen(*argv);
	Sizearg *s = emalloc(sizeof(*s));
	/* if the number is followed by 'c', the size will by in bytes */
	s->bytes = p > *argv && *--p == 'c';

	if (s->bytes)
		*p = '\0';

	/* FIXME: no need to have this in get_n_arg and here */
	switch (**argv) {
	case '+': s->n.cmp = GT; (*argv)++; break;
	case '-': s->n.cmp = LT; (*argv)++; break;
	default : s->n.cmp = EQ;            break;
	}

	s->n.n = strtol(*argv, &end, 10);
	if (end == *argv || *end)
		eprintf("bad number '%s'\n", *argv);

	extra->p = s;
	return argv;
}

static char **
get_exec_arg(char **argv, Extra *extra)
{
	char **arg;
	int nbraces = 0;
	Execarg *e = emalloc(sizeof(*e));

	for (arg = argv; *arg; arg++)
		if (!strcmp(*arg, ";"))
			break;
		else if (arg > argv && !strcmp(*(arg - 1), "{}") && !strcmp(*arg, "+"))
			break;
		else if (!strcmp(*arg, "{}"))
			nbraces++;

	if (!*arg)
		eprintf("no terminating ; or {} + for -exec primary\n");

	e->isplus = **arg == '+';
	*arg = NULL;

	if (e->isplus) {
		char **new;

		*(arg - 1) = NULL; /* don't need the {} in there now */
		e->u.p.arglen = e->u.p.filelen = 0;
		e->u.p.first = e->u.p.next = arg - argv - 1;
		e->u.p.cap = (arg - argv) * 2;
		e->argv = emalloc(e->u.p.cap * sizeof(*e->argv));

		for (arg = argv, new = e->argv; *arg; arg++, new++) {
			*new = *arg;
			e->u.p.arglen += strlen(*arg) + 1 + sizeof(*arg);
		}
		arg++; /* due to our extra NULL */
	} else {
		char ***braces;

		e->argv = argv;
		e->u.s.braces = emalloc(++nbraces * sizeof(*e->u.s.braces)); /* ++ for NULL */

		for (arg = argv, braces = e->u.s.braces; *arg; arg++)
			if (!strcmp(*arg, "{}"))
				*braces++ = arg;
	}
	extra->p = e;
	return arg;
}

static char **
get_ok_arg(char **argv, Extra *extra)
{
	char **arg, ***braces;
	int nbraces = 0;
	Okarg *o = emalloc(sizeof(*o));

	for (arg = argv; *arg; arg++)
		if (!strcmp(*arg, ";"))
			break;
		else if (!strcmp(*arg, "{}"))
			nbraces++;

	if (!*arg)
		eprintf("no terminating ; for -ok primary\n");
	*arg = NULL;

	o->argv = argv;
	o->braces = emalloc(++nbraces * sizeof(*o->braces));

	for (arg = argv, braces = o->braces; *arg; arg++)
		if (!strcmp(*arg, "{}"))
			*braces++ = arg;

	extra->p = o;
	return arg;
}

/* FIXME: ignoring nanoseconds */
static char **
get_newer_arg(char **argv, Extra *extra)
{
	struct stat st;

	if (stat(*argv, &st))
		eprintf("failed to stat '%s':", *argv);

	extra->i = st.st_mtime;
	return argv;
}

/*
 * Freeargs
 */
static void
free_extra(Extra extra)
{
	free(extra.p);
}

static void
free_exec_arg(Extra extra)
{
	Execarg *e = extra.p;
	if (!e->isplus) {
		free(e->u.s.braces);
	} else {
		char **arg;

		e->argv[e->u.p.next] = NULL;

		/* if we have files, do the last exec */
		if (e->u.p.first != e->u.p.next) {
			int status;
			pid_t pid = fork();

			if (!pid) { /* child */
				execvp(*e->argv, e->argv);
				eprintf("exec %s failed:", *e->argv);
			}
			waitpid(pid, &status, 0);
			gflags.ret |= status;
		}
		for (arg = e->argv + e->u.p.first; *arg; arg++)
			free(*arg);
		free(e->argv);
	}
	free(e);
}

static void
free_ok_arg(Extra extra)
{
	Okarg *o = extra.p;
	free(o->braces);
	free(o);
}

/*
 * Parsing/Building/Running
 */
static Pri_info *
find_primary(char *name)
{
	Pri_info *p;

	for (p = primaries; p->name; p++)
		if (!strcmp(name, p->name))
			return p;
	return NULL;
}

static Op_info *
find_op(char *name)
{
	Op_info *o;

	for (o = ops; o->name; o++)
		if (!strcmp(name, o->name))
			return o;
	return NULL;
}

/* given the expression from the command line
 * 1) convert arguments from strings to Tok and place in an array duplicating
 *    the infix expression given, inserting "-a" where it was omitted
 * 2) allocate an array to hold the correct number of Tok, and convert from
 *    infix to rpn (using shunting-yard), add -a and -print if necessary
 * 3) evaluate the rpn filling in left and right pointers to create an
 *    expression tree (Tok are still all contained in the rpn array, just
 *    pointing at eachother)
 */
static void
parse(int argc, char **argv)
{
	int lasttype = -1;
	Tok infix[2 * argc], *stack[argc], *tok, *rpn, *out, **top;
	char **arg;
	size_t ntok = 0;
	int print = 1;
	Tok and = { .u.oinfo = find_op("-a"), .type = AND };

	/* convert argv to infix expression of Tok, inserting in *tok */
	for (arg = argv, tok = infix; *arg; arg++, tok++) {
		Op_info *op;
		Pri_info *pri = find_primary(*arg);

		if (pri) { /* token is a primary, fill out Tok and get arguments */

			/* FIXME: should never have to ask "which primary is this?" Should
			 * probably move this into getarg even though there's no arg */
			if (pri->func == pri_depth)
				gflags.depth = 1;
			else if (pri->func == pri_xdev)
				gflags.xdev = 1;
			else if (pri->func == pri_exec || pri->func == pri_ok || pri->func == pri_print)
				print = 0;

			if (lasttype == PRIM || lasttype == RPAR) {
				*tok++ = and;
				ntok++;
			}
			if (pri->getarg) {
				if (!*++arg)
					eprintf("no argument for primary %s\n", pri->name);
				arg = pri->getarg(arg, &tok->extra);
			}
			tok->u.pinfo = pri;
			tok->type = PRIM;

		} else if ((op = find_op(*arg))) { /* token is an operator */
			if (lasttype == LPAR && op->type == RPAR)
				eprintf("empty parens\n");
			if (lasttype == PRIM && op->type == NOT) { /* need another implicit -a */
				*tok++ = and;
				ntok++;
			}
			tok->type = op->type;
			tok->u.oinfo = op;

		} else { /* token is neither primary nor operator, must be path in the wrong place */
			eprintf("paths must precede expression: %s\n", *arg);
		}
		if (tok->type != LPAR && tok->type != RPAR)
			ntok++; /* won't have parens in rpn */
		lasttype = tok->type;
	}
	tok->type = END;
	ntok++;

	if (print && (arg != argv)) /* need to add -a -print (not just -print) */
		print++;

	/* use shunting-yard to convert from infix to rpn
	 * https://en.wikipedia.org/wiki/Shunting-yard_algorithm
	 * read from infix, resulting rpn ends up in rpn, next position in rpn is out
	 * push operators onto stack, next position in stack is top
	 */
	rpn = emalloc((ntok + print) * sizeof(*rpn));
	for (tok = infix, out = rpn, top = stack; tok->type != END; tok++) {
		switch (tok->type) {
		case PRIM: *out++ = *tok; break;
		case LPAR: *top++ =  tok; break;
		case RPAR:
			while (top-- > stack && (*top)->type != LPAR)
				*out++ = **top;
			if (top < stack)
				eprintf("extra )\n");
			break;
		default:
			/* this expression can be simplified, but I decided copy the
			 * verbage from the wikipedia page in order to more clearly explain
			 * what's going on */
			while (top-- > stack &&
			       (( tok->u.oinfo->lassoc && tok->u.oinfo->prec <= (*top)->u.oinfo->prec) ||
			        (!tok->u.oinfo->lassoc && tok->u.oinfo->prec <  (*top)->u.oinfo->prec)))
				*out++ = **top;

			/* top now points to either an operator we didn't pop, or stack[-1]
			 * either way we need to increment it before using it, then
			 * increment again so the stack works */
			top++;
			*top++ = tok;
			break;
		}
	}
	while (top-- > stack) {
		if ((*top)->type == LPAR)
			eprintf("extra (\n");
		*out++ = **top;
	}

	/* if there was no expression, use -print
	 * if there was an expression but no -print, -exec, or -ok, add -a -print
	 * in rpn, not infix
	 */
	if (print) {
		out->u.pinfo = find_primary("-print");
		out->type = PRIM;
		out++;
	}
	if (print == 2)
		*out++ = and;

	out->type = END;

	/* rpn now holds all operators and arguments in reverse polish notation
	 * values are pushed onto stack, operators pop values off stack into left
	 * and right pointers, pushing operator node back onto stack
	 * could probably just do this during shunting-yard, but this is simpler
	 * code IMO
	 */
	for (tok = rpn, top = stack; tok->type != END; tok++) {
		if (tok->type == PRIM) {
			*top++ = tok;
		} else {
			if (top - stack < tok->u.oinfo->nargs)
				eprintf("insufficient arguments for operator %s\n", tok->u.oinfo->name);
			tok->right = *--top;
			tok->left  = tok->u.oinfo->nargs == 2 ? *--top : NULL;
			*top++ = tok;
		}
	}
	if (--top != stack)
		eprintf("extra arguments\n");

	toks = rpn;
	root = *top;
}

/* for a primary, run and return result
 * for an operator evaluate the left side of the tree, decide whether or not to
 * evaluate the right based on the short-circuit boolean logic, return result
 * NOTE: operator NOT has NULL left side, expression on right side
 */
static int
eval(Tok *tok, Arg *arg)
{
	int ret;

	if (!tok)
		return 0;

	if (tok->type == PRIM) {
		arg->extra = tok->extra;
		return tok->u.pinfo->func(arg);
	}

	ret = eval(tok->left, arg);

	if ((tok->type == AND && ret) || (tok->type == OR && !ret) || tok->type == NOT)
		ret = eval(tok->right, arg);

	return ret ^ (tok->type == NOT);
}

/* evaluate path, if it's a directory iterate through directory entries and
 * recurse
 */
static void
find(char *path, Findhist *hist)
{
	struct stat st;
	DIR *dir;
	struct dirent *de;
	Findhist *f, cur;
	size_t len = strlen(path) + 2; /* null and '/' */
	Arg arg = { path, &st, { NULL } };

	if ((gflags.l || (gflags.h && !hist) ? stat(path, &st) : lstat(path, &st)) < 0) {
		weprintf("failed to stat %s:", path);
		return;
	}

	gflags.prune = 0;

	/* don't eval now iff we will hit the eval at the bottom which means
	 * 1. we are a directory 2. we have -depth 3. we don't have -xdev or we are
	 * on same device (so most of the time we eval here) */
	if (!S_ISDIR(st.st_mode) ||
	    !gflags.depth        ||
	    (gflags.xdev && hist && st.st_dev != hist->dev))
		eval(root, &arg);

	if (!S_ISDIR(st.st_mode)                          ||
	    gflags.prune                                  ||
	    (gflags.xdev && hist && st.st_dev != hist->dev))
		return;

	for (f = hist; f; f = f->next) {
		if (f->dev == st.st_dev && f->ino == st.st_ino) {
			weprintf("loop detected '%s' is '%s'\n", path, f->path);
			return;
		}
	}
	cur.next = hist;
	cur.path = path;
	cur.dev  = st.st_dev;
	cur.ino  = st.st_ino;

	if (!(dir = opendir(path))) {
		weprintf("failed to opendir %s:", path);
		/* should we just ignore this since we hit an error? */
		if (gflags.depth)
			eval(root, &arg);
		return;
	}

	/* FIXME: check errno to see if we are done or encountered an error? */
	while ((de = readdir(dir))) {
		size_t pathcap = len + strlen(de->d_name);
		char pathbuf[pathcap], *p;

		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
			continue;

		p = pathbuf + strlcpy(pathbuf, path, pathcap);
		if (*--p != '/')
			strlcat(pathbuf, "/", pathcap);
		strlcat(pathbuf, de->d_name, pathcap);
		find(pathbuf, &cur);
	}
	closedir(dir); /* check return value? */

	if (gflags.depth)
		eval(root, &arg);
}

static void
usage(void)
{
	eprintf("usage: %s [-H|-L] path... [expression...]\n", argv0);
}

int
main(int argc, char **argv)
{
	char **paths;
	int npaths;
	Tok *t;

	ARGBEGIN {
	case 'H': gflags.l = !(gflags.h = 1); break;
	case 'L': gflags.h = !(gflags.l = 1); break;
	default : usage();
	} ARGEND;

	paths = argv;

	for (; *argv && **argv != '-' && strcmp(*argv, "!") && strcmp(*argv, "("); argv++)
		;

	if (!(npaths = argv - paths))
		eprintf("must specify a path\n");

	parse(argc - npaths, argv);

	/* calculate number of bytes in environ for -exec {} + ARG_MAX avoidance
	 * libc implementation defined whether null bytes, pointers, and alignment
	 * are counted, so count them */
	for (argv = environ; *argv; argv++)
		envlen += strlen(*argv) + 1 + sizeof(*argv);

	if ((argmax = sysconf(_SC_ARG_MAX)) == (size_t)-1)
		argmax = _POSIX_ARG_MAX;

	if (clock_gettime(CLOCK_REALTIME, &start) < 0)
		weprintf("clock_gettime() failed:");

	while (npaths--)
		find(*paths, NULL);

	for (t = toks; t->type != END; t++)
		if (t->type == PRIM && t->u.pinfo->freearg)
			t->u.pinfo->freearg(t->extra);
	free(toks);

	return gflags.ret;
}
