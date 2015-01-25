/* See LICENSE file for copyright and license details. */
#include <sys/types.h>

#include <regex.h>
#include <stddef.h>

#include "arg.h"
#include "compat.h"

#define UTF8_POINT(c) (((c) & 0xc0) != 0x80)

#undef MIN
#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#undef MAX
#define MAX(x,y)  ((x) > (y) ? (x) : (y))

#define LEN(x) (sizeof (x) / sizeof *(x))

extern char *argv0;

char *agetcwd(void);
void apathmax(char **, long *);

void *ecalloc(size_t, size_t);
void *emalloc(size_t size);
void *erealloc(void *, size_t);
char *estrdup(const char *);

void enprintf(int, const char *, ...);
void eprintf(const char *, ...);
void weprintf(const char *, ...);

double estrtod(const char *);
long estrtol(const char *, int);

#undef strcasestr
char *strcasestr(const char *, const char *);

#undef strlcat
size_t strlcat(char *, const char *, size_t);
#undef strlcpy
size_t strlcpy(char *, const char *, size_t);

#undef strsep
char *strsep(char **, const char *);

/* regex */
int enregcomp(int, regex_t *, const char *, int);
int eregcomp(regex_t *, const char *, int);

/* misc */
void enmasse(int, char **, int (*)(const char *, const char *));
void fnck(const char *, const char *, int (*)(const char *, const char *));
mode_t getumask(void);
char *humansize(double);
mode_t parsemode(const char *, mode_t, mode_t);
void putword(const char *);
void recurse(const char *, void (*)(const char *));
