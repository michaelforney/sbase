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

static void uudecode(FILE *, FILE *);
static void parseheader(FILE *, const char *, const char *, mode_t *, char **);
static void parsemode(const char *, mode_t *);
static FILE *parsefile(const char *);

static void
usage(void)
{
	eprintf("usage: %s [file]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp = NULL, *nfp = NULL;
	char *fname;
	mode_t mode = 0;

	ARGBEGIN {
	case 'm':
		eprintf("-m not implemented\n");
	default:
		usage();
	} ARGEND;

	if (argc > 1)
		usage();

	if (argc == 0) {
		parseheader(stdin, "<stdin>", "begin ", &mode, &fname);
		if ((nfp = parsefile(fname)) == NULL)
			eprintf("fopen %s:", fname);
		uudecode(stdin, nfp);
	} else {
		if ((fp = fopen(argv[0], "r")) == NULL)
			eprintf("fopen %s:", argv[0]);
		parseheader(fp, argv[0], "begin ", &mode, &fname);
		if ((nfp = parsefile(fname)) == NULL)
			eprintf("fopen %s:", fname);
		uudecode(fp, nfp);
	}

	if (chmod(fname, mode) < 0)
		eprintf("chmod %s:", fname);
	if (fp)
		fclose(fp);
	if (nfp)
		fclose(nfp);

	return EXIT_SUCCESS;
}

static FILE *
parsefile(const char *fname)
{
	struct stat st;
	int ret;

	if (strcmp(fname, "/dev/stdout") == 0)
		return stdout;
	ret = lstat(fname, &st);
	/* if it is a new file, try to open it */
	if (ret < 0 && errno == ENOENT)
		goto tropen;
	if (ret < 0) {
		weprintf("lstat %s:", fname);
		return NULL;
	}
	if (!S_ISREG(st.st_mode)) {
		weprintf("for safety uudecode operates only on regular files and /dev/stdout\n");
		return NULL;
	}
tropen:
	return fopen(fname,"w");
}

static void
parseheader(FILE *fp, const char *s, const char *header, mode_t *mode, char **fname)
{
	char bufs[PATH_MAX + 11]; /* len header + mode + maxname */
	char *p, *q;
	size_t n;

	if (fgets(bufs, sizeof(bufs), fp) == NULL)
		if (ferror(fp))
			eprintf("%s: read error:", s);
	if (bufs[0] == '\0' || feof(fp))
		eprintf("empty or nil header string\n");
	if ((p = strchr(bufs, '\n')) == NULL)
		eprintf("header string too long or non-newline terminated file\n");
	p = bufs;
	if (strncmp(bufs, header, strlen(header)) != 0)
		eprintf("malformed header prefix\n");
	p += strlen(header);
	if ((q = strchr(p, ' ')) == NULL)
		eprintf("malformed mode string in header\n");
	*q++ = '\0';
	/* now mode should be null terminated, q points to fname */
	parsemode(p, mode);
        n = strlen(q);
        while (n > 0 && (q[n - 1] == '\n' || q[n - 1] == '\r'))
	        q[--n] = '\0';
        if (n > 0)
		*fname = q;
}

static void
parsemode(const char *str, mode_t *validmode)
{
	char *end;
	int octal;

	if (str == NULL || str == '\0')
		eprintf("invalid mode\n");
	octal = strtol(str, &end, 8);
	if (*end == '\0') {
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
		}
	}
}

static void
uudecode(FILE *fp, FILE *outfp)
{
	char *bufb = NULL, *p, *nl;
	size_t n=0;
	int ch, i;

#define DEC(c)  (((c) - ' ') & 077) /* single character decode */
#define IS_DEC(c) ( (((c) - ' ') >= 0) && (((c) - ' ') <= 077 + 1) )
#define OUT_OF_RANGE(c) do {						\
		eprintf("character %c out of range: [%d-%d]",(c), 1+' ',077+' '+1); \
	} while (0)

	while (afgets(&bufb,&n,fp)) {
		p = bufb;
		/* trim newlines */
		if ((nl = strchr(bufb, '\n')) != NULL)
			*nl = '\0';
		else
			eprintf("no newline found, aborting\n");
		/* check for last line */
		if ((i = DEC(*p)) <= 0)
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
		if (ferror(fp))
			eprintf("read error:");
	}
	/* check for end or fail */
	afgets(&bufb, &n, fp);
	if (strnlen(bufb, 3) < 3 || strncmp(bufb, "end", 3) != 0 || bufb[3] != '\n')
		eprintf("valid uudecode footer \"end\" not found\n");
}
