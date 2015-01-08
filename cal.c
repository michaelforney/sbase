/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "util.h"

enum {Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec};
enum caltype {Julian, Gregorian};
enum {TRANS_YEAR = 1752, TRANS_MONTH = Sep, TRANS_DAY = 2};

static int
isleap(int year, enum caltype cal)
{
	if (cal == Gregorian) {
		if (year % 400 == 0)
			return 1;
		if (year % 100 == 0)
			return 0;
		return (year % 4 == 0);
	}
	else { /* cal == Julian */
		return (year % 4 == 0);
	}
}

static int
monthlength(int year, int month, enum caltype cal)
{
	int mdays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	return (month==Feb && isleap(year,cal))  ?  29  :  mdays[month];
}

/* From http://www.tondering.dk/claus/cal/chrweek.php#calcdow */
static int
dayofweek(int year, int month, int dom, enum caltype cal)
{
	int m, y, a;
	month += 1;  /*  in this formula, 1 <= month <= 12  */
	a = (14 - month) / 12;
	y = year - a;
	m = month + 12*a - 2;

	if (cal == Gregorian)
		return (dom + y + y/4 - y/100 + y/400 + (31*m)/12) % 7;
	else  /* cal == Julian */
		return (5 + dom + y + y/4 + (31*m)/12) % 7;
}

static void
printgrid(int year, int month, int fday, int line)
{
	enum caltype cal;
	int trans; /* are we in the transition from Julian to Gregorian? */
	int offset, dom, d=0;

	if (year < TRANS_YEAR || (year == TRANS_YEAR && month <= TRANS_MONTH))
		cal = Julian;
	else
		cal = Gregorian;
	trans = (year == TRANS_YEAR && month == TRANS_MONTH);
	offset = dayofweek(year, month, 1, cal) - fday;
	if (offset < 0)
		offset += 7;
	if (line==1) {
		for ( ; d < offset; ++d)
			printf("   ");
		dom = 1;
	} else {
		dom = 8-offset + (line-2)*7;
		if (trans && !(line==2 && fday==3))
			dom += 11;
	}
	for ( ; d < 7 && dom <= monthlength(year, month, cal); ++d, ++dom) {
		printf("%2d ", dom);
		if (trans && dom==TRANS_DAY)
			dom += 11;
	}
	for ( ; d < 7; ++d)
		printf("   ");
}

static void
drawcal(int year, int month, int ncols, int nmons, int fday)
{
	char *smon[] = {"  January", " February", "    March", "    April",
	                "      May", "     June", "     July", "   August",
	                "September", "  October", " November", " December" };
	char *days[] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa", };
	int m, n, col, cur_year, cur_month, line, dow;

	for (m = 0; m < nmons; ) {
		n = m;
		for (col = 0; m < nmons && col < ncols; ++col, ++m) {
			cur_year = year + m/12;
			cur_month = month + m%12;
			if (cur_month > 11) {
				cur_month -= 12;
				cur_year += 1;
			}
			printf("   %s %d    ", smon[cur_month], cur_year);
			printf("  ");
		}
		printf("\n");
		for (col = 0, m = n; m < nmons && col < ncols; ++col, ++m) {
			for (dow = fday; dow < (fday+7); ++dow)
				printf("%s ", days[dow%7]);
			printf("  ");
		}
		printf("\n");
		for (line=1; line<=6; ++line) {
			for (col=0, m=n; m<nmons && col<ncols; ++col, ++m) {
				cur_year = year + m/12;
				cur_month = month + m%12;
				if (cur_month > 11) {
					cur_month -= 12;
					cur_year += 1;
				}
				printgrid(cur_year, cur_month, fday, line);
				printf("  ");
			}
			printf("\n");
		}
	}
}

static void
usage(void)
{
	eprintf("usage: %s [-1] [-3] [-m] [-s] [-y] [-c columns]"
		" [-f firstday] [-n nmonths] [ [month] year]\n",
			argv0);
}

int
main(int argc, char *argv[])
{
	int year, month, ncols, nmons, fday;
	struct tm *ltime;
	time_t now;

	now = time(NULL);
	ltime = localtime(&now);
	year = ltime->tm_year + 1900;
	month = ltime->tm_mon + 1;
	fday = 0;

	ncols = 3;
	nmons = 1;

	ARGBEGIN {
	case '1':
		nmons = 1;
		break;
	case '3':
		nmons = 3;
		month -= 1;
		if(month == 0) {
			month = 12;
			year--;
		}
		break;
	case 'c':
		ncols = estrtol(EARGF(usage()), 0);
		break;
	case 'f':
		fday = estrtol(EARGF(usage()), 0);
		break;
	case 'm': /* Monday */
		fday = 1;
		break;
	case 'n':
		nmons = estrtol(EARGF(usage()), 0);
		break;
	case 's': /* Sunday */
		fday = 0;
		break;
	case 'y':
		month = 1;
		nmons = 12;
		break;
	default:
		usage();
	} ARGEND;

	switch (argc) {
	case 2:
		month = estrtol(argv[0], 0);
		argv++;
	case 1:
		year = estrtol(argv[0], 0);
		break;
	case 0:
		break;
	default:
		usage();
	}

	if (ncols < 0 || month < 1 || month > 12 || nmons < 1 || fday < 0 || fday > 6) {
		usage();
	}

	drawcal(year, month-1, ncols, nmons, fday);

	return 0;
}
