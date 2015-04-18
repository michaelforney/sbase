/* See LICENSE file for copyright and license details. */
#include "fs.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-f] [-Rr] file ...\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct recursor r = { .fn = rm, .hist = NULL, .depth = 0, .follow = 'P', .flags = NODIRS };

	ARGBEGIN {
	case 'f':
		rm_fflag = 1;
		break;
	case 'R':
	case 'r':
		r.flags &= ~NODIRS;
		break;
	default:
		usage();
	} ARGEND;

	if (!argc) {
		if (!rm_fflag)
			usage();
		else
			return 0;
	}

	for (; *argv; argc--, argv++)
		recurse(*argv, NULL, &r);

	return rm_status || recurse_status;
}
