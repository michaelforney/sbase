/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <libgen.h>

#include "util.h"

char afmt[4096];

char *
getalignedwidthfmt(char *fmt, char *starts, char *steps, char *ends)
{
  char starttmp[4096], steptmp[4096], endtmp[4096];
  char *startdelim, *stepdelim, *enddelim;
  int startl, startr, stepr, endl, endr;
  int maxl, maxr;

  /*
   * 1.) Get a temp buffer we can work in.
   * 2.) Calculate the *r(ight) and *l(eft) size.
   */
  memmove(starttmp, starts, strlen(starts)+1);
  startdelim = strchr(starttmp, '.');
  if (startdelim != NULL) {
    startdelim[0] = '\0';
    startr = strlen(startdelim+1);
  } else {
    startr = 0;
  }
  startl = strlen(starttmp);
  if (starttmp[0] == '+')
    startl--;

  memmove(endtmp, ends, strlen(ends)+1);
  enddelim = strchr(endtmp, '.');
  if (enddelim != NULL) {
    enddelim[0] = '\0';
    endr = strlen(enddelim+1);
  } else {
    endr = 0;
  }
  endl = strlen(endtmp);
  if (endtmp[0] == '+')
    endl--;

  /*
   * We do not care for the left length of the step, because it
   * will never be displayed.
   */
  memmove(steptmp, steps, strlen(steps)+1);
  stepdelim = strchr(steptmp, '.');
  if (stepdelim != NULL) {
    stepdelim[0] = '\0';
    stepr = strlen(stepdelim+1);
  } else {
    stepr = 0;
  }

  maxl = (startl > endl)? startl : endl;
  maxr = (startl > endr)? startr : endr;
  if (stepr > maxr)
    maxr = stepr;

  if (maxl <= 1) {
    snprintf(afmt, sizeof(afmt), "%%.%df", maxr);
  } else if (maxr == 0) {
    snprintf(afmt, sizeof(afmt), "%%0%d.f", maxl);
  } else {
    snprintf(afmt, sizeof(afmt), "%%0%d.%df", maxl+maxr+1, maxr);
  }

  return afmt;
}

int
main(int argc, char *argv[])
{
  char c, *fmt, *sep, *starts, *steps, *ends;
  bool wflag = false;
  double start, step, end, out;

  sep = "\n";
  fmt = "%G";

  starts = "1";
  steps = "1";
  ends = "1";

  while((c = getopt(argc, argv, "f:s:w")) != -1) {
    switch(c) {
      case 'f':
        fmt = optarg;
        break;
      case 's':
        sep = optarg;
        break;
      case 'w':
        wflag = true;
        break;
    }
  }

  switch(argc-optind) {
    case 3:
      starts = argv[optind++];
      steps = argv[optind++];
      ends = argv[optind++];
      break;
    case 2:
      starts = argv[optind++];
      ends = argv[optind++];
      break;
    case 1:
      ends = argv[optind++];
      break;
    default:
      eprintf("usage: %s [-w] [-f fmt] [-s separator] [start [step]]"
		     " end\n", basename(argv[0]));
  }

  start = atof(starts);
  step = atof(steps);
  end = atof(ends);

  if (step == 0)
    return EXIT_FAILURE;

  if (start > end) {
    if (step > 0)
      return EXIT_FAILURE;
  } else if (start < end) {
    if (step < 0)
      return EXIT_FAILURE;
  }

  if (wflag)
    fmt = getalignedwidthfmt(fmt, starts, steps, ends);
  printf("%s\n", fmt);

  for (out = start;;) {
    printf(fmt, out);

    out += step;
    if (start > end) {
      if (out >= end) {
        printf("%s", sep);
      } else {
        break;
      }
    } else if (start < end) {
      if (out <= end) {
        printf("%s", sep);
      } else {
        break;
      }
    } else {
      break;
    }
  }
  printf("\n");

  return EXIT_SUCCESS;
}

