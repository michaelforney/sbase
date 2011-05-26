/* See LICENSE file for copyright and license details. */

char *agetcwd(void);
void enmasse(int, char **, int (*)(const char *, const char *));
void eprintf(const char *, ...);
void recurse(const char *, void (*)(const char *));
