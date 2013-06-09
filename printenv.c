#include <stdio.h>
#include <stdlib.h>

extern char **environ;

int
main(int argc, char **argv)
{
	char *var;

	if(argc == 1) {
		while(*environ)
			printf("%s\n", *environ++);

		return 0;
	}
	while(*++argv) {
		if((var = getenv(*argv)))
			printf("%s\n", var);
	}

	return 0;
}

