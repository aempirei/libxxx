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

#define ANSI(x)	"\33[" #x "m"	
#define COLOR1	ANSI(1;34)
#define COLOR2	ANSI(0;34)
#define COLOR3	ANSI(1;33)
#define COLOR4	ANSI(0;33)
#define COLOR5	ANSI(1;31)
#define COLOR6	ANSI(0;31)

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
		std::cout << grammar_str(g);
        }

        if(do_parse) {

                FILE *fp = fopen(filename, "r");
                if(fp == NULL) {
			perror("fopen()");
			return -1;
		}

                ast q(g, fp);

                fclose(fp);

                std::cout << q.str();
        }

        return 0;
}
