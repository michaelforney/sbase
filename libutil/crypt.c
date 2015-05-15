/* See LICENSE file for copyright and license details. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../crypt.h"
#include "../text.h"
#include "../util.h"

static int
hexdec(int c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return -1; /* unknown character */
}

static int
mdcheckline(const char *s, uint8_t *md, size_t sz)
{
	size_t i;
	int b1, b2;

	for (i = 0; i < sz; i++) {
		if (!*s || (b1 = hexdec(*s++)) < 0)
			return -1; /* invalid format */
		if (!*s || (b2 = hexdec(*s++)) < 0)
			return -1; /* invalid format */
		if ((uint8_t)((b1 << 4) | b2) != md[i])
			return 0; /* value mismatch */
	}
	return (i == sz) ? 1 : 0;
}

static void
mdchecklist(FILE *listfp, struct crypt_ops *ops, uint8_t *md, size_t sz,
            int *formatsucks, int *noread, int *nonmatch)
{
	FILE *fp;
	size_t bufsiz = 0;
	int r;
	char *line = NULL, *file, *p;

	while (getline(&line, &bufsiz, listfp) > 0) {
		if (!(file = strstr(line, "  "))) {
			(*formatsucks)++;
			continue;
		}
		if ((file - line) / 2 != sz) {
			(*formatsucks)++; /* checksum length mismatch */
			continue;
		}
		*file = '\0';
		file += 2;
		for (p = file; *p && *p != '\n' && *p != '\r'; p++); /* strip newline */
		*p = '\0';
		if (!(fp = fopen(file, "r"))) {
			weprintf("fopen %s:", file);
			(*noread)++;
			continue;
		}
		cryptsum(ops, fp, file, md);
		r = mdcheckline(line, md, sz);
		if (r == 1) {
			printf("%s: OK\n", file);
		} else if (r == 0) {
			printf("%s: FAILED\n", file);
			(*nonmatch)++;
		} else {
			(*formatsucks)++;
		}
		fclose(fp);
	}
	free(line);
}

int
cryptcheck(int argc, char *argv[], struct crypt_ops *ops, uint8_t *md, size_t sz)
{
	FILE *fp;
	int formatsucks = 0, noread = 0, nonmatch = 0, ret = 0;

	if (argc == 0) {
		mdchecklist(stdin, ops, md, sz, &formatsucks, &noread, &nonmatch);
	} else {
		for (; *argv; argc--, argv++) {
			if (!(fp = fopen(*argv, "r"))) {
				weprintf("fopen %s:", *argv);
				ret = 1;
				continue;
			}
			mdchecklist(fp, ops, md, sz, &formatsucks, &noread, &nonmatch);
			fclose(fp);
		}
	}

	if (formatsucks) {
		weprintf("%d lines are improperly formatted\n", formatsucks);
		ret = 1;
	}
	if (noread) {
		weprintf("%d listed file could not be read\n", noread);
		ret = 1;
	}
	if (nonmatch) {
		weprintf("%d computed checksums did NOT match\n", nonmatch);
		ret = 1;
	}

	return ret;
}

int
cryptmain(int argc, char *argv[], struct crypt_ops *ops, uint8_t *md, size_t sz)
{
	FILE *fp;
	int ret = 0;

	if (argc == 0) {
		cryptsum(ops, stdin, "<stdin>", md);
		mdprint(md, "<stdin>", sz);
	} else {
		for (; *argv; argc--, argv++) {
			if ((*argv)[0] == '-' && !(*argv)[1]) {
				*argv = "<stdin>";
				fp = stdin;
			} else if (!(fp = fopen(*argv, "r"))) {
				weprintf("fopen %s:", *argv);
				ret = 1;
				continue;
			}
			if (cryptsum(ops, fp, *argv, md)) {
				ret = 1;
			} else {
				mdprint(md, *argv, sz);
			}
			if (fp != stdin && fshut(fp, *argv))
				ret = 1;
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
		weprintf("%s: read error:", f);
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
