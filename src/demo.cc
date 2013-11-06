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
                ss << q.name;
        } else {

                ss << std::setw(4) << q.offset << " " << std::setw(depth) << "" << q.name;
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

static std::string q_string(const predicate_quantifier& x) {

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

#define RULE(X)         { rule() << X }
#define RULES(...)      rule::singletons({ __VA_ARGS__ })
#define LITERAL(X,Y)    X[Y] = RULE(Y)
#define ESCAPED(X,Y)    X[Y] = RULE("\\" + Y)
#define DISCARD(X)      X << predicate_modifier::discard
#define PUSH(X)         X << predicate_modifier::push
#define LIFT(X)         X << predicate_modifier::lift

static void define_p_grammar(grammar& z) {

        rule::default_type = rule_type::recursive;

        //
        // recursive rules
        //

        z["document"]           = RULE("statement" << q::star << DISCARD("eof"));

        z["statement"]          = RULE(DISCARD("*") << "command" << DISCARD("eol"));

        z["command"]            = RULES("comment", "typedef", "funcdef", "funcdecl");

        z["typedef"]            = RULE("name" << DISCARD(".") << DISCARD("~") << DISCARD(".") << "type");
        z["funcdef"]            = RULE("name" << DISCARD(".") << DISCARD("<-") << DISCARD(".") << "func");
        z["funcdecl"]           = RULE("name" << DISCARD(".") << DISCARD(":") << DISCARD(".") << "type");

        z["type"]               = RULES("signature", "name");

        z["name"]               = { RULE("token" << DISCARD("-") << LIFT("name")), RULE("token") };

        z["token"]              = RULES("abstraction","symbol");

        z["abstraction"]        = RULES("[X]", "{X}", "<X>", "/X/", "\\X\\");

        z["[X]"]                = RULE(DISCARD("[") << "name" << DISCARD("]"));
        z["{X}"]                = RULE(DISCARD("{") << "name" << DISCARD("}"));
        z["<X>"]                = RULE(DISCARD("<") << "name" << DISCARD(">"));
        z["/X/"]                = RULE(DISCARD("/") << "name" << DISCARD("/"));
        z["\\X\\"]              = RULE(DISCARD("\\") << "name" << DISCARD("\\"));

        z["fullname"]           = { RULE("name" << DISCARD("_") << LIFT("fullname")), RULE("name") };

        z["signature"]          = RULE("fullname" << DISCARD(".") << DISCARD("->") << DISCARD(".") << "name");
        z["reference"]          = RULE(DISCARD("@") << "number" << q::question);

        z["func"]               = { RULE("expr" << DISCARD(".") << DISCARD(",") << DISCARD(".") << LIFT("func")), RULE("expr") };
        z["expr"]               = { RULE("call" << DISCARD(".") << DISCARD("|") << DISCARD(".") << LIFT("expr")), RULE("call") };
        z["call"]               = { RULE("name" << DISCARD("_") << "parameters"), RULE("name") };
        z["parameters"]         = { RULE("parameter" << DISCARD("_") << LIFT("parameters")), RULE("parameter") };

        z["parameter"]          = RULES("name", "literal", "reference", "integer");
        z["literal"]            = RULES("literal1", "literal2");
        z["integer"]            = RULES("number","hex","oct","dec","bin");

        rule::default_type = rule_type::terminal;

        //
        // terminal rules
        //

        z["."]          = RULE("[ \\t]*");
        z["_"]          = RULE("[ \\t]+");

        z["*"]          = RULE("\\s*");

        z["eol"]        = RULE("\\s*($|\\z)");

        z["eof"]        = RULE("\\z");

        z["comment"]    = RULE("#[^\\n]*");
        z["symbol"]     = RULE("[[:alpha:]][[:alnum:]]*");

        z["literal1"]   = RULE("\"(\\\\\\\\|\\\\\"|[^\"])*\"");
        z["literal2"]   = RULE("\\((\\\\\\\\|\\\\\\)|[^)])*\\)");

        z["number"]     = RULE("[-+]?(0|[1-9]\\d*)\\b");

        z["hex"]        = RULE("0[xX][[:xdigit:]]+\\b");
        z["oct"]        = RULE("0[oO][0-7]+\\b");
        z["dec"]        = RULE("0[dD]\\d+\\b");
        z["bin"]        = RULE("0[bB][01]+\\b");

        //
        // simple terminal rules
        //

        const std::list<std::string> escapes = { "\\", "/", "{", "}", "[", "]", "|", "~" };

        const std::list<std::string> literals = { "<", ">", "<-", "->", ":", ",", "@", "-" };

        for(auto escape : escapes)
                ESCAPED(z, escape);

        for(auto literal : literals)
                LITERAL(z, literal);
}

#undef RULE
#undef RULES
#undef LITERAL
#undef ESCAPED
#undef DISCARD
#undef LIFT
#undef PUSH

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
                                ss << ' ' << z.name << q_string(z.quantifier);

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

        // std::locale::global(std::locale("en_US.UTF-8"));

        define_p_grammar(g);

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

                ast q(g, fp);

                fclose(fp);

                std::cout << ast_string(q);
        }

        return 0;
}
