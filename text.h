/* See LICENSE file for copyright and license details. */

struct linebuf {
	char **lines;
	size_t nlines;
	size_t capacity;
};
#define EMPTY_LINEBUF {NULL, 0, 0,}
void getlines(FILE *, struct linebuf *);

void concat(FILE *, const char *, FILE *, const char *);
