#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>
#include <cassert>

#include <list>
#include <iostream>
#include <thread>
#include <sstream>

#include <unistd.h>

#include <liblang.hh>

using namespace lang;

static void usage(const char *arg0) {
	fprintf(stderr, "\nusage: %s [options] < input\n\n", arg0);
}

std::string qstring(const quantifier& x) {

	std::stringstream ss;

	ss << '[';

	if(x.first == x.second) {
		ss << x.first;
	} else {
		if(x.first != 0)
			ss << x.first;
		ss << "...";
		if(x.second != INT_MAX)
			ss << x.second;
	}

	ss << ']';

	return ss.str();
}

int main(int argc, char **argv) {

	if(argc < 1) {
		usage(*argv);
		return -1;
	}

	rule a;
	rule b;
	rule c;

	a << rule::terminal << "suck-a-dick";
	b << rule::recursive << "suck-a-dick" << "fuck-you" << q::plus << "dicks" << q::question << "fcks" << q::star << "nl";

	std::cout << "B = " << std::endl;

	for(auto x : b.recursive_value) {
		std::cout << '\t' << x.first << ' ' << qstring(x.second) << std::endl;
	}

	c << "suck-a-dick";

	return 0;
}
