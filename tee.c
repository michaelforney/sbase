/* See LICENSE file for copyright and license details. */
#include <signal.h>
#include <stdio.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-ai] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE **fps = NULL;
	size_t i, n, nfps;
	int ret = 0, aflag = 0, iflag = 0;
	char buf[BUFSIZ];

	ARGBEGIN {
	case 'a':
		aflag = 1;
		break;
	case 'i':
		iflag = 1;
		break;
	default:
		usage();
	} ARGEND

	if (iflag && signal(SIGINT, SIG_IGN) == SIG_ERR)
		eprintf("signal:");
	nfps = argc + 1;
	fps = ecalloc(nfps, sizeof(*fps));

	for (i = 0; i < argc; i++) {
		if (!(fps[i] = fopen(argv[i], aflag ? "a" : "w"))) {
			weprintf("fopen %s:", argv[i]);
			ret = 1;
		}
	}
	fps[i] = stdout;

	while ((n = fread(buf, 1, sizeof(buf), stdin))) {
		for (i = 0; i < nfps; i++) {
			if (fps[i] && fwrite(buf, 1, n, fps[i]) != n) {
				fshut(fps[i], (i != argc) ? argv[i] : "<stdout>");
				fps[i] = NULL;
				ret = 1;
			}
		}
	}

	ret |= fshut(stdin, "<stdin>");
	for (i = 0; i < nfps; i++)
		if (fps[i])
			ret |= fshut(fps[i], (i != argc) ? argv[i] : "<stdout>");

	return ret;
}
