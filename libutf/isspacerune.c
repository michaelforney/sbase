/* Automatically generated by mkrunetype.awk */
#include <stdlib.h>

#include "../utf.h"
#include "runetype.h"

static Rune space2[][2] = {
	{ 0x0009, 0x000D },
	{ 0x001C, 0x0020 },
	{ 0x2000, 0x200A },
	{ 0x2028, 0x2029 },
};

static Rune space1[] = {
	0x0085,
	0x00A0,
	0x1680,
	0x202F,
	0x205F,
	0x3000,
};

int
isspacerune(Rune r)
{
	Rune *match;

	if(bsearch(&r, space2, nelem(space2), sizeof *space2, &rune2cmp))
		return 1;
	if(bsearch(&r, space1, nelem(space1), sizeof *space1, &rune1cmp))
		return 1;
	return 0;
}
