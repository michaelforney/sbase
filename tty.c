/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(void)
{
	char *tty;

	if((tty = ttyname(STDIN_FILENO))) {
		puts(tty);
		return 0;
	}
	else if(errno == ENOTTY) {
		puts("not a tty");
		return 1;
	}
	else {
		perror("ttyname");
		return 2;
	}
}
