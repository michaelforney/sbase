/* See LICENSE file for copyright and license details. */
#include <stddef.h>
#include <sys/types.h>
#include "arg.h"

#define UTF8_POINT(c) (((c) & 0xc0) != 0x80)

#undef MIN
#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#undef MAX
#define MAX(x,y)  ((x) > (y) ? (x) : (y))

#define LEN(x) (sizeof (x) / sizeof *(x))

extern char *argv0;

char *agetcwd(void);
void apathmax(char **, long *);
void enmasse(int, char **, int (*)(const char *, const char *));
void *ecalloc(size_t, size_t);
void *emalloc(size_t size);
void *erealloc(void *, size_t);
void eprintf(const char *, ...);
void enprintf(int, const char *, ...);
char *estrdup(const char *);
double estrtod(const char *);
long estrtol(const char *, int);
void fnck(const char *, const char *, int (*)(const char *, const char *));
char *humansize(double);
void putword(const char *);
void recurse(const char *, void (*)(const char *));

#undef strlcat
size_t strlcat(char *, const char *, size_t);
#undef strlcpy
size_t strlcpy(char *, const char *, size_t);
void weprintf(const char *, ...);

mode_t getumask(void);
mode_t parsemode(const char *, mode_t, mode_t);
