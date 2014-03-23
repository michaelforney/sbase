/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../util.h"
#include "../crypt.h"

int
cryptmain(int argc, char *argv[],
	  struct crypt_ops *ops, uint8_t *md, size_t sz)
{
	FILE *fp;
	int ret = EXIT_SUCCESS;

	if (argc == 0) {
		cryptsum(ops, stdin, "<stdin>", md);
		mdprint(md, "<stdin>", sz);
	} else {
		for (; argc > 0; argc--) {
			if((fp = fopen(*argv, "r")) == NULL) {
				weprintf("fopen %s:", *argv);
				ret = EXIT_FAILURE;
				continue;
			}
			if(cryptsum(ops, fp, *argv, md) == 1)
				ret = EXIT_FAILURE;
			else
				mdprint(md, *argv, sz);
			fclose(fp);
			argv++;
		}
	}
	return ret;
}

int
cryptsum(struct crypt_ops *ops, FILE *fp, const char *f,
	 uint8_t *md)
{
	uint8_t buf[BUFSIZ];
	size_t n;

	ops->init(ops->s);
	while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
		ops->update(ops->s, buf, n);
	if (ferror(fp)) {
		weprintf("read error: %s:", f);
		return 1;
	}
	ops->sum(ops->s, md);
	return 0;
}

void
mdprint(const uint8_t *md, const char *f, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++)
		printf("%02x", md[i]);
	printf("  %s\n", f);
}
