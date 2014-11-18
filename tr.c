/* See LICENSE file for copyright and license details. */
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "text.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-d] [-c] set1 [set2]\n", argv0);
}

static int dflag, cflag;
static wchar_t mappings[0x110000];

struct wset_state {
	char *s;               /* current character */
	wchar_t rfirst, rlast; /* first and last in range */
	wchar_t prev;          /* previous returned character */
	int prev_was_range;    /* was the previous character part of a c-c range? */
};

struct set_state {
	char *s, rfirst, rlast, prev;
	int prev_was_octal; /* was the previous returned character written in octal? */
};

static void
set_state_defaults(struct set_state *s)
{
	s->rfirst = 1;
	s->rlast = 0;
	s->prev_was_octal = 1;
}

static void
wset_state_defaults(struct wset_state *s)
{
	s->rfirst = 1;
	s->rlast = 0;
	s->prev_was_range = 1;
}

/* sets *s to the char that was intended to be written.
 * returns how many bytes the s pointer has to advance to skip the
 * escape sequence if it was an octal, always zero otherwise. */
static int
resolve_escape(char *s)
{
	int i;
	unsigned char c;

	switch (*s) {
	case 'n':
		*s = '\n';
		return 0;
	case 't':
		*s = '\t';
		return 0;
	case 'r':
		*s = '\r';
		return 0;
	case 'f':
		*s = '\f';
		return 0;
	case 'a':
		*s = '\a';
		return 0;
	case 'b':
		*s = '\b';
		return 0;
	case 'v':
		*s = '\v';
		return 0;
	case '\\':
		*s = '\\';
		return 0;
	case '\0':
		eprintf("stray '\\' at end of input:");
	default: ;
	}

	if(*s < '0' || *s > '7')
		eprintf("invalid character after '\\':");
	for(i = 0, c = 0; s[i] >= '0' && s[i] <= '7' && i < 3; i++) {
		c <<= 3;
		c += s[i]-'0';
	}
	if(*s > '3' && i == 3)
		eprintf("octal byte cannot be bigger than 377:");
	*s = c;
	return i;
}

#define embtowc(a, b) mbtowc(a, b, 4)

static int
xmbtowc(wchar_t *unicodep, const char *s)
{
	int rv;

	rv = embtowc(unicodep, s);
	if (rv < 0)
			eprintf("mbtowc: invalid input sequence:");
	return rv;
}

static int
has_octal_escapes(const char *s)
{
	while (*s)
		if (*s++ == '\\' && *s >= '0' && *s <= '7')
			return 1;
	return 0;
}

static char
get_next_char(struct set_state *s)
{
	char c;
	int nchars;

start:
	if (s->rfirst <= s->rlast) {
		c = s->rfirst;
		s->rfirst++;
		return c;
	}

	if (*s->s == '-' && !s->prev_was_octal) {
		s->s++;
		if (!*s->s)
			return '-';
		if (*s->s == '\\' && (nchars = resolve_escape(++(s->s))))
			goto char_is_octal;
		s->rlast = *(s->s)++;
		if (!s->rlast)
			return '\0';
		s->prev_was_octal = 1;
		s->rfirst = ++(s->prev);
		goto start;
	}
	if (*s->s == '\\' && (nchars = resolve_escape(++(s->s))))
		goto char_is_octal;

	s->prev_was_octal = 0;
	c = *(s->s)++;
	s->prev = c;
	return c;

char_is_octal:
	s->prev_was_octal = 1;
	c = *s->s;
	s->s += nchars;
	return c;
}

static wchar_t
get_next_wchar(struct wset_state *s)
{
start:
	if (s->rfirst <= s->rlast) {
		s->prev = s->rfirst;
		s->rfirst++;
		return s->prev;
	}

	if (*s->s == '-' && !s->prev_was_range) {
		s->s++;
		if (!*s->s)
			return '-';
		if (*s->s == '\\')
			resolve_escape(++(s->s));
		s->s += xmbtowc(&s->rlast, s->s);
		if (!s->rlast)
			return '\0';
		s->rfirst = ++(s->prev);
		s->prev_was_range = 1;
		goto start;
	}

	if (*s->s == '\\')
		resolve_escape(++(s->s));
	s->s += xmbtowc(&s->prev, s->s);
	s->prev_was_range = 0;
	return s->prev;
}

static int
is_mapping_wide(const char *set1, const char *set2)
{
	struct set_state ss1, ss2;
	struct wset_state wss1, wss2;
	wchar_t wc1, wc2, last_wc2;

	if (has_octal_escapes(set1)) {
		set_state_defaults(&ss1);
		ss1.s = (char *) set1;
		if (set2) {
			set_state_defaults(&ss2);
			ss2.s = (char *) set2;
			/* if the character returned is from an octal triplet, it might be null
			 * and still need to continue */
			while ((wc1 = (unsigned char) get_next_char(&ss1)) || ss1.prev_was_octal ) {
				if (!(wc2 = (unsigned char) get_next_char(&ss2)))
					wc2 = last_wc2;
				mappings[wc1] = wc2;
				last_wc2 = wc2;
			}
		} else {
			while ((wc1 = (unsigned char) get_next_char(&ss1)) || ss1.prev_was_octal)
				mappings[wc1] = 1;
		}
		return 0;
	} else {
		wset_state_defaults(&wss1);
		wss1.s = (char *) set1;
		if (set2) {
			wset_state_defaults(&wss2);
			wss2.s = (char *) set2;
			while ((wc1 = get_next_wchar(&wss1))) {
				if (!(wc2 = get_next_wchar(&wss2)))
					wc2 = last_wc2;
				mappings[wc1] = wc2;
				last_wc2 = wc2;
			}
		} else {
			while ((wc1 = get_next_wchar(&wss1)))
				mappings[wc1] = 1;
		}
		return 1;
	}
	return 0; /* unreachable */
}

static void
wmap_null(char *in, ssize_t nbytes)
{
	char *s;
	wchar_t rune;
	int parsed_bytes = 0;

	s = in;
	while (nbytes) {
		parsed_bytes = embtowc(&rune, s);
		if (parsed_bytes < 0) {
			rune = *s;
			parsed_bytes = 1;
		}
		if (((!mappings[rune])&1) ^ cflag)
			putwchar(rune);
		s += parsed_bytes;
		nbytes -= parsed_bytes;
	}
}

static void
wmap_set(char *in, ssize_t nbytes)
{
	char *s;
	wchar_t rune;
	int parsed_bytes = 0;

	s = in;
	while (nbytes) {
		parsed_bytes = embtowc(&rune, s);
		if (parsed_bytes < 0) {
			rune = *s;
			parsed_bytes = 1;
		}
		if (!mappings[rune])
			putwchar(rune);
		else
			putwchar(mappings[rune]);
		nbytes -= parsed_bytes;
		s += parsed_bytes;
	}
}

static void
map_null(char *in, ssize_t nbytes)
{
	char *s;

	for (s = in; nbytes; s++, nbytes--)
		if (((!mappings[(unsigned char)*s])&1) ^ cflag)
			putchar(*s);
}

static void
map_set(char *in, ssize_t nbytes)
{
	char *s;

	for (s = in; nbytes; s++, nbytes--)
		if (!mappings[(unsigned char)*s])
			putchar(*s);
		else
			putchar(mappings[(unsigned char)*s]);
}

int
main(int argc, char *argv[])
{
	char *buf = NULL;
	size_t size = 0;
	ssize_t nbytes;
	void (*mapfunc)(char*, ssize_t);

	setlocale(LC_ALL, "");
	dflag = cflag = 0;

	ARGBEGIN {
	case 'd':
		dflag = 1;
		break;
	case 'c':
		cflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0)
		usage();

	if (dflag) {
		if (argc != 1)
			usage();
		if (is_mapping_wide(argv[0], NULL))
			mapfunc = wmap_null;
		else
			mapfunc = map_null;
	} else if (cflag) {
		usage();
	} else if (argc == 2) {
		if (is_mapping_wide(argv[0], argv[1]))
			mapfunc = wmap_set;
		else
			mapfunc = map_set;
	} else {
		usage();
	}

	while ((nbytes = getline(&buf, &size, stdin)) != -1)
		mapfunc(buf, nbytes);
	free(buf);
	if (ferror(stdin))
		eprintf("<stdin>: read error:");

	return 0;
}
