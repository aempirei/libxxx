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

static std::string ast_string(const ast& q, int depth=0, bool basic=false) {

        std::stringstream ss;

        if(basic) {
                ss << q.rulename;
        } else {

                ss << std::setw(4) << q.offset << " " << std::setw(depth) << "" << q.rulename;
        }

        switch(q.type) {

                case rule_type::undefined:

                        ss << " := (UNDEFINED)" << std::endl;
                        break;

                case rule_type::terminal:

                        ss << " =~ " << '"' << q.string << '"' << std::endl;
                        break;

                case rule_type::recursive:

                        if(q.children.size() == 1) {
                                ss << ' ' << ast_string(q.children.back(), depth + 2, true);
                        } else {

                                ss << std::endl;

                                for(const auto& qq : q.children)
                                        ss << ast_string(qq, depth + 2);
                        }

                        break;

                default:
                        ss << " := (UNKNOWN)";
                        break;
        }

        return ss.str();
}

#define P(X)            { rule() << X }
#define PS(...)         rule::singletons({ __VA_ARGS__ })
#define D(X)            X << predicate_modifier::discard
#define L(X)            X << predicate_modifier::lift
#define LITERAL(X,Y)    X[Y] = P(Y)
#define ESCAPED(X,Y)    X[Y] = P("\\" + Y)

static void define_peg_grammar(grammar& z) {

        rule::default_type = rule_type::recursive;

        //
        // recursive rules
        //

        z["document"]   = P("line" << q::star << D("eof"));

        z["line"]       = P(D("ws") << "rule" << D("eol"));

        z["rule"]       = PS("recursive", "terminal");

        z["terminal"]   = P("name" << D("_") << D("~=") << D("_") << "regex");
        z["recursive"]  = P("name" << D("_") << D(":=") << D("_") << "ordered");

        z["ordered"]    = { P("predicates" << D("_") << D("/") << D("_") << L("ordered")), P("predicates") };

        z["predicates"] = { P("predicate" << D("_") << L("predicates")), P("predicate") };

        z["predicate"]  = P("modifier" << q::question << "name" << "quantifier" << q::question);

        //

        rule::default_type = rule_type::terminal;

        //
        // terminal rules
        //

        z["_"]          = P("[ \\t]+");
        z["name"]       = P("[-.\\w]+");
        z["ws"]         = P("\\s*");
        z["eol"]        = P("\\s*($|\\z)");
        z["eof"]        = P("\\z");
        z["modifier"]   = PS("!", "^");
        z["quantifier"] = PS("*", "?", "+");
        z["regex"]      = P("\\/(\\/|[^\\/\\n])*\\/");

        //
        // simple terminal rules
        //

        const std::list<std::string> escapes = { "^", "*", "?", "+", "/", "~=" };

        const std::list<std::string> literals = { ":=", "!" };

        for(auto escape : escapes)
                ESCAPED(z, escape);

        for(auto literal : literals)
                LITERAL(z, literal);
}

#undef P
#undef PS
#undef D
#undef L
#undef LITERAL
#undef ESCAPED

static void usage(const char *arg0) {

        const int w = 18;

        std::cerr << std::endl << "usage: " << arg0 << " [-{h}] [-p filename]" << std::endl << std::endl;

        std::cerr << '\t' << std::left << std::setw(w) << "-h" << "print help" << std::endl;
        std::cerr << '\t' << std::left << std::setw(w) << "-p filename" << "parse file" << std::endl;

        std::cerr << std::endl;
}

int main(int argc, char **argv) {

        bool do_parse = false;

        const char *filename = NULL;

        int opt;

        if(argc == 1) {
                usage(*argv);
                return -1;
        }

        while ((opt = getopt(argc, argv, "hp:")) != -1) {
                switch (opt) {
                        case 'p':
                                do_parse = true;
                                filename = optarg;
                                break;
                        case 'h':
                        case '?':
                        default:
                                usage(*argv);
                                return -1;
                }
        }

        grammar g;
        ast q;

        define_peg_grammar(g);

        if(do_parse) {

                FILE *fp = fopen(filename, "r");

                if(!fp) {
                        perror("fopen()");
                        return -1;
                }

                parse(g, fp, q);

                fclose(fp);

                std::cout << ast_string(q);
        }

        return 0;
}
