/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../text.h"
#include "../util.h"

ssize_t
agetline(char **p, size_t *size, FILE *fp)
{
	return getline(p, size, fp);
}
