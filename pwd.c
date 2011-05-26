/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

int
main(void)
{
	puts(agetcwd());
	return EXIT_SUCCESS;
}
