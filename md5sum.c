/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "util.h"
#include "crypt.h"
#include "md5.h"

struct md5 s;
struct crypt_ops md5_ops = {
	md5_init,
	md5_update,
	md5_sum,
	&s,
};

static void
usage(void)
{
	eprintf("usage: %s [file...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	uint8_t md[MD5_DIGEST_LENGTH];

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc == 0) {
		cryptsum(&md5_ops, stdin, "<stdin>", md);
		mdprint(md, "<stdin>", sizeof(md));
	} else {
		for (; argc > 0; argc--) {
			if ((fp = fopen(*argv, "r"))  == NULL)
				eprintf("fopen %s:", *argv);
			cryptsum(&md5_ops, fp, *argv, md);
			mdprint(md, *argv, sizeof(md));
			fclose(fp);
			argv++;
		}
	}

	return 0;
}
