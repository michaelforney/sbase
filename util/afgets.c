/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../text.h"
#include "../util.h"

char *
afgets(char **p, size_t *size, FILE *fp)
{
	char buf[BUFSIZ];
	size_t n, len = 0;

	while(fgets(buf, sizeof buf, fp)) {
		len += (n = strlen(buf));
		if(len+1 > *size && !(*p = realloc(*p, len+1)))
			eprintf("realloc:");
		strcpy(&(*p)[len-n], buf);

		if(buf[n-1] == '\n' || feof(fp))
			break;
	}
	return (len > 0) ? *p : NULL;
}
