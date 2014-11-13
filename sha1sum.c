/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "crypt.h"
#include "sha1.h"
#include "util.h"

struct sha1 s;
struct crypt_ops sha1_ops = {
	sha1_init,
	sha1_update,
	sha1_sum,
	&s,
};

static void
usage(void)
{
	eprintf("usage: %s [-c] [file...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	uint8_t md[SHA1_DIGEST_LENGTH];
	char *checkfile = NULL;
	bool cflag = false;

	ARGBEGIN {
	case 'c':
		cflag = true;
		checkfile = ARGF();
		break;
	default:
		usage();
	} ARGEND;

	if (cflag)
		return cryptcheck(checkfile, argc, argv, &sha1_ops, md, sizeof(md));
	return cryptmain(argc, argv, &sha1_ops, md, sizeof(md));
}
