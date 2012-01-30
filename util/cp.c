/* See LICENSE file for copyright and license details. */
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../fs.h"
#include "../text.h"
#include "../util.h"

bool cp_rflag = false;

int
cp(const char *s1, const char *s2)
{
	FILE *f1, *f2;
	char *ns1, *ns2;
	long size1, size2;
	struct dirent *d;
	struct stat st;
	DIR *dp;

	if (stat(s1, &st) == 0 && S_ISDIR(st.st_mode)) {
		if (!cp_rflag) {
			eprintf("%s: is a directory\n", s1);
		}
		else {
			if(!(dp = opendir(s1)))
				eprintf("opendir %s:", s1);
			if (mkdir(s2, st.st_mode) == -1 && errno != EEXIST)
				eprintf("mkdir %s:", s2);
			apathmax(&ns1, &size1);
			apathmax(&ns2, &size2);
			while((d = readdir(dp)))
				if(strcmp(d->d_name, ".") && strcmp(d->d_name, "..")) {
					if(snprintf(ns1, size1, "%s/%s", s1, d->d_name) > size1)
						eprintf("%s/%s: filename too long\n", s1, d->d_name);
					if(snprintf(ns2, size2, "%s/%s", s2, d->d_name) > size2)
						eprintf("%s/%s: filename too long\n", s2, d->d_name);
					fnck(ns1, ns2, cp);
				}
			closedir(dp);
			free(ns1);
			free(ns2);
		}
		return 0;
	}
	if(!(f1 = fopen(s1, "r")))
		eprintf("fopen %s:", s1);
	if(!(f2 = fopen(s2, "w")))
		eprintf("fopen %s:", s2);
	concat(f1, s1, f2, s2);
	fclose(f2);
	fclose(f1);
	return 0;
}
