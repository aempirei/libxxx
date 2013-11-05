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

static std::string q_string(const quantifier& x) {

        if(x == q::one) {

                return "";

        } else if(x == q::question) {

                return "\33[1;34m?\33[0m";

        } else if(x == q::star) {

                return "\33[1;34m*\33[0m";

        } else if(x == q::plus) {

                return "\33[1;34m+\33[0m";
        }

        std::stringstream ss;

        ss << "\33[1;34m{\33[0;34m" << x.first;

        if(x.first != x.second) {
                ss << "\33[1;34m,\33[0;34m";
                if(x.second != INT_MAX)
                        ss << x.second;
        }

        ss << "\33[1;34m}\33[0m";

        return ss.str();
}

#define P(X)            { rule() << X }
#define PS(...)         rule::singletons({ __VA_ARGS__ })
#define LITERAL(X,Y)    X[Y] = P(Y)
#define ESCAPED(X,Y)    X[Y] = P("\\" + Y)
#define D(X)            X << predicate_modifier::discard
#define L(X)            X << predicate_modifier::lift

static void define_peg_grammar(grammar& z) {

        rule::default_type = rule_type::recursive;

        //
        // recursive rules
        //

        z["document"]   = P("line" << q::star << D("eof"));
        z["line"]       = P(D("ws") << "rule" << D("eol"));
        z["terminal"]   = P("name" << D("_") << D("~=") << D("_") << "regex");
        z["recursive"]  = P("name" << D("_") << D(":=") << D("_") << "ordered");
        z["predicate"]  = P("modifier" << q::question << "name" << "quantifier" << q::question);

        z["rule"]       = PS("terminal", "recursive");
        z["modifier"]   = PS("!", "^");
        z["quantifier"] = PS("*", "?", "+");

        z["predicates"] = { P("predicate" << D("_") << L("predicates")), P("predicate") };
        z["ordered"]    = { P("predicates" << D("_") << D("/") << D("_") << L("ordered")), P("predicates") };

        //

        rule::default_type = rule_type::terminal;

        //
        // terminal rules
        //

        z["_"]          = P("[\\s]+");
        z["name"]       = P("[[:alpha:]][[:alnum:]]*");
        z["ws"]         = P("[\\s\\n]*");
        z["eol"]        = P("[\\s]*($|\\z)");
        z["eof"]        = P("\\z");
        z["regex"]      = P("\\/(\\/|[^\\/\\n])*\\/");

        //
        // simple terminal rules
        //

        const std::list<std::string> escapes = { "^", "*", "?", "+", "\\", "/", "{", "}", "[", "]", "~=" };

        const std::list<std::string> literals = { ":=", "!" };

        for(auto escape : escapes)
                ESCAPED(z, escape);

        for(auto literal : literals)
                LITERAL(z, literal);
}

#undef P
#undef PS
#undef LITERAL
#undef ESCAPED
#undef D
#undef L

static int grammar_rule_width(const grammar& g) {

        std::string ms;

        for(const auto& x : g)
                if(x.first.length() > ms.length())
                        ms = x.first;

        return (int)ms.length();
}

static std::string rule_list_string(const std::list<rule>& rs) {

        std::stringstream ss;

        const char yel[] = "\33[1;33m";
        const char brown[] = "\33[0;33m";
        const char red[] = "\33[0;31m";
        const char none[] = "\33[0m";

        for(auto iter = rs.begin(); iter != rs.end(); iter++) {

                auto& y = *iter;

                if(y.type == rule_type::recursive) {

                        for(auto z : y.recursive_value)
                                ss << ' ' << z.first << q_string(z.second);

                } else if(y.type == rule_type::terminal) {

                        ss << ' ' << yel << '/' << brown << y.terminal_value.str() << yel << '/' << none;

                } else {

                        ss << " UNDEFINED" << std::endl;
                }

                if(next(iter) == rs.end())
                        ss << std::endl;
                else
                        ss << ' ' << red << '/' << none;
        }

        return ss.str();
}

static void usage(const char *arg0) {

        const int w = 18;

        std::cerr << std::endl << "usage: " << arg0 << " [-{gh}] [-p filename]" << std::endl << std::endl;

        std::cerr << '\t' << std::left << std::setw(w) << "-g" << "print grammar specification" << std::endl;
        std::cerr << '\t' << std::left << std::setw(w) << "-h" << "print help" << std::endl;
        std::cerr << '\t' << std::left << std::setw(w) << "-p filename" << "parse file" << std::endl;

        std::cerr << std::endl;
}

int main(int argc, char **argv) {

        bool do_grammar = false;
        bool do_parse = false;

        const char *filename = NULL;

        int opt;

        if(argc == 1) {
                usage(*argv);
                return -1;
        }

        while ((opt = getopt(argc, argv, "hgp:")) != -1) {
                switch (opt) {
                        case 'g':
                                do_grammar = true;
                                break;
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

        // std::locale::global(std::locale("en_US.UTF-8"));

        define_peg_grammar(g);

        if(do_grammar) {

                int width = grammar_rule_width(g);

                for(auto x : g)
                        std::cout << std::setw(width) << x.first << ' ' << "\33[1;30m" << ":=" << "\33[0m" << rule_list_string(x.second);
        }

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
