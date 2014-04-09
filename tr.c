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
	eprintf("usage: %s [-d] set1 [set2]\n", argv0);
}

static void
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

static void
parsemapping(const char *set1, const char *set2, wchar_t *mappings)
{
	char *s1, *s2;
	wchar_t runeleft;
	wchar_t runeright;
	int leftbytes;
	int rightbytes;

	s1 = (char *)set1;
	if(set2)
		s2 = (char *)set2;
	else
		s2 = (char *)set1;

	while(*s1) {
		if(*s1 == '\\')
			handleescapes(++s1);
		leftbytes = mbtowc(&runeleft, s1, 4);
		s1 += leftbytes;
		if(*s2 == '\\')
			handleescapes(++s2);
		if(*s2 != '\0') {
			rightbytes = mbtowc(&runeright, s2, 4);
			s2 += rightbytes;
		}
		mappings[runeleft] = runeright;
	}
}

static void
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

static void
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
	int dflag = 0;

	setlocale(LC_ALL, "");

	mappings = (wchar_t *)mmap(NULL, 0x110000 * sizeof(wchar_t),
				   PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
	if (mappings == MAP_FAILED)
		eprintf("mmap:");

	ARGBEGIN {
	case 'd':
		dflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if(argc == 0)
		usage();

	if(dflag) {
		if(argc >= 2)
			usage();
		parsemapping(argv[0], NULL, mappings);
		mapfunc = maptonull;
	} else {
		if(argc != 2)
			usage();
		parsemapping(argv[0], argv[1], mappings);
		mapfunc = maptoset;
	}

	while(afgets(&buf, &size, stdin))
		mapfunc(mappings, buf);
	free(buf);
	if(ferror(stdin))
		eprintf("<stdin>: read error:");

	munmap(mappings, 0x110000 * sizeof(wchar_t));

	return EXIT_SUCCESS;
}
