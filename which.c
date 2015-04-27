#include <sys/stat.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "arg.h"
#include "util.h"

static int aflag;

static int
which(const char *path, const char *name)
{
	char file[PATH_MAX], *p, *s, *ptr;
	size_t len;
	struct stat st;
	int found = 0;

	p = ptr = estrdup(path);
	for (s = p; (s = strsep(&p, ":")); ) {
		if (!s[0])
			s = ".";
		len = strlen(s);

		if (snprintf(file, sizeof(file), "%s%s%s",
			s,
			len > 0 && s[len - 1] != '/' ? "/" : "",
			name) >= sizeof(file))
			eprintf("path too long\n");

		if (stat(file, &st) == 0 && S_ISREG(st.st_mode) &&
		    access(file, X_OK) == 0) {
			found = 1;
			puts(file);
			if (!aflag)
				break;
		}
	}
	free(ptr);

	return found;
}

static void
usage(void)
{
	eprintf("usage: %s [-a] name...\n", argv0);
}

int
main(int argc, char *argv[])
{
	char *path;
	int i, found;

	ARGBEGIN {
	case 'a':
		aflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (!argc)
		usage();

	if (!(path = getenv("PATH")))
		eprintf("$PATH not set\n");

	for (i = 0, found = 0; i < argc; i++) {
		if (which(path, argv[i]))
			found++;
		else
			weprintf("%s: Command not found.\n", argv[i]);
	}
	return !found ? 2 : found == argc ? 0 : 1;
}
