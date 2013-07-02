#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "util.h"

enum {
	/* from <linux/vt.h> */
	VT_ACTIVATE = 0x5606,
	VT_WAITACTIVE = 0x5607,
	/* from <linux/kd.h> */
	KDGKBTYPE = 0x4B33
};

char *vts[] = {
	"/proc/self/fd/0",
	"/dev/console",
	"/dev/tty",
	"/dev/tty0",
};

static void
usage(void)
{
	eprintf("usage: chvt N\n");
}

int
main(int argc, char **argv)
{
	int n, i, fd;
	char c;

	if(argc!=2 || strspn(argv[1], "1234567890") != strlen(argv[1]))
		usage();

	n = atoi(argv[1]);
	for(i = 0; i < LEN(vts); i++) {
		fd = open(vts[i], O_RDONLY);
		if(fd < 1)
			continue;
		c = 0;
		if(ioctl(fd, KDGKBTYPE, &c) == 0)
			goto VTfound;
		close(fd);
	}

	eprintf("chvt: couldn't find a console.\n");
VTfound:
	if(ioctl(fd, VT_ACTIVATE, n) == -1)
		eprintf("chvt: VT_ACTIVATE '%d':", n);
	if(ioctl(fd, VT_WAITACTIVE, n) == -1)
		eprintf("chvt: VT_WAITACTIVE '%d':", n);
	close(fd);

	return 0;
}

