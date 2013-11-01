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

	if(argc < 1) {
		usage(*argv);
		return -1;
	}

	grammar plang;

	std::locale::global(std::locale("en_US.UTF-8"));

	plang[""] = { rule::singleton("statement") << q::star };

	plang["statement"] = rule::singletons({ "typedef", "funcdef", "funcdecl", "comment" });

	for(auto x : plang) {
		std::cout << x.first << " := " << std::endl;
	}

	/*

	   std::cout << "B = " << std::endl;

	   std::cout << "string = " << argv[1] << std::endl;
	   std::cout << "regex  = " << argv[2] << std::endl;

	   std::cout << "a is a " << (a.type == rule::terminal ? "regex" : "list") << std::endl;

	   boost::cmatch cm;

	   if(boost::regex_search(argv[1], cm, a.terminal_value)) {
	   std::cout << "regex matches string!" << std::endl;
	   for(auto x : cm)
	   std::cout << "\tmatch: " << x << std::endl;
	   }

	   for(auto x : b.recursive_value)
	   std::cout << '\t' << x.first << ' ' << qstring(x.second) << std::endl;

	 */

	return 0;
}
