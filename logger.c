/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#define SYSLOG_NAMES
#include <syslog.h>
#include <unistd.h>

#include "util.h"

int
decodefac(char *fac)
{
	CODE *c;
	int facility = -1;

	for (c = facilitynames; c->c_name; c++)
		if (!strcasecmp(fac, c->c_name))
			facility = c->c_val;
	if (facility == -1)
		eprintf("invalid facility name: %s\n", fac);
	return facility & LOG_FACMASK;
}

int
decodelev(char *lev)
{
	CODE *c;
	int level = -1;

	for (c = prioritynames; c->c_name; c++)
		if (!strcasecmp(lev, c->c_name))
			level = c->c_val;
	if (level == -1)
		eprintf("invalid level name: %s\n", lev);
	return level & LOG_PRIMASK;
}

int
decodepri(char *pri)
{
	char *p;

	if (!(p = strchr(pri, '.')))
		eprintf("invalid priority name: %s\n", pri);
	*p++ = '\0';
	if (!*p)
		eprintf("invalid priority name: %s\n", pri);
	return decodefac(pri) | decodelev(p);
}

static void
usage(void)
{
	eprintf("usage: %s [-is] [-p priority] [-t tag] [message ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	char *buf = NULL, *tag = NULL;
	size_t sz = 0;
	int logflags = 0, priority = LOG_NOTICE;
	int i;

	ARGBEGIN {
	case 'i':
		logflags |= LOG_PID;
		break;
	case 'p':
		priority = decodepri(EARGF(usage()));
		break;
	case 's':
		logflags |= LOG_PERROR;
		break;
	case 't':
		tag = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND;

	openlog(tag ? tag : getlogin(), logflags, 0);

	if (argc == 0) {
		while(getline(&buf, &sz, stdin) != -1)
			syslog(priority, "%s", buf);
		if (ferror(stdin))
			eprintf("%s: read error:", "<stdin>");
	} else {
		for (i = 0; i < argc; i++)
			sz += strlen(argv[i]);
		sz += argc;
		buf = ecalloc(1, sz);
		for (i = 0; i < argc; i++) {
			strlcat(buf, argv[i], sz);
			if (i + 1 < argc)
				strlcat(buf, " ", sz);
		}
		syslog(priority, "%s", buf);
	}
	free(buf);
	closelog();
	return 0;
}
