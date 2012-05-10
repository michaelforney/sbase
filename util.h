/* See LICENSE file for copyright and license details. */

#define UTF8_POINT(c) (((c) & 0xc0) != 0x80)

#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#define MAX(x,y)  ((x) > (y) ? (x) : (y))

char *agetcwd(void);
void apathmax(char **, long *);
void enmasse(int, char **, int (*)(const char *, const char *));
void eprintf(const char *, ...);
void enprintf(int, const char *, ...);
long estrtol(const char *, int);
void fnck(const char *, const char *, int (*)(const char *, const char *));
void putword(const char *);
void recurse(const char *, void (*)(const char *));
