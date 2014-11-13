/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "fs.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-adfpRrv] source... dest\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct stat st;

	ARGBEGIN {
	case 'a':
		cp_aflag = true; /* implies -dpr */
		cp_dflag = true;
		cp_pflag = true;
		cp_rflag = true;
		break;
	case 'd':
		cp_dflag = true;
		break;
	case 'p':
		cp_pflag = true;
		break;
	case 'f':
		cp_fflag = true;
		break;
	case 'R':
	case 'r':
		cp_rflag = true;
		break;
	case 'v':
		cp_vflag = true;
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 2)
		usage();

	if (argc > 2 && !(stat(argv[argc-1], &st) == 0 && S_ISDIR(st.st_mode)))
		eprintf("%s: not a directory\n", argv[argc-1]);
	enmasse(argc, argv, cp);
	return cp_status;
}
