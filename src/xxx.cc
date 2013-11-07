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

#include <unistd.h>

#include <xxx.hh>

using namespace xxx;

#define P(X)            { rule() << X }
#define PS(...)         rule::singletons({ __VA_ARGS__ })
#define D(X)            X << predicate_modifier::discard
#define L(X)            X << predicate_modifier::lift
#define LITERAL(X,Y)    X[Y] = P(Y)
#define ESCAPED(X,Y)    X[Y] = P("\\" + Y)

static void load_dynamic_grammar(grammar& g, const ast& a) {

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
                                                        if(iter->matches[0] == "^") {
                                                                p.modifier = predicate_modifier::lift;
                                                        } else if(iter->matches[0] == "!") {
                                                                p.modifier = predicate_modifier::discard;
                                                        }

                                                        iter++;
                                                } else {
                                                        p.modifier = predicate_modifier::push;
                                                }

                                                if(iter->name == "name") {
                                                        p.name = iter->matches[0];
                                                        iter++;
                                                }

                                                if(iter != d.children.end() && iter->name == "quantifier") {
                                                        if(iter->matches[0] == "*") {
                                                                p.quantifier = q::star;
                                                        } else if(iter->matches[0] == "+") {
                                                                p.quantifier = q::plus;
                                                        } else if(iter->matches[0] == "?") {
                                                                p.quantifier = q::question;
                                                        }
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
}

static void define_peg_grammar(grammar& g) {

        rule::default_type = rule_type::recursive;

        //
        // recursive rules
        //

        g["document"]   = P(L("rule") << q::star << D("eof"));

        g["rule"]       = P(D("ws") << L("rule'") << D("eol"));

        g["rule'"]      = PS("recursive", "terminal");

        g["terminal"]   = P("name" << D("_") << D("~=") << D("_") << "regex");
        g["recursive"]  = P("name" << D("_") << D(":=") << D("_") << "ordered");

        g["ordered"]    = { P("predicates" << D("_") << D("/") << D("_") << L("ordered")), P("predicates") };

        g["predicates"] = { P("predicate" << D("_") << L("predicates")), P("predicate") };

        g["predicate"]  = P("modifier" << q::question << "name" << "quantifier" << q::question);

        //

        rule::default_type = rule_type::terminal;

        //
        // terminal rules
        //

        g["_"]          = P("[ \\t]+");
        g["name"]       = P("[-.\\w]+");
        g["ws"]         = P("\\s*");
        g["eol"]        = P("\\s*($|\\z)");
        g["eof"]        = P("\\z");
        g["modifier"]   = P("[!^]");
        g["quantifier"] = P("[*?+]");
        g["regex"]      = P("/(\\/|[^\\/\\n])*/");

        //
        // simple terminal rules
        //

        const std::list<std::string> escapes = { "^", "*", "?", "+", "/", "~=" };

        const std::list<std::string> literals = { ":=", "!" };

        for(auto escape : escapes)
                ESCAPED(g, escape);

        for(auto literal : literals)
                LITERAL(g, literal);
}

#undef P
#undef PS
#undef D
#undef L
#undef LITERAL
#undef ESCAPED

#define FLAG '\t' << std::left << std::setw(18)

static void usage(const char *arg0) {

        std::cerr << std::endl << "usage: " << arg0 << " [-{hpax}] [-g filename]" << std::endl << std::endl;

        std::cerr << FLAG << "-h" << "print help" << std::endl;
        std::cerr << FLAG << "-p" << "print grammar" << std::endl;
        std::cerr << FLAG << "-a" << "print ast" << std::endl;
        std::cerr << FLAG << "-x" << "print xml" << std::endl;
        std::cerr << FLAG << "-g filename" << "grammar specification" << std::endl;

        std::cerr << std::endl;
}

int main(int argc, char **argv) {

        bool do_print_xml = false;
        bool do_print_grammar = false;
	bool do_load_grammar = false;
	bool do_print_ast = false;

        const char *filename = NULL;

        int opt;

        if(argc == 1) {
                usage(*argv);
                return -1;
        }

        while ((opt = getopt(argc, argv, "xhpag:")) != -1) {
                switch (opt) {
                        case 'g':
				do_load_grammar = true;
				filename = optarg;
				break;
			case 'a':
				do_print_ast = true;
				break;
			case 'x':
				do_print_xml = true;
				break;
			case 'p':
				do_print_grammar = true;
				break;
                        case 'h':
                        case '?':
                        default:
                                usage(*argv);
                                return -1;
                }
        }

        grammar g;

        define_peg_grammar(g);

	if(do_print_grammar) {
		std::cout << grammar_str(g) << std::endl;
	}

        if(do_load_grammar) {

                grammar h;

                FILE *fp = fopen(filename, "r");

                if(!fp) {
                        perror("fopen()");
                        return -1;
                }

                ast a(g, fp);

                fclose(fp);

		if(do_print_ast)
			std::cout << a.str() << std::endl;
                
                load_dynamic_grammar(h, a);

		if(do_print_grammar)
			std::cout << grammar_str(h) << std::endl;

                ast b(h, stdin);

		if(do_print_ast)
			std::cout << b.str() << std::endl;

		if(do_print_xml)
			std::cout << b.xml() << std::endl;
        }

        return 0;
}
