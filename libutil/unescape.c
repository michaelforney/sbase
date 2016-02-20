/* See LICENSE file for copyright and license details. */
#include <string.h>

#include "../util.h"

size_t
unescape(char *s)
{
	size_t len, i, off, m, factor, q;

	len = strlen(s);

	for (i = 0; i < len; i++) {
		if (s[i] != '\\')
			continue;
		off = 0;

		switch (s[i + 1]) {
		case '\\': s[i] = '\\'; off++; break;
		case '\'': s[i] = '\'', off++; break;
		case '"':  s[i] =  '"', off++; break;
		case 'a':  s[i] = '\a'; off++; break;
		case 'b':  s[i] = '\b'; off++; break;
		case 'e':  s[i] =  033; off++; break;
		case 'f':  s[i] = '\f'; off++; break;
		case 'n':  s[i] = '\n'; off++; break;
		case 'r':  s[i] = '\r'; off++; break;
		case 't':  s[i] = '\t'; off++; break;
		case 'v':  s[i] = '\v'; off++; break;
		case 'x':
			/* "\xH[H]" hexadecimal escape */
			for (m = i + 2; m < i + 1 + 3 && m < len; m++)
				if ((s[m] < '0' && s[m] > '9') &&
				    (s[m] < 'A' && s[m] > 'F') &&
				    (s[m] < 'a' && s[m] > 'f'))
					break;
			if (m == i + 2)
				eprintf("%s: invalid escape sequence '\\%c'\n", argv0, s[i + 1]);
			off += m - i - 1;
			for (--m, q = 0, factor = 1; m > i + 1; m--) {
				if (s[m] >= '0' && s[m] <= '9')
					q += (s[m] - '0') * factor;
				else if (s[m] >= 'A' && s[m] <= 'F')
					q += ((s[m] - 'A') + 10) * factor;
				else if (s[m] >= 'a' && s[m] <= 'f')
					q += ((s[m] - 'a') + 10) * factor;
				factor *= 16;
			}
			s[i] = q;
			break;
		case '\0':
			eprintf("%s: null escape sequence\n", argv0);
		default:
			/* "\O[OOO]" octal escape */
			for (m = i + 1; m < i + 1 + 4 && m < len; m++)
				if (s[m] < '0' || s[m] > '7')
					break;
			if (m == i + 1)
				eprintf("%s: invalid escape sequence '\\%c'\n", argv0, s[i + 1]);
			off += m - i - 1;
			for (--m, q = 0, factor = 1; m > i; m--) {
				q += (s[m] - '0') * factor;
				factor *= 8;
			}
			s[i] = (q > 255) ? 255 : q;
		}

		for (m = i + 1; m <= len - off; m++)
			s[m] = s[m + off];
		len -= off;
	}

	return len;
}
