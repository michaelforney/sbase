/* See LICENSE file for copyright and license details. */
#include <sys/types.h>

struct history {
        struct history *prev;
        dev_t dev;
        ino_t ino;
};

struct recursor {
        void (*fn)(const char *, void *, struct recursor *);
        struct history *hist;
        int depth;
        int follow;
        int flags;
};

enum {
        SAMEDEV = 1 << 0,
};

extern int cp_aflag;
extern int cp_fflag;
extern int cp_pflag;
extern int cp_rflag;
extern int cp_vflag;
extern int cp_HLPflag;
extern int cp_status;

extern int rm_fflag;
extern int rm_rflag;
extern int rm_status;

extern int recurse_status;

void recurse(const char *, void *, struct recursor *);

int cp(const char *, const char *, int);
void rm(const char *, void *, struct recursor *);
