/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include "util.h"

static void show_stat(const char *file, struct stat *st);

static void
usage(void)
{
	eprintf("usage: %s [-L] [file...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct stat st;
	int i, ret = 0;
	int Lflag = 0;
	int (*fn)(const char *, struct stat *);

	ARGBEGIN {
	case 'L':
		Lflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0) {
		if (fstat(STDIN_FILENO, &st) < 0)
			eprintf("stat <stdin>:");
		show_stat("<stdin>", &st);
	}

	for (i = 0; i < argc; i++) {
		fn = Lflag ? stat : lstat;
		if (fn(argv[i], &st) == -1) {
			fprintf(stderr, "%s %s: ", Lflag ? "stat" : "lstat",
				argv[i]);
			perror(NULL);
			ret = 1;
			continue;
		}
		show_stat(argv[i], &st);
	}

	return ret;
}

static void
show_stat(const char *file, struct stat *st)
{
	char buf[100];

	printf("  File: ‘%s’\n", file);
	printf("  Size: %lu\tBlocks: %lu\tIO Block: %lu\n", (unsigned long)st->st_size,
	       (unsigned long)st->st_blocks, (unsigned long)st->st_blksize);
	printf("Device: %xh/%ud\tInode: %lu\tLinks %lu\n", major(st->st_dev),
	       minor(st->st_dev), (unsigned long)st->st_ino, (unsigned long)st->st_nlink);
	printf("Access: %04o\tUid: %u\tGid: %u\n", st->st_mode & 0777, st->st_uid, st->st_gid);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&st->st_atime));
	printf("Access: %s\n", buf);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&st->st_mtime));
	printf("Modify: %s\n", buf);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&st->st_ctime));
	printf("Change: %s\n", buf);
}
