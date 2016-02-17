/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "util.h"

#define eprintf(...)  enprintf(2, __VA_ARGS__)
#define estrdup(...)  enstrdup(2, __VA_ARGS__)
#define ecalloc(...)  encalloc(2, __VA_ARGS__)
#define efshut(...)   enfshut(2, __VA_ARGS__)

#define WHITE  0
#define GREY   1
#define BLACK  2

struct vertex;

struct edge
{
	struct vertex *to;
	struct edge *next;
};

struct vertex
{
	char *name;
	struct vertex *next;
	struct edge edges;
	size_t in_edges;
	int colour;
};

static struct vertex graph;

static void
find_vertex(const char *name, struct vertex **it, struct vertex **prev)
{
	for (*prev = &graph; (*it = (*prev)->next); *prev = *it) {
		int cmp = strcmp(name, (*it)->name);
		if (cmp > 0)
			continue;
		if (cmp < 0)
			*it = 0;
		return;
	}
}

static void
find_edge(struct vertex* from, const char *to, struct edge **it, struct edge **prev)
{
	for (*prev = &(from->edges); (*it = (*prev)->next); *prev = *it) {
		int cmp = strcmp(to, (*it)->to->name);
		if (cmp > 0)
			continue;
		if (cmp < 0)
			*it = 0;
		return;
	}
}

static struct vertex *
add_vertex(char *name)
{
	struct vertex *vertex;
	struct vertex *prev;

	find_vertex(name, &vertex, &prev);
	if (vertex)
		return vertex;

	vertex = ecalloc(1, sizeof(*vertex));
	vertex->name = name;
	vertex->next = prev->next;
	prev->next = vertex;

	return vertex;
}

static struct edge *
add_edge(struct vertex* from, struct vertex* to)
{
	struct edge *edge;
	struct edge *prev;

	find_edge(from, to->name, &edge, &prev);
	if (edge)
		return edge;

	edge = ecalloc(1, sizeof(*edge));
	edge->to = to;
	edge->next = prev->next;
	prev->next = edge;
	to->in_edges += 1;

	return edge;
}

static void
load_graph(FILE *fp)
{
#define SKIP(VAR, START, FUNC)  for (VAR = START; FUNC(*VAR) && *VAR; VAR++)
#define TOKEN_END(P)            do { if (*P) *P++ = 0; else P = 0; } while (0)

	char *line = 0;
	size_t size = 0;
	ssize_t len;
	char *p;
	char *name;
	struct vertex *from = 0;

	while ((len = getline(&line, &size, fp)) != -1) {
		if (len && line[len - 1] == '\n')
			line[len - 1] = 0;
		for (p = line; p;) {
			SKIP(name, p, isspace);
			if (!*name)
				break;
			SKIP(p, name, !isspace);
			TOKEN_END(p);
			if (!from) {
				from = add_vertex(estrdup(name));
			} else if (strcmp(from->name, name)) {
				add_edge(from, add_vertex(estrdup(name)));
				from = 0;
			} else {
				from = 0;
			}
		}
	}

	free(line);

	if (from)
		eprintf("odd number of tokens in input, did you intended to use -l?\n");
}

static int
sort_graph_visit(struct vertex *u)
{
	struct edge *e = &(u->edges);
	struct vertex *v;
	int r = 0;
	u->colour = GREY;
	printf("%s\n", u->name);
	while ((e = e->next)) {
		v = e->to;
		if (v->colour == WHITE) {
			v->in_edges -= 1;
			if (v->in_edges == 0)
				r |= sort_graph_visit(v);
		} else if (v->colour == GREY) {
			r = 1;
			fprintf(stderr, "%s: loop detected between %s and %s\n",
			        argv0, u->name, v->name);
		}
	}
	u->colour = BLACK;
	return r;
}

static int
sort_graph(void)
{
	struct vertex *u, *prev;
	int r = 0;
	size_t in_edges;
	for (in_edges = 0; graph.next; in_edges++) {
		for (prev = &graph; (u = prev->next); prev = u) {
			if (u->colour != WHITE)
				goto unlist;
			if (u->in_edges > in_edges)
				continue;
			r |= sort_graph_visit(u);
		unlist:
			prev->next = u->next;
			u = prev;
		}
	}
	return r;
}

static void
usage(void)
{
	eprintf("usage: %s [file]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp = stdin;
	const char *fn = "<stdin>";
	int ret = 0;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc > 1)
		usage();
	if (argc && strcmp(*argv, "-"))
		if (!(fp = fopen(fn = *argv, "r")))
			eprintf("fopen %s:", *argv);

	memset(&graph, 0, sizeof(graph));
	load_graph(fp);
	efshut(fp, fn);

	ret = sort_graph();

	if (fshut(stdout, "<stdout>") | fshut(stderr, "<stderr>"))
		ret = 2;

	return ret;
}
