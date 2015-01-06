#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>
#include <cassert>
#include <cstdarg>

#include <list>
#include <iostream>
#include <iomanip>
#include <thread>
#include <sstream>
#include <algorithm>

extern "C" {
#include <unistd.h>
}

#include <xxx.hh>
#include <xxx-xxx.hh>

using namespace xxx;

static grammar load_dynamic_grammar(const ast& a) {

	grammar g;

	if(a.name == "document") {

		for(const auto& b : a.children) {

			const auto& name = b.children[0].matches[0];

			if(b.name == "terminal") {

				std::string reg = b.children[1].matches[0].substr(1,std::string::npos);

				reg.pop_back();

				g[name] = { rule(rule_type::terminal) << reg };

			} else {

				g[name] = {};

				// ordered

				for(const auto& c : b.children[1].children) {

					// predicates

					rule r(rule_type::recursive);

					for(const auto& d : c.children) {

						// predicate

						predicate p;

						auto iter = d.children.begin();

						if(iter->name == "modifier") {

							/**/ if(iter->matches[0] == "^") p.modifier = predicate_modifier::lift;
							else if(iter->matches[0] == "!") p.modifier = predicate_modifier::discard;
							else if(iter->matches[0] == ">") p.modifier = predicate_modifier::peek;

							iter++;

						} else {
							p.modifier = predicate_modifier::push;
						}

						if(iter->name == "name") {
							p.name = iter->matches[0];
							iter++;
						}

						if(iter != d.children.end() && iter->name == "quantifier") {

							/**/ if(iter->matches[0] == "*") p.quantifier = q::star;
							else if(iter->matches[0] == "+") p.quantifier = q::plus;
							else if(iter->matches[0] == "?") p.quantifier = q::question;

						} else {
							p.quantifier = q::one;
						}

						r << p;
					}

					g[name].push_back(r);
				}
			}
		}
	}

	return g;
}

#define FLAG '\t' << std::left << std::setw(18)

static void usage(const char *arg0) {

    auto usageline = [](char flag, const char *opt, const char *msg) {
        std::cerr << '\t' << std::left << std::setw(18) << '-' << flag;
        if(opt != nullptr) std::cerr << ' ' << opt;
        if(msg != nullptr) std::cerr << ' ' << msg;
        std::cerr << std::endl;
    };

	std::cerr << std::endl << "usage: " << arg0 << " [-{hpax}] [-g filename]" << std::endl << std::endl;

    usageline('h', nullptr   , "show this help"       );
    usageline('p', nullptr   , "display grammar"      );
    usageline('a', nullptr   , "display ast"          );
    usageline('x', nullptr   , "display xml"          );
    usageline('c', nullptr   , "display code"         );
    usageline('i', nullptr   , "parse stdin"          );
    usageline('g', "filename", "grammar specification");

	std::cerr << std::endl;
}

#undef FLAG

int main(int argc, char **argv) {

	bool do_print_code		= false;
	bool do_print_xml		= false;
	bool do_print_grammar	= false;
	bool do_load_grammar	= false;
	bool do_print_ast		= false;
	bool do_parse_input		= false;

	const char *filename = nullptr;

	int opt;

	if(argc == 1) {
		usage(*argv);
		return -1;
	}

	while ((opt = getopt(argc, argv, "achpsixg:")) != -1) {

		switch (opt) {

			case 'g': do_load_grammar	= true; filename = optarg;	break;
			case 'a': do_print_ast		= true;						break;
			case 'x': do_print_xml		= true;						break;
			case 'p': do_print_grammar	= true;						break;
			case 'c': do_print_code		= true;						break;
			case 'i': do_parse_input	= true;						break;

			case 'h':
			case '?':
			default:

					  usage(*argv);
					  return -1;
		}
	}

	grammar g = define_grammar();

	if(do_print_grammar and not do_parse_input)
		std::cout << grammar_str(g);

	if(do_load_grammar) {

		FILE *fp = fopen(filename, "r");
		if(fp == nullptr) {
			perror("fopen()");
			return -1;
		}

		ast a(g, fp);

		fclose(fp);

		if(do_print_ast and not do_parse_input)
			std::cout << a.str() << std::endl;

		if(do_print_code)
			std::cout << a.code();

		grammar h = load_dynamic_grammar(a);

		if(do_parse_input) {

			ast b(h, stdin);

			if(do_print_grammar)
				std::cout << grammar_str(h);

			if(do_print_ast)
				std::cout << b.str() << std::endl;

			if(do_print_xml)
				std::cout << b.xml() << std::endl;
		}
	}

	return 0;
}
