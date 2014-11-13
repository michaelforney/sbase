/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util.h"

static void chmodr(const char *);

static bool rflag = false;
static char *modestr = "";
static mode_t mask = 0;
static int ret = 0;

static void
usage(void)
{
	eprintf("usage: %s [-R] mode [file...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int c;
	argv0 = argv[0];

	while (--argc > 0 && (*++argv)[0] == '-') {
		while ((c = *++argv[0])) {
			switch (c) {
			case 'R':
				rflag = true;
				break;
			case 'r': case 'w': case 'x': case 's': case 't':
				/*
				 * -[rwxst] are valid modes so do not interpret
				 * them as options - in any case we are done if
				 * we hit this case
				 */
				--argv[0];
				goto done;
			default:
				usage();
			}
		}
	}

done:
	mask = getumask();
	modestr = argv[0];
	argv++;
	argc--;

	if (argc < 1)
		usage();

	for (; argc > 0; argc--, argv++)
		chmodr(argv[0]);
	return ret;
}

void
chmodr(const char *path)
{
	struct stat st;
	mode_t m;

	if (stat(path, &st) == -1) {
		weprintf("stat %s:", path);
		ret = 1;
		return;
	}

	m = parsemode(modestr, st.st_mode, mask);
	if (chmod(path, m) == -1) {
		weprintf("chmod %s:", path);
		ret = 1;
	}
	if (rflag)
		recurse(path, chmodr);
}
