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

	if(x == q::one) {

		return "";

	} else if(x == q::question) {

		return "?";

	} else if(x == q::star) {

		return "*";

	} else if(x == q::plus) {

		return "+";
	}

	std::stringstream ss;

	ss << '{' << x.first;

	if(x.first != x.second) {
		ss << ',';
		if(x.second != INT_MAX)
			ss << x.second;
	}

	ss << '}';

	return ss.str();
}

int main(int argc, char **argv) {

	if(argc < 1) {
		usage(*argv);
		return -1;
	}

	grammar plang;

	std::locale::global(std::locale("en_US.UTF-8"));

	plang["s"] = { rule::recursive("statement") << q::star };

	plang["statement"] = rule::singletons({ "typedef", "funcdef", "funcdecl", "comment" });

	plang["typedef"] = { rule::recursive("name") << "tilde" << "signature" };

	for(auto x : plang) {

		std::cout << x.first << " :=";

		for(auto iter = x.second.begin(); iter != x.second.end(); iter++) {

			auto& y = *iter;

			if(y.type == rule::rule_type::recursive) {

				for(auto z : y.recursive_value)
					std::cout << ' ' << z.first << qstring(z.second);

			} else if(y.type == rule::rule_type::terminal) {

				std::cout << " /regex/" << std::endl;

			} else {

				std::cout << " UNDEFINED" << std::endl;
			}

			if(next(iter) == x.second.end())
				std::cout << std::endl;
			else
				std::cout << " /";
		}
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
