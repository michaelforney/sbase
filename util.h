/* See LICENSE file for copyright and license details. */

#define UTF8_POINT(c) (((c) & 0xc0) != 0x80)

#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#define MAX(x,y)  ((x) > (y) ? (x) : (y))

#define LEN(x) (sizeof (x) / sizeof *(x))

#define ARGBEGIN \
	{ \
		if(!argv0) \
			argv0 = argv[0]; \
		for(argc--, argv++; argv[0] && argv[0][0] == '-' && argv[0][1] != '\0'; argc--, argv++) { \
			if(argv[0][1] == '-' && argv[0][2] == '\0') { /* -- signifies end of flags */ \
				argc--; argv++; \
				break; \
			} \
			for(argv[0]++; *argv[0] != '\0'; argv[0]++) \
				switch(*argv[0])

#define ARGEND \
		} \
	}

#define ARGC()    (*argv[0])
#define ARGF()    (*argv[0] ? argv[0] \
                  : argv[1] ? (argc--, *++argv) : NULL)
#define EARGF(x)  (*argv[0] ? argv[0] \
                  : argv[1] ? (argc--, *++argv) : ((x), abort(), NULL))

extern char *argv0;

char *agetcwd(void);
void apathmax(char **, long *);
void enmasse(int, char **, int (*)(const char *, const char *));
void eprintf(const char *, ...);
void enprintf(int, const char *, ...);
long estrtol(const char *, int);
void fnck(const char *, const char *, int (*)(const char *, const char *));
void putword(const char *);
void recurse(const char *, void (*)(const char *));
