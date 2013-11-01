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
	fprintf(stderr, "\nusage: %s string regex [options] < input\n\n", arg0);
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

	if(argc < 3) {
		usage(*argv);
		return -1;
	}

	rule a;
	rule b;
	rule c;

	std::locale::global(std::locale("en_US.UTF-8"));

	a << rule::terminal << argv[2];
	b << rule::recursive << "suck-a-dick" << "fuck-you" << q::plus << "dicks" << q::question << "fcks" << q::star << "nl";

	std::cout << "B = " << std::endl;

	std::cout << "string = " << argv[1] << std::endl;
	std::cout << "regex  = " << argv[2] << std::endl;

	std::cout << "a is a " << (a.type == rule::terminal ? "regex" : "list") << std::endl;

	// if(boost::regex_match(argv[1], cm, a.terminal_value)) std::cout << "regex matches string!" << std::endl;

	boost::regex e("(.)",  boost::regex::perl|boost::regex::icase);

	boost::cmatch cm;

	std::cout << "test match: " << boost::regex_match("fuck you", cm, e) << std::endl;

	for(auto x : b.recursive_value)
		std::cout << '\t' << x.first << ' ' << qstring(x.second) << std::endl;

	return 0;
}
