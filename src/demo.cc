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

                        ss << " =~ " << '"' << q.matches[0] << '"' << std::endl;
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

#define ANSI(x)	"\33[" #x "m"	
#define COLOR1	ANSI(1;34)
#define COLOR2	ANSI(0;34)
#define COLOR3	ANSI(1;33)
#define COLOR4	ANSI(0;33)
#define COLOR5	ANSI(1;31)
#define COLOR6	ANSI(0;31)

static std::string q_string(const predicate_quantifier& x) {

        if(x == q::one) {

                return "";

        } else if(x == q::question) {

		return COLOR1 "?" ANSI(0);

        } else if(x == q::star) {

		return COLOR1 "*" ANSI(0);

        } else if(x == q::plus) {

		return COLOR1 "+" ANSI(0);
        }

        std::stringstream ss;

	ss << COLOR1 "{" COLOR2 << x.first;

        if(x.first != x.second) {
		ss << COLOR1 "," COLOR2;
                if(x.second != INT_MAX)
                        ss << x.second;
        }

	ss << COLOR1 "}" ANSI(0);

        return ss.str();
}

#define RULE(X)         { rule() << X }
#define RULES(...)      rule::singletons({ __VA_ARGS__ })
#define LITERAL(X,Y)    X[Y] = RULE(Y)
#define ESCAPED(X,Y)    X[Y] = RULE("\\" + Y)
#define DISCARD(X)      X << predicate_modifier::discard
#define PUSH(X)         X << predicate_modifier::push
#define LIFT(X)         X << predicate_modifier::lift
#define PEEK(X)         X << predicate_modifier::peek

static void define_p_grammar(grammar& g) {

        rule::default_type = rule_type::recursive;

        //
        // recursive rules
        //

        g["document"]           = RULE("statement" << q::star << DISCARD("eof"));

        g["statement"]          = RULE(DISCARD("*") << "command" << DISCARD("eol"));

        g["command"]            = RULES("comment", "typedef", "funcdef", "funcdecl");

        g["typedef"]            = RULE("name" << DISCARD(".") << DISCARD("~") << DISCARD(".") << "type");
        g["funcdef"]            = RULE("name" << DISCARD(".") << DISCARD("<-") << DISCARD(".") << "func");
        g["funcdecl"]           = RULE("name" << DISCARD(".") << DISCARD(":") << DISCARD(".") << "type");

        g["type"]               = RULES("signature", "name");

        g["name"]               = { RULE("token" << DISCARD("-") << LIFT("name")), RULE("token") };

        g["token"]              = RULES("abstraction","symbol");

        g["abstraction"]        = RULES("[X]", "{X}", "<X>", "/X/", "\\X\\");

        g["[X]"]                = RULE(DISCARD("[") << "name" << DISCARD("]"));
        g["{X}"]                = RULE(DISCARD("{") << "name" << DISCARD("}"));
        g["<X>"]                = RULE(DISCARD("<") << "name" << DISCARD(">"));
        g["/X/"]                = RULE(DISCARD("/") << "name" << DISCARD("/"));
        g["\\X\\"]              = RULE(DISCARD("\\") << "name" << DISCARD("\\"));

        g["fullname"]           = { RULE("name" << DISCARD("_") << LIFT("fullname")), RULE("name") };

        g["signature"]          = RULE("fullname" << DISCARD(".") << DISCARD("->") << DISCARD(".") << "name");
        g["reference"]          = RULE(DISCARD("@") << "number" << q::question);

        g["func"]               = { RULE("expr" << DISCARD(".") << DISCARD(",") << DISCARD(".") << LIFT("func")), RULE("expr") };
        g["expr"]               = { RULE("call" << DISCARD(".") << DISCARD("|") << DISCARD(".") << LIFT("expr")), RULE("call") };
        g["call"]               = { RULE("name" << DISCARD("_") << "parameters"), RULE("name") };
        g["parameters"]         = { RULE("parameter" << DISCARD("_") << LIFT("parameters")), RULE("parameter") };

        g["parameter"]          = RULES("name", "literal", "reference", "integer");
        g["literal"]            = RULES("literal1", "literal2");
        g["integer"]            = RULES("number","hex","oct","dec","bin");

        rule::default_type = rule_type::terminal;

        //
        // terminal rules
        //

        g["."]          = RULE("[ \\t]*");
        g["_"]          = RULE("[ \\t]+");

        g["*"]          = RULE("\\s*");

        g["eol"]        = RULE("\\s*($|\\z)");

        g["eof"]        = RULE("\\z");

        g["comment"]    = RULE("#[^\\n]*");
        g["symbol"]     = RULE("[[:alpha:]][[:alnum:]]*");

        g["literal1"]   = RULE("\"(\\\\\\\\|\\\\\"|[^\"])*\"");
        g["literal2"]   = RULE("\\((\\\\\\\\|\\\\\\)|[^)])*\\)");

        g["number"]     = RULE("[-+]?(0|[1-9]\\d*)\\b");

        g["hex"]        = RULE("0[xX][[:xdigit:]]+\\b");
        g["oct"]        = RULE("0[oO][0-7]+\\b");
        g["dec"]        = RULE("0[dD]\\d+\\b");
        g["bin"]        = RULE("0[bB][01]+\\b");

        //
        // simple terminal rules
        //

        const std::list<std::string> escapes = { "\\", "/", "{", "}", "[", "]", "|", "~" };

        const std::list<std::string> literals = { "<", ">", "<-", "->", ":", ",", "@", "-" };

        for(auto escape : escapes)
                ESCAPED(g, escape);

        for(auto literal : literals)
                LITERAL(g, literal);
}

static int grammar_rule_width(const grammar& g) {

        std::string ms;

        for(const auto& x : g)
                if(x.first.size() > ms.size())
                        ms = x.first;

        return (int)ms.size();
}

static std::string rule_list_string(const std::list<rule>& rs) {

        std::stringstream ss;

        for(auto iter = rs.begin(); iter != rs.end(); iter++) {

                auto& y = *iter;

                if(y.type == rule_type::recursive) {

                        for(auto z : y.recursive)
                                ss << ' ' << z.name << q_string(z.quantifier);

                } else if(y.type == rule_type::terminal) {

                        ss << " " COLOR3 "/" COLOR4 << y.terminal.str() << COLOR3 "/" ANSI(0);

                } else {

                        ss << " UNDEFINED" << std::endl;
                }

                if(next(iter) == rs.end())
                        ss << std::endl;
                else
			ss << " " COLOR5 "/" ANSI(0);
        }

        return ss.str();
}

#define FLAG '\t' << std::left << std::setw(18)

static void usage(const char *arg0) {

        std::cerr << std::endl << "usage: " << arg0 << " [-{gh}] [-p filename]" << std::endl << std::endl;

	std::cerr << FLAG << "-g" << "print grammar specification" << std::endl;
        std::cerr << FLAG << "-h" << "print help" << std::endl;
        std::cerr << FLAG << "-p filename" << "parse file" << std::endl;

        std::cerr << std::endl;
}

#undef FLAG	

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

        define_p_grammar(g);

        if(do_grammar) {

                int width = grammar_rule_width(g);

                for(auto x : g)
                        std::cout << std::setw(width) << x.first << " " COLOR6 ":=" ANSI(0) << rule_list_string(x.second);
        }

        if(do_parse) {

                FILE *fp = fopen(filename, "r");
                if(fp == NULL)
			throw new std::runtime_error(std::string("fopen() failed--") + strerror(errno));

                ast q(g, fp);

                fclose(fp);

                std::cout << ast_string(q);
        }

        return 0;
}
