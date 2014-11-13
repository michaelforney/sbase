/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "util.h"

#define MONTHMAX 100

static void drawcal(int, int, int, int, int, int);
static int dayofweek(int, int, int, int);
static int isleap(int);
static void usage(void);

static void
drawcal(int year, int month, int day, int ncols, int nmons, int fday)
{
	char str[21];
	int count[MONTHMAX];
	int d, i, r, j;
	int moff, yoff, cur, last, ndays, day1;
	char *smon[] = {
		"    January", "    February", "     March",
		"     April", "      May", "      June",
		"      July", "     August", "   September",
		"    October", "    November", "    December" };
	int mdays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int row = 0;
	char *days[] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa", };

	if (!ncols)
		ncols = nmons;
	while (nmons > 0) {
		last = MIN(nmons, ncols);
		for (i = 0; i < last; i++) {
			moff = month + ncols * row + i - 1;
			cur = moff % 12;
			yoff = year + moff / 12;

			snprintf(str, sizeof(str), "%s %d", smon[cur], yoff);
			printf("%-20s   ", str);
			count[i] = 1;
		}
		printf("\n");

		for (i = 0; i < last; i++) {
			for (j = fday; j < LEN(days); j++)
				printf("%s ", days[j]);
			for (j = 0; j < fday; j++)
				printf("%s ", days[j]);
			printf("  ");
		}
		printf("\n");

		for (r = 0; r < 6; r++) {
			for (i = 0; i < last; i++) {
				moff = month + ncols * row + i - 1;
				cur = moff % 12;
				yoff = year + moff / 12;

				ndays = mdays[cur] + ((cur == 1) && isleap(yoff));
				day1 = dayofweek(year, cur, 1, fday);

				for (d = 0; d < 7; d++) {
					if ((r || d >= day1) && count[i] <= ndays)
						printf("%2d ", count[i]++);
					else
						printf("   ");
				}
				printf("  ");
			}
			printf("\n");
		}
		nmons -= ncols;
		row++;
	}
}

static int
dayofweek(int year, int month, int day, int fday)
{
	static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

	day += 7 - fday;
	year -= month < 2;
	return (year + year / 4 - year / 100 + year / 400 + t[month] + day) % 7;
}

static int
isleap(int year)
{
	if (year % 400 == 0)
		return 1;
	if (year % 100 == 0)
		return 0;
	return (year % 4 == 0);
}

static void
usage(void)
{
	eprintf("usage: %s [-1] [-3] [-m] [-s] [-y] [-c columns]"
		" [-f firstday] [-n nmonths] [ [ [day] month] year]\n",
			argv0);
}

int
main(int argc, char *argv[])
{
	int year, month, day, ncols, nmons, fday;
	struct tm *ltime;
	time_t now;

	now = time(NULL);
	ltime = localtime(&now);
	year = ltime->tm_year + 1900;
	month = ltime->tm_mon + 1;
	day = ltime->tm_mday;
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
	case 3:
		day = estrtol(argv[0], 0);
		argv++;
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

	if (ncols < 0 || month < 1 || month > 12 || nmons < 1 \
			|| nmons > MONTHMAX || fday < 0 || fday > 6) {
		usage();
	}

	drawcal(year, month, day, ncols, nmons, fday);

	return 0;
}
