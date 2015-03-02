/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include "util.h"

static int    Rflag   = 0;
static char  *modestr = "";
static mode_t mask    = 0;
static int    ret     = 0;

void
chmodr(const char *path, int depth)
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
	}
	if (Rflag)
		recurse(path, chmodr, depth);
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

	argv0 = argv[0];
	for (i = 1; i < argc && argv[i][0] == '-'; i++) {
		switch (argv[i][1]) {
		case 'R':
			Rflag = 1;
			break;
		case 'H':
		case 'L':
		case 'P':
			recurse_follow = argv[i][1];
			break;
		case 'r': case 'w': case 'x': case 's': case 't':
			/*
			 * -[rwxst] are valid modes so do not interpret
			 * them as options - in any case we are done if
			 * we hit this case
			 */
			goto done;
		default:
			usage();
		}
	}
done:
	mask = getumask();
	modestr = argv[i];

	if (argc - i - 1 < 1)
		usage();

	for (++i; i < argc; i++)
		chmodr(argv[i], 0);

	return ret;
}
