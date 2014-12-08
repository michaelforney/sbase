/* See LICENSE file for copyright and license details. */
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
		/* implies -dpr */
		cp_aflag = cp_Pflag = cp_pflag = cp_rflag = 1;
		break;
	case 'P':
		cp_Pflag = 1;
		break;
	case 'p':
		cp_pflag = 1;
		break;
	case 'f':
		cp_fflag = 1;
		break;
	case 'R':
	case 'r':
		cp_rflag = 1;
		break;
	case 'v':
		cp_vflag = 1;
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
