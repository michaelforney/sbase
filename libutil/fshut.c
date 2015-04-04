#include <stdio.h>
#include <stdlib.h>

#include "../util.h"

int fshut(FILE *fp, const char *fname)
{
	int ret = 0;

	if (fflush(fp) && !ret) {
		weprintf("fflush %s:", fname);
		ret = 1;
	}

	if (ferror(fp) && !ret) {
		weprintf("ferror %s:", fname);
		ret = 1;
	}

	if (fclose(fp) && !ret) {
		weprintf("fclose %s:", fname);
		ret = 1;
	}

	return ret;
}

void enfshut(int status, FILE *fp, const char *fname)
{
	if (fshut(fp, fname))
		exit(status);
}

void efshut(FILE *fp, const char *fname)
{
	enfshut(1, fp, fname);
}
