/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "../util.h"

off_t
parseoffset(const char *str)
{
	off_t res;
	size_t scale = 1;
	int base = 10;
	char *end;

	if (!str || !*str) {
		weprintf("parseoffset: empty string\n");
		return -1;
	}

	/* bases */
	if (!strncasecmp(str, "0x", strlen("0x"))) {
		base = 16;
	} else if (*str == '0') {
		str++;
		base = 8;
	}

	res = strtol(str, &end, base);
	if (res < 0) {
		weprintf("parseoffset %s: negative value\n", str);
		return -1;
	}

	/* suffix */
	if (*end) {
		switch (toupper((int)*end)) {
		case 'B':
			scale = 512L;
			break;
		case 'K':
			scale = 1024L;
			break;
		case 'M':
			scale = 1024L * 1024L;
			break;
		case 'G':
			scale = 1024L * 1024L * 1024L;
			break;
		default:
			weprintf("parseoffset %s: invalid suffix\n", str);
			return -1;
		}
	}

	/* prevent overflow */
	if (res > (SIZE_MAX / scale)) {
		weprintf("parseoffset %s: out of range\n", str);
		return -1;
	}

	return res * scale;
}
