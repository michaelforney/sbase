/* See LICENSE file for copyright and license details. */
#include <locale.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include "util.h"

typedef struct {
	FILE *fp;
	const char *name;
} Fdescr;

static size_t unescape(wchar_t *);
static wint_t in(Fdescr *);
static void out(wchar_t);
static void sequential(Fdescr *, int, const wchar_t *, size_t);
static void parallel(Fdescr *, int, const wchar_t *, size_t);

static void
usage(void)
{
	eprintf("usage: %s [-s] [-d list] file...\n", argv0);
}

int
main(int argc, char *argv[])
{
	const char *adelim = NULL;
	bool seq = false;
	wchar_t *delim = NULL;
	size_t len;
	Fdescr *dsc = NULL;
	int i;

	setlocale(LC_CTYPE, "");

	ARGBEGIN {
	case 's':
		seq = true;
		break;
	case 'd':
		adelim = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND;

	if(argc == 0)
		usage();

	/* populate delimeters */
	if(!adelim)
		adelim = "\t";

	len = mbstowcs(NULL, adelim, 0);
	if(len == (size_t)-1)
		eprintf("invalid delimiter\n");

	if(!(delim = malloc((len + 1) * sizeof(*delim))))
		eprintf("out of memory\n");

	mbstowcs(delim, adelim, len);
	len = unescape(delim);
	if(len == 0)
		eprintf("no delimiters specified\n");

	/* populate file list */
	if(!(dsc = malloc(argc * sizeof(*dsc))))
		eprintf("out of memory\n");

	for(i = 0; i < argc; i++) {
		if(strcmp(argv[i], "-") == 0)
			dsc[i].fp = stdin;
		else
			dsc[i].fp = fopen(argv[i], "r");

		if(!dsc[i].fp)
			eprintf("can't open '%s':", argv[i]);

		dsc[i].name = argv[i];
	}

	if(seq)
		sequential(dsc, argc, delim, len);
	else
		parallel(dsc, argc, delim, len);

	for(i = 0; i < argc; i++) {
		if(dsc[i].fp != stdin)
			(void)fclose(dsc[i].fp);
	}

	free(delim);
	free(dsc);

	return 0;
}

static size_t
unescape(wchar_t *delim)
{
	wchar_t c;
	size_t i;
	size_t len;

	for(i = 0, len = 0; (c = delim[i++]) != '\0'; len++) {
		if(c == '\\') {
			switch(delim[i++]) {
			case 'n':
				delim[len] = '\n';
				break;
			case 't':
				delim[len] = '\t';
				break;
			case '0':
				delim[len] = '\0';
				break;
			case '\\':
				delim[len] = '\\';
				break;
			case '\0':
			default:
				/* POSIX: unspecified results */
				return len;
			}
		} else
			delim[len] = c;
	}

	return len;
}

static wint_t
in(Fdescr *f)
{
	wint_t c = fgetwc(f->fp);

	if(c == WEOF && ferror(f->fp))
		eprintf("'%s' read error:", f->name);

	return c;
}

static void
out(wchar_t c)
{
	putwchar(c);
	if(ferror(stdout))
		eprintf("write error:");
}

static void
sequential(Fdescr *dsc, int len, const wchar_t *delim, size_t cnt)
{
	int i;
	size_t d;
	wint_t c, last;

	for(i = 0; i < len; i++) {
		d = 0;
		last = WEOF;

		while((c = in(&dsc[i])) != WEOF) {
			if(last == '\n') {
				if(delim[d] != '\0')
					out(delim[d]);

				d++;
				d %= cnt;
			}

			if(c != '\n')
				out((wchar_t)c);

			last = c;
		}

		if(last == '\n')
			out((wchar_t)last);
	}
}

static void
parallel(Fdescr *dsc, int len, const wchar_t *delim, size_t cnt)
{
	int last, i;
	wint_t c, o;
	wchar_t d;

	do {
		last = 0;
		for(i = 0; i < len; i++) {
			d = delim[i % cnt];

			do {
				o = in(&dsc[i]);
				c = o;
				switch(c) {
				case WEOF:
					if(last == 0)
						break;

					o = '\n';
					/* fallthrough */
				case '\n':
					if(i != len - 1)
						o = d;
					break;
				default:
					break;
				}

				if(o != WEOF) {
					/* pad with delimiters up to this point */
					while(++last < i) {
						if(d != '\0')
							out(d);
					}
					out((wchar_t)o);
				}
			} while(c != '\n' && c != WEOF);
		}
	} while(last > 0);
}
