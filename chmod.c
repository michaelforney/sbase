/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include "util.h"

static int    Rflag   = 0;
static char  *modestr = "";
static mode_t mask    = 0;
static int    ret     = 0;

static void
chmodr(const char *path, int depth, void *data)
{
	struct stat st;
	mode_t m;

	if (stat(path, &st) < 0) {
		weprintf("stat %s:", path);
		ret = 1;
		return;
	}

	m = parsemode(modestr, st.st_mode, mask);
	if (chmod(path, m) < 0) {
		weprintf("chmod %s:", path);
		ret = 1;
	} else if (Rflag)
		recurse(path, chmodr, depth, NULL);
}

static void
usage(void)
{
	eprintf("usage: %s [-R [-H | -L | -P]] mode file ...\n", argv0);
}

int
main(int argc, char *argv[])
{
	size_t i;

	argv0 = *(argv++);
	argc--;
	for (; *argv && (*argv)[0] == '-'; argc--, argv++) {
		if (!(*argv)[1])
			usage();
		for (i = 1; (*argv)[i]; i++) {
			switch ((*argv)[i]) {
			case 'R':
				Rflag = 1;
				break;
			case 'H':
			case 'L':
			case 'P':
				recurse_follow = (*argv)[i];
				break;
			case 'r': case 'w': case 'x': case 's': case 't':
				/* -[rwxst] are valid modes, so we're done */
				if (i == 1)
					goto done;
				/* fallthrough */
			case '-':
				/* -- terminator */
				if (i == 1 && !(*argv)[i + 1]) {
					argv++;
					argc--;
					goto done;
				}
				/* fallthrough */
			default:
				usage();
			}
		}
	}
done:
	mask = getumask();
	modestr = *argv;

	if (argc < 2)
		usage();

	for (--argc, ++argv; *argv; argc--, argv++)
		chmodr(*argv, 0, NULL);

	return ret;
}
