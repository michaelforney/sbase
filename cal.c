/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "util.h"

#define MONTHMAX 100

static void drawcal(int, int, int, int, int);
static int dayofweek(int, int, int);
static bool isleap(int);
static void usage(void);

static void
drawcal(int year, int month, int day, int ncols, int nmons)
{
	char str[21];
	int count[MONTHMAX];
	int d, i, r;
	int moff, yoff, cur, last, ndays, day1;
	char *smon[]= {
		"    January", "    February", "     March",
		"     April", "      May", "      June",
		"      July", "     August", "   September",
		"    October", "    November", "    December" };
	int mdays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int row = 0;

	while (nmons > 0) {
		last = MIN(nmons, ncols);
		for (i = 0; i < last; i++) {
			moff = month + ncols * row + i - 1;
			cur = moff % 12;
			yoff = year + moff / 12;

			sprintf(str, "%s %d", smon[cur], yoff);
			printf("%-20s   ", str);
			count[i] = 1;
		}
		printf("\n");

		for (i = 0; i < last; i++)
			printf("Su  M Tu  W Th  F Sa   ");
		printf("\n");

		for (r = 0; r < 6; r++) {
			for (i = 0; i < last; i++) {
				moff = month + ncols * row + i - 1;
				cur = moff % 12;
				yoff = year + moff / 12;

				ndays = mdays[cur] + ((cur == 1) & isleap(yoff));
				day1 = dayofweek(year, cur, 1);

				for (d = 0; d < 7; d++)
					if ((r || d >= day1) && count[i] <= ndays)
						printf("%2d ", count[i]++);
					else
						printf("   ");
				printf("  ");
			}
			printf("\n");
		}
		nmons -= ncols;
		row++;
	}
}

static void
defaults(int *year, int *month, int *day, int *ncols, int *nmons)
{
	time_t now;
	struct tm *ltime;

	now = time(NULL);
	ltime = localtime(&now);
	*year = ltime->tm_year + 1900;
	*month = ltime->tm_mon + 1;
	*day = ltime->tm_mday;

	*ncols = 3;
	*nmons = 1;
}

static int
dayofweek(int year, int month, int day)
{
	int a, y, m;

	month++;
	a = (14 - month) / 12;
	y = year + 4800 - a;
	m = month + 12 * a -3;
	return (day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045 + 1) % 7;
}

static bool
isleap(int year)
{
	bool leap = false;

	if (year % 4   == 0) leap = true;
	if (year % 100 == 0) leap = false;
	if (year % 400 == 0) leap = true;
	return leap;
}


static void
usage(void)
{
    eprintf("usage: %s [-c columns] [-m month] [-n number] [-y year]\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	int year, month, day, ncols, nmons;

	defaults(&year, &month, &day, &ncols, &nmons);

	ARGBEGIN {
	case 'c':
		ncols = (int) estrtol(EARGF(usage()), 0);
		break;
	case 'm':
		month = (int) estrtol(EARGF(usage()), 0);
		break;
	case 'n':
		nmons = (int) estrtol(EARGF(usage()), 0);
		break;
	case 'y':
		year = (int) estrtol(EARGF(usage()), 0);
		break;
	default:
		usage();
	} ARGEND;

	if (ncols < 0 || ncols > MONTHMAX || month < 1 || month > 12 || nmons < 1 || year > 9999)
		usage();

	drawcal(year, month, day, ncols, nmons);
	exit(0);
}

