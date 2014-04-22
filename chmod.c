/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "util.h"

static void chmodr(const char *);

static bool rflag = false;
static int oper = '=';
static mode_t mode = 0;

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
	parsemode(argv[0], &mode, &oper);
	argv++;
	argc--;

	if(argc < 1)
		usage();

	for (; argc > 0; argc--, argv++)
		chmodr(argv[0]);
	return EXIT_SUCCESS;
}

void
chmodr(const char *path)
{
	struct stat st;

	if(stat(path, &st) == -1)
		eprintf("stat %s:", path);

	switch(oper) {
	case '+':
		st.st_mode |= mode;
		break;
	case '-':
		st.st_mode &= ~mode;
		break;
	case '=':
		st.st_mode = mode;
		break;
	}
	if(chmod(path, st.st_mode) == -1)
		weprintf("chmod %s:", path);
	if(rflag)
		recurse(path, chmodr);
}
