/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "util.h"
#include "text.h"

static int uudecode(FILE *, FILE*);
static int checkheader(FILE *, const char *, mode_t *, char **, char **);
static int checkmode(const char *, mode_t *);
static FILE *checkfile(const char *);

static void
usage(void)
{
	eprintf("usage: %s [file]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp, *nfp;
	char *fname, *headerr;
	mode_t mode = 0;
	int chmodtest;

	ARGBEGIN {
	case 'm':
		eprintf("-m not implemented\n");
	default:
		usage();
	} ARGEND;

	if (argc > 1)
		usage();

	if (argc == 0) {
		if (checkheader(stdin, "begin ", &mode, &fname, &headerr) < 0)
			eprintf("%s\n",headerr);

		nfp = checkfile(fname);
		if (nfp == NULL)
			eprintf("fopen %s:", fname);

		if (uudecode(stdin, nfp) < 0)
			goto fail;

	} else {
		if (!(fp = fopen(argv[0], "r")))
			eprintf("fopen %s:", argv[0]);

		if (checkheader(fp, "begin ", &mode, &fname, &headerr) < 0) {
			fclose(fp);
			eprintf("%s\n",headerr);
		}
		nfp = checkfile(fname);
		if (nfp == NULL) {
			fclose(fp);
			eprintf("fopen %s:", fname);
		}
		if (uudecode(fp, nfp) < 0) {
			fclose(fp);
			goto fail;
		}
	}
	chmodtest=fchmod(fileno(nfp),mode);
	fclose(fp);
	fclose(nfp);
	if (chmodtest == -1)
		eprintf("chmod %s:%s",fname,strerror(errno));

	return EXIT_SUCCESS;
fail:
	fclose(nfp);
	return EXIT_FAILURE;
}

static FILE *
checkfile(const char *fname)
{
	struct stat st;
	int ret;
	if (strcmp(fname,"/dev/stdout") == 0)
		return stdout;

	ret = lstat(fname, &st);
	if (ret < 0 && errno == ENOENT)
		goto tropen; /* new file, try to open it */

	if (ret < 0) {
		weprintf("stat: %s:", fname);
		return NULL;
	}

	if (!S_ISREG(st.st_mode)) {
		weprintf("for safety uudecode operates only on regular files and /dev/stdout\n");
		return NULL;
	}
tropen:
	return fopen(fname,"w");
}

static int
checkheader(FILE *fp, const char *header, mode_t *mode, char **fname, char **headerr)
{
	char bufs[PATH_MAX + 11]; /* len header + mode + maxname */
	char *p, *q, *bufp = NULL;
	size_t n;
	fgets(bufs, sizeof(bufs), fp);
	if (bufp == NULL || bufp == '\0') { /* empty or null str */
		*headerr = "empty or null header string";
		return -1;
	}
	if ((p = strchr(bufs, '\n')) == NULL) { /* line too long or last line on file */
		*headerr = "header string too long or non-newline terminated file";
		return -1;
	}
	p = bufs;
	if (strncmp(bufs, header, strlen(header)) != 0) {
		*headerr = "malformed header prefix";
		return -1;
	}
	p += strlen(header);
	if ((q = strchr(p, ' ')) == NULL) { /* malformed mode */
		*headerr = "malformed mode string in header";
		return -1;
	}
	*q++ = '\0'; /* now mode should be null terminated,q points to fname */
	if (checkmode(p,mode) < 0) { /* error from checkmode */
		*headerr = "invalid mode in header";
		return -1;
	}
        n = strlen(q);
        while (n > 0 && (q[n-1] == '\n' || q[n-1] == '\r'))
	        q[--n] = '\0';

        if (n > 0)
		*fname = q;

	return 1;
}


static int
checkmode(const char *str,mode_t *validmode)
{
	char *end;
	int octal;
	if (str == NULL || str == '\0')
		return -1; /* null str */

	octal = strtol(str, &end, 8);
	if (*end == '\0') { /* successful conversion from a valid str */
		if (octal >= 0 && octal <= 07777) {
			if(octal & 04000) *validmode |= S_ISUID;
			if(octal & 02000) *validmode |= S_ISGID;
			if(octal & 01000) *validmode |= S_ISVTX;
			if(octal & 00400) *validmode |= S_IRUSR;
			if(octal & 00200) *validmode |= S_IWUSR;
			if(octal & 00100) *validmode |= S_IXUSR;
			if(octal & 00040) *validmode |= S_IRGRP;
			if(octal & 00020) *validmode |= S_IWGRP;
			if(octal & 00010) *validmode |= S_IXGRP;
			if(octal & 00004) *validmode |= S_IROTH;
			if(octal & 00002) *validmode |= S_IWOTH;
			if(octal & 00001) *validmode |= S_IXOTH;
			*validmode &= 07777;
			return 1;
		}
	}
	return -1; /* malformed mode */
}

static int
uudecode(FILE *fp, FILE *outfp)
{
	char *bufb=NULL, *p,*nl;
	size_t n=0;
	int ch , i;
#define DEC(c)  (((c) - ' ') & 077)             /* single character decode */
#define IS_DEC(c) ( (((c) - ' ') >= 0) && (((c) - ' ') <= 077 + 1) )
#define OUT_OF_RANGE(c) do {						\
		weprintf("character %c out of range: [%d-%d]",(c), 1+' ',077+' '+1); \
		return -1;						\
	} while (0)

	while (afgets(&bufb,&n,fp)) {
		p = bufb;
		if ((nl=strchr(bufb, '\n')) != NULL) { /* trim newlines */
			*nl = '\0';
		} else {
			weprintf("no newline found, aborting\n");
			return -1;
		}
		if ((i = DEC(*p)) <= 0) /* check for last line */
			break;

		for (++p; i > 0; p += 4, i -= 3) {
			if (i >= 3) {
				if (!(IS_DEC(*p) && IS_DEC(*(p + 1)) &&
				      IS_DEC(*(p + 2)) && IS_DEC(*(p + 3))))
					OUT_OF_RANGE(*p);

				ch = DEC(p[0]) << 2 | DEC(p[1]) >> 4;
				putc(ch, outfp);
				ch = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
				putc(ch, outfp);
				ch = DEC(p[2]) << 6 | DEC(p[3]);
				putc(ch, outfp);
			} else {
				if (i >= 1) {
					if (!(IS_DEC(*p) && IS_DEC(*(p + 1))))
						OUT_OF_RANGE(*p);

					ch = DEC(p[0]) << 2 | DEC(p[1]) >> 4;
					putc(ch, outfp);
				}
				if (i >= 2) {
					if (!(IS_DEC(*(p + 1)) &&
					      IS_DEC(*(p + 2))))
						OUT_OF_RANGE(*p);

					ch = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
					putc(ch, outfp);
				}
			}
		}
		if (ferror(fp)) {
			weprintf("read error");
			return -1;
		}
	}
	/* check for end or fail*/
	afgets(&bufb,&n,fp);
	if (strnlen(bufb,3) < 3 || strncmp(bufb, "end", 3) != 0 || bufb[3] != '\n') {
		weprintf("valid uudecode footer \"end\" not found\n");
		return -1;
	}
	return 1;
}
