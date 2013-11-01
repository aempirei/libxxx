#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>
#include <cassert>

#include <list>
#include <iostream>
#include <thread>

#include <unistd.h>

#include <liblang.hh>

template <typename F> static void each_line(FILE *fp, F fn) {

	char line[1024];

	while(fgets(line, sizeof(line) - 1, fp) != NULL) {
		char *p = strpbrk(line, "\r\n");
		if(p != NULL)
			*p = '\0';
		fn(line);
	}
}


static void usage(const char *arg0) {
	fprintf(stderr, "\nusage: %s [options] < input\n\n", arg0);
}

int main(int argc, char **argv) {

	if(argc < 2) {
		usage(*argv);
		return -1;
	}

	setvbuf(stderr, NULL, _IONBF, 0);

	std::cout << "argument count: " << argc << std::endl;
	std::cout << "argument 1: " << argv[1] << std::endl;

	return 0;
}
