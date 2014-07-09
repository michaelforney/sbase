/* See LICENSE file for copyright and license details. */
#include <stdbool.h>

extern bool cp_aflag;
extern bool cp_dflag;
extern bool cp_fflag;
extern bool cp_pflag;
extern bool cp_rflag;

extern bool rm_fflag;
extern bool rm_rflag;

int cp(const char *, const char *);
void rm(const char *);
