/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <locale.h>
#include <wchar.h>
#include "text.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s set1 [set2]\n", argv0);
}

void
handleescapes(char *s)
{
	switch(*s) {
	case 'n':
		*s = '\n';
		break;
	case 't':
		*s = '\t';
		break;
	case '\\':
		*s = '\\';
		break;
	case 'r':
		*s = '\r';
		break;
	case 'f':
		*s = '\f';
		break;
	case 'a':
		*s = '\a';
		break;
	case 'b':
		*s = '\b';
		break;
	case 'v':
		*s = '\v';
		break;
	}
}

void
parsemapping(const char *set1, const char *set2, wchar_t *mappings)
{
	char *s;
	wchar_t runeleft;
	wchar_t runeright;
	int leftbytes;
	int rightbytes;
	size_t n = 0;
	size_t lset2;

	if(set2) {
		lset2 = strnlen(set2, 255 * sizeof(wchar_t));
	} else {
		set2 = &set1[0];
		lset2 = 0;
	}

	s = (char *)set1;
	while(*s) {
		if(*s == '\\')
			handleescapes(++s);
		leftbytes = mbtowc(&runeleft, s, 4);
		if(set2[n] != '\0')
			rightbytes = mbtowc(&runeright, set2 + n, 4);
		mappings[runeleft] = runeright;
		s += leftbytes;
		if(n < lset2)
			n += rightbytes;
	}
}

void
maptonull(const wchar_t *mappings, char *in)
{
	const char *s;
	wchar_t runeleft;
	int leftbytes = 0;

	s = in;
	while(*s) {
		leftbytes = mbtowc(&runeleft, s, 4);
		if(!mappings[runeleft])
			putwchar(runeleft);
		s += leftbytes;
	}
}

void
maptoset(const wchar_t *mappings, char *in)
{
	const char *s;
	wchar_t runeleft;
	int leftbytes = 0;

	s = in;
	while(*s) {
		leftbytes = mbtowc(&runeleft, s, 4);
		if(!mappings[runeleft])
			putwchar(runeleft);
		else
			putwchar(mappings[runeleft]);
		s += leftbytes;
	}
}

int
main(int argc, char *argv[])
{
	wchar_t *mappings;
	char *buf = NULL;
	size_t size = 0;
	void (*mapfunc)(const wchar_t*, char*);

	setlocale(LC_ALL, "");

	mappings = (wchar_t *)mmap(NULL, 0x110000 * sizeof(wchar_t),
				   PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
	if (mappings == MAP_FAILED)
		eprintf("mmap:");

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if(argc == 0)
		usage();

	if(argc >= 2) {
		parsemapping(argv[0], argv[1], mappings);
		mapfunc = maptoset;
	} else {
		parsemapping(argv[0], NULL, mappings);
		mapfunc = maptonull;
	}

	while(afgets(&buf, &size, stdin))
		mapfunc(mappings, buf);
	free(buf);
	if(ferror(stdin))
		eprintf("<stdin>: read error:");

	munmap(mappings, 0x110000 * sizeof(wchar_t));

	return EXIT_SUCCESS;
}
