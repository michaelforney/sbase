#include <stddef.h>
#include <time.h>

typedef struct target Target;

enum {
	NOEXPORT,
	EXPORT,
};

enum {
	UNDEF,
	ENVIRON,
	CMDLINE,
	INTERNAL,
	MAKEFILE,
	MAKEFLAGS,
};

struct loc {
	char *fname;
	int lineno;
};

struct action {
	char *line;
	struct loc loc;
};

struct target {
	char *name;
	char *target;
	char *req;
	time_t stamp;
	int defined;

	int ndeps;
	struct target **deps;

	int nactions;
	struct action *actions;

	struct target *next;
};

void *emalloc(size_t);
void *erealloc(void *, size_t);
char *estrdup(char *);

void dumprules(void);
void dumpmacros(void);

char *expandstring(char *, Target *, struct loc *);
void addtarget(char *, int);
void inject(char *);
int build(char *);
int hash(char *);
int parse(char *);
void debug(char *, ...);
void error(char *, ...);
void warning(char *, ...);
void adddep(char *, char *);
void addrule(char *, struct action *, int);
void freeloc(struct loc *);

char *getmacro(char *);
void setmacro(char *, char *, int, int);

/* system depdendant */
void killchild(void);
time_t stamp(char *);
int launch(char *, int);
int putenv(char *);
void exportvar(char *, char *);
int is_dir(char *);

/* main.c */
extern int kflag, dflag, nflag, iflag, sflag;
extern int eflag, pflag, tflag, qflag;
extern int exitstatus;

#ifdef SIGABRT
extern volatile sig_atomic_t stop;
#endif

/* defaults.c */
extern char defaults[];
