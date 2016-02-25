/* See LICENSE file for copyright and license details. */
#include <stddef.h>
#include <string.h>

char *
strmem(char *haystack, char *needle, size_t needlelen)
{
	size_t i;

	for (i = 0; i < needlelen; i++) {
		if (haystack[i] == '\0') {
			return NULL;
		}
	}

	for (; haystack[i]; i++) {
		if (!(memcmp(haystack + i - needlelen, needle, needlelen))) {
			return (haystack + i - needlelen);
		}
	}

	return NULL;
}
