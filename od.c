/* See LICENSE file for copyright and license details. */
#include <stdlib.h>
#include <string.h>

#include "util.h"

static char addr_radix = 'o';
static char *type = "o";

static void
usage(void)
{
	eprintf("usage: %s [-A d|o|x|n] [-t a|c|d|o|u|x] [file ...]\n", argv0);
}

static void
print_address(FILE *f, size_t addr)
{
	switch (addr_radix) {
	case 'x':
		fprintf(f, "%06zx ", addr);
		break;
	case 'd':
		fprintf(f, "%06zd ", addr);
		break;
	case 'n':
		fprintf(f, "%s", " ");
		break;
	case 'o':
	default:
		fprintf(f, "%06zo ", addr);
		break;
	}
}

static char char_string[2];

static const char *
escaped_char(unsigned char c)
{
	switch (c) {
	case '\0':
		return "\\0";
	case '\a':
		return "\\a";
	case '\b':
		return "\\b";
	case '\t':
		return "\\t";
	case '\n':
		return "\\n";
	case '\v':
		return "\\v";
	case '\f':
		return "\\f";
	case '\r':
		return "\\r";
	default:
		char_string[0] = c;
		char_string[1] = '\0';
		return char_string;
	}
}

static const char *
named_char(unsigned char c)
{
	static const int table_size = 33;
	static const char * named_chars[] = {"nul", "soh", "stx", "etx", "eot", 
	 "enq", "ack", "bel", "bs", "ht", "nl", "vt", "ff", "cr", "so", "si", 
	 "dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb", "can", "em", 
	 "sub", "esc", "fs", "gs", "rs", "us", "sp"};

	c &= ~128; /* clear high bit of byte, as required by standard */
	if (c < table_size) {
		return named_chars[c];
	} else if (c == 127) {
		return "del"; 
	} else {
		char_string[0] = c;
		char_string[1] = '\0';
		return char_string;
	}
}

static void
print_content(FILE *f, char type, unsigned char cont)
{
	switch (type) {
	case 'a':
		fprintf(f, "%3s ", named_char(cont));
		break;
	case 'c':
		fprintf(f, "%3s ", escaped_char(cont));
		break;
	case 'd':
		fprintf(f, "%4hhd ", cont);
		break;
	case 'o':
		fprintf(f, "%03hho ", cont);
		break;
	case 'u':
		fprintf(f, "%3hhu ", cont);
		break;
	case 'x':
		fprintf(f, "%02hhx ", cont);
		break;
	}
}

static void
od(FILE *fp_in, const char *name_in, FILE *fp_out, const char *name_out)
{
	unsigned char buf[BUFSIZ];
	size_t addr, buf_size, i;
	const size_t bytes_per_line = 16;

	addr = 0;
	for (; (buf_size = fread(buf, 1, BUFSIZ, fp_in)); ) {
		for (i = 0; i < buf_size; ++i, ++addr) {
			if ((addr % bytes_per_line) == 0) {
				if (addr != 0) fprintf(fp_out, "%s", "\n");
				print_address(fp_out, addr);
			}
			print_content(fp_out, type[0], buf[i]);
		}
		if (feof(fp_in) || ferror(fp_in) || ferror(fp_out))
			break;
	}
	fprintf(fp_out, "\n%.7zx \n", addr);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int ret = 0;
    char *s;

	ARGBEGIN {
	case 'A':
		s = EARGF(usage());
		if (strlen(s) > 1 || !strchr("doxn", s[0]))
			usage();
		addr_radix = s[0];
		break;
	case 't':
		type = EARGF(usage());
		if (strlen(type) > 1 || !strchr("acdoux", type[0]))
			usage();
		break;
	default:
		usage();
	} ARGEND;

	if (!argc) {
		od(stdin, "<stdin>", stdout, "<stdout>");
	} else {
		for (; *argv; argc--, argv++) {
			if (!strcmp(*argv, "-")) {
				*argv = "<stdin>";
				fp = stdin;
			} else if (!(fp = fopen(*argv, "r"))) {
				weprintf("fopen %s:", *argv);
				ret = 1;
				continue;
			}
			od(fp, *argv, stdout, "<stdout>");
			if (fp != stdin && fshut(fp, *argv))
				ret = 1;
		}
	}

	ret |= fshut(stdin, "<stdin>") | fshut(stdout, "<stdout>");

	return ret;
}
