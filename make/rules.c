#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "make.h"

#define TABSIZ 128
#define FORCE   1
#define NOFORCE 0

static Target *htab[TABSIZ], *deftarget;

void
dumprules(void)
{
	int i;
	Target **pp, **q, *p;

	for (pp = htab; pp < &htab[TABSIZ]; ++pp) {
		for (p = *pp; p; p = p->next) {
			if (!p->defined)
				continue;
			printf("%s:", p->name);
			for (q = p->deps; q && *q; ++q)
				printf(" %s", (*q)->name);
			putchar('\n');
			for (i = 0; i < p->nactions; i++)
				printf("\t%s\n", p->actions[i].line);
			putchar('\n');
		}
	}
}

static Target *
lookup(char *name)
{
	Target *tp;
	int h = hash(name) & TABSIZ-1;

	for (tp = htab[h]; tp && strcmp(tp->name, name); tp = tp->next)
		;

	if (tp)
		return tp;

	tp = emalloc(sizeof(*tp));
	tp->name = estrdup(name);
	tp->target = tp->name;
	tp->req = NULL;
	tp->ndeps = 0;
	tp->deps = NULL;
	tp->actions = NULL;
	tp->nactions = 0;
	tp->next = htab[h];
	tp->defined = 0;
	htab[h] = tp;

	return tp;
}

static void
cleanup(Target *tp)
{
	int sig, precious;
	Target *p, **q;

	sig = stop;
	printf("make: signal %d arrived\n", sig);

	precious = 0;
	p = lookup(".PRECIOUS");
	for (q = p->deps; q && *q; q++) {
		if (strcmp((*q)->name, tp->name) == 0) {
			precious = 1;
			break;
		}
	}

	if (!precious && !nflag && !qflag && !is_dir(tp->name)) {
		printf("make: trying to remove target %s\n", tp->name);
		remove(tp->name);
	}

	signal(sig, SIG_DFL);
	raise(sig);
}

static int
depends(char *target, char *dep)
{
	int i;
	Target **p, *tp = lookup(target);

	for (p = tp->deps; p && *p; ++p) {
		if (strcmp((*p)->name, target) == 0)
			return 1;
	}

	return 0;
}

static int
is_suffix(char *s)
{
	int n;

	if (s[0] != '.')
		return 0;

	for (n = 0; s = strchr(s, '.'); n++)
		s++;

	return n == 2;
}

void
addtarget(char *target, int ndeps)
{
	Target *tp = lookup(target);

	tp->defined = 1;
	if (!deftarget && target[0] != '.') {
		deftarget = tp;
		return;
	}

	if (strcmp(target, ".SUFFIXES") == 0 && ndeps == 0) {
		free(tp->deps);
		tp->deps = NULL;
		tp->ndeps = 0;
		return;
	}

	if (strcmp(target, ".DEFAULT") == 0) {
		if (ndeps > 0)
			error("DEFAULT rule with prerequisites");
		return;
	}

	if (strcmp(target, ".SILENT") == 0 && ndeps == 0) {
		sflag = 1;
		return;
	}

	if (strcmp(target, ".IGNORE") == 0 && ndeps == 0) {
		iflag = 1;
		return;
	}
}

void
adddep(char *target, char *dep)
{
	int i;
	size_t siz;
	Target **p, *tp = lookup(target);

	if (depends(dep, target)) {
		warning("circular dependency %s <- %s dropped", target, dep);
		return;
	}

	for (p = tp->deps; p && *p; ++p) {
		if (strcmp((*p)->name, dep) == 0)
			return;
	}

	tp->ndeps++;
	siz = (tp->ndeps + 1) * sizeof(Target *);
	tp->deps = erealloc(tp->deps, siz);
	tp->deps[tp->ndeps-1] = lookup(dep);
	tp->deps[tp->ndeps] = NULL;

	debug("adding dependency %s <- %s", target, dep);
}

static void
freeaction(struct action *act)
{
	free(act->line);
	freeloc(&act->loc);
}

void
addrule(char *target, struct action  *acts, int n)
{
	int i;
	struct action *v;
	Target *tp = lookup(target);

	debug("adding actions for target %s", target);

	if (tp->actions) {
		debug("overring actions of target %s", target);
		for (i = 0; i < tp->nactions; i++)
			freeaction(&tp->actions[i]);
		free(tp->actions);
	}

	v = emalloc(n * sizeof(*v));
	for (i = 0; i < n; i++) {
		v[i].line = estrdup(acts[i].line);
		v[i].loc.lineno = acts[i].loc.lineno;
		v[i].loc.fname = estrdup(acts[i].loc.fname);
	}

	tp->nactions = n;
	tp->actions = v;
}

static int
execline(Target *tp, char *line, int ignore, int silence)
{
	char *s, *t;
	Target *p, **q;
	int r, at, plus, minus, l;

	debug("executing '%s'", line);

	at = plus = minus = 0;
	for (s = line; ; s++) {
		switch (*s) {
		case '@':
			at = 1;
			break;
		case '-':
			minus = 1;
			break;
		case '+':
			plus = 1;
			break;
		default:
			goto out_loop;
		}
	}

out_loop:
	/* unescape $$ */
	for (l = strlen(s)+1, t = s; *t; --l, ++t) {
		if (t[0] == '$' && t[1] == '$') {
			memmove(t+1, t+2, l-2);
			l--;
		}
	}

	if (tflag && !plus)
		return 0;

	if (sflag || silence || (qflag && !plus))
		at = 1;
	if (nflag)
		at = 0;
	if (!at) {
		puts(s);
		fflush(stdout);
	}

	if ((nflag || qflag) && !plus) {
		if (qflag)
			exitstatus = 1;
		return 0;
	}

	if (minus || iflag || ignore)
		ignore = 1;

	r = launch(s, ignore);
	if (ignore)
		return 0;

	return r;
}

static int
touch(char *name, int ignore, int silence)
{
	char *cmd;
	int r, n;

	n = snprintf(NULL, 0, "touch %s", name) + 1;
	cmd = emalloc(n);
	snprintf(cmd, n, "touch %s", name);

	if (!sflag && !silence)
		puts(cmd);

	r = system(cmd);
	free(cmd);

	if (ignore || iflag)
		return 0;

	return r;
}

static int
touchdeps(Target *tp, int ignore, int silent)
{
	int r;
	Target **p;

	if (tp->req) {
		r = touch(tp->req, silent, ignore);
		if (r)
			return r;
	}

	for (p = tp->deps; p && *p; ++p) {
		r = touch((*p)->name, silent, ignore);
		if (r)
			return r;
	}

	return 0;
}

static int
run(Target *tp)
{
	int r, i, ignore, silent;
	char *s;
	Target *p, **q;

	silent = 0;
	p = lookup(".SILENT");
	for (q = p->deps; q && *q; ++q) {
		if (strcmp((*q)->name, tp->name) == 0) {
			debug("target %s error silent by .SILENT", tp->name);
			silent = 1;
		}
	}

	ignore = 0;
	p = lookup(".IGNORE");
	for (q = p->deps; q && *q; ++q) {
		if (strcmp((*q)->name, tp->name) == 0) {
			debug("target %s error ignored by .IGNORE", tp->name);
			ignore = 1;
		}
	}

	if (tflag) {
		r = touchdeps(tp, ignore, silent);
		if (r)
			return r;
	}

	for (i = 0; i < tp->nactions; i++) {
		struct action *p;

		if (stop)
			cleanup(tp);

		p = &tp->actions[i];
		debug("executing action '%s'", p->line);
		s = expandstring(p->line, tp, &p->loc);
		r = execline(tp, s, ignore, silent);
		free(s);

		if (r)
			return r;
	}

	if (tflag) {
		r = touch(tp->target, ignore, silent);
		if (r)
			return r;
	}

	return 0;
}

static int
enabled(char *suffix)
{
	Target **p, *tp = lookup(".SUFFIXES");

	for (p = tp->deps; p && *p; ++p) {
		if (strcmp(suffix, (*p)->name) == 0)
			return 1;
	}

	return 0;
}

static Target *
inference(Target *tp, int force)
{
	time_t t;
	int tolen, r;
	char *to, *from;
	Target *q, **p, *suffixes;
	char buf[FILENAME_MAX], fname[FILENAME_MAX];

	debug("searching an inference rule for %s", tp->name);

	to = strrchr(tp->name, '.');
	if (to && !enabled(to))
		return NULL;
	tolen = to ? to - tp->name : strlen(tp->name);

	if (!to)
		to = "";

	suffixes = lookup(".SUFFIXES");
	for (p = suffixes->deps; p && *p; ++p) {
		from = (*p)->name;
		debug("trying suffix %s", from);

		r = snprintf(buf,
		             sizeof(buf),
		             "%s%s",
		             from, to);

		if (r < 0 || r >= sizeof(buf))
			error("suffixes too long %s %s", from, to);

		q = lookup(buf);
		if (!q->actions)
			continue;

		r = snprintf(fname,
		             sizeof(fname),
		             "%*.*s%s",
		             tolen, tolen, tp->name, from);

		if (r < 0 || r >= sizeof(fname)) {
			error("prerequisite name too long %s %s",
			      tp->name, from);
		}

		debug("\tsearching prerequisite %s", fname);

		t = stamp(fname);
		if (t == -1) {
			debug("\tprerequisite %s not found", fname);
			continue;
		}

		if (!force && t <= tp->stamp) {
			debug("\tdiscarded because is newer");
			debug("\t%s: %s", tp->name, ctime(&tp->stamp));
			debug("\t%s: %s", fname, ctime(&t));
			continue;
		}

		free(q->req);
		q->req = estrdup(fname);
		q->deps = tp->deps;
		q->target = tp->name;
		q->stamp = tp->stamp;

		debug("using inference rule %s with %s", q->name, fname);
		return q;
	}

	return NULL;
}

static int
update(Target *tp)
{
	Target *p;

	debug("%s needs to be updated", tp->name);

	if (tp->actions) {
		debug("using target rule to build %s", tp->name);
		return run(tp);
	}

	if ((p = inference(tp, FORCE)) != NULL) {
		debug("using inference rule %s", p->name);
		return run(p);
	}

	p = lookup(".DEFAULT");
	if (p->defined) {
		debug("using default rule");
		return run(p);
	}

	debug("not rule found to update %s", tp->name);

	if (!tp->defined)
		error("don't know how to make %s", tp->name);

	return 0;
}

static int 
rebuild(Target *tp, int *buildp)
{
	Target **p, *q;;
	int r, need, build, err, def;

	debug("checking rebuild of %s", tp->name);

	tp->stamp = stamp(tp->name);

	def = err = need = 0;
	for (p = tp->deps; p && *p; ++p) {
		if (stop)
			cleanup(tp);

		q = *p;
		debug("checking dependency %s", q->name);

		if (strcmp(q->name, tp->name) == 0 && q->actions)
			def = 1;

		build = 0;
		if (rebuild(q, &build) != 0) {
			err = 1;
			continue;
		}

		if (build) {
			debug("rebuild of %s forces rebuild of %s",
			       q->name, tp->name);
			need = 1;
		} else if (q->stamp > tp->stamp) {
			debug("dependency %s is newer than %s",
			      q->name, tp->name);
			need = 1;
		}
	}

	if (tp->stamp == -1) {
		need = 1;
	} else if (!def)  {
		debug("no action found for %s, looking a inference rule",
		      tp->name);
		if (inference(tp, NOFORCE))
			need = 1;
	}

	if (err) {
		warning("target %s not remade because of errors", tp->name);
		return 1;
	} else if (need) {
		*buildp = 1;

		debug("target %s needs updating", tp->name);
		r = update(tp);
		if (r == 0)
			return 0;

		if (stop)
			cleanup(tp);

		exitstatus = 1;

		if (!kflag)
			error("target %s: error %d", tp->name, r);
		else
			warning("target %s: error %d", tp->name, r);
		return r;
	}

	return 0;
}

int
build(char *name)
{
	int build, r;

	if (!name) {
		if (!deftarget) {
			printf("make: no target to make\n");
			return 0;
		}
		name = deftarget->name;
	}

	debug("checking target %s", name);

	build = 0;
	return rebuild(lookup(name), &build);
}
