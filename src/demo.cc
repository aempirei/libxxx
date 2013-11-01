#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>
#include <cassert>

#include <list>
#include <iostream>
#include <iomanip>
#include <thread>
#include <sstream>
#include <algorithm>

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

void define_p_grammar(grammar& z) {

        rule::default_type = rule_type::recursive;

        //
        // recursive rules
        //

        z["document"]           = RULE("statement" << q::star);

        z["statement"]          = RULES("typedef", "funcdef", "funcdecl", "comment");

        z["typedef"]            = RULE("." << "name" << "." << "~" << "." << "signature" << ".");
        z["funcdef"]            = RULE("." << "name" << "." << "<-" << "." << "funcbody" << ".");
        z["funcdecl"]           = RULE("." << "name" << "." << ":" << "." << "signature" << ".");

        z["comment"]            = RULE("." << "hash" << "tail");

        z["name"]               = { RULE("token" << "-" << "name"), RULE("token") };

        z["token"]              = RULES("symbol", "abstraction");
        z["abstraction"]        = RULES("abstraction1", "abstraction2", "abstraction3", "abstraction4", "abstraction5");

        z["abstraction1"]       = RULE("[" << "name" << "]");
        z["abstraction2"]       = RULE("{" << "name" << "}");
        z["abstraction3"]       = RULE("<" << "name" << ">");
        z["abstraction4"]       = RULE("/" << "name" << "/");
        z["abstraction5"]       = RULE("\\" << "name" << "\\");

        z["fullname"]           = { RULE("name" << "_" << "fullname"), RULE("name") };

        z["signature"]          = RULE("fullname" << "." << "->" << "." << "name");
        z["reference"]          = RULE("@" << "number" << q::question);

        z["funcbody"]           = { RULE("expr" << "." << "," << "." << "funcbody"), RULE("expr") };
        z["expr"]               = { RULE("call" << "." << "|" << "." << "expr"), RULE("call") };
        z["call"]               = { RULE("name" << "_" << "parameters"), RULE("name") };
        z["parameters"]         = { RULE("parameter" << "_" << "parameters"), RULE("parameter") };

        z["parameter"]          = RULES("name", "literal", "reference");
        z["literal"]            = RULES("literal1", "literal2");

        /*

           z["name"]               = RULE("token" << "-token" << q::star);
           z["-token"]             = RULE("-" << "token");
           z["signature"]          = RULE("name_" << q::star << "name" << "." << "->" << "." << "name");
           z["name_"]              = RULE("name" << "_");
           z["funcbody"]           = RULE("expr.,." << q::star << "expr");
           z["expr.,."]            = RULE("expr" << "." << "," << ".");
           z["expr"]               = RULE("call.|." << q::star << "call");
           z["call"]               = RULE("name" << "_parameter" << q::star);
           z["call.|."]            = RULE("call" << "." << "|" << ".");
           z["_parameter"]         = RULE("_" << "parameter");

         */

        rule::default_type = rule_type::terminal;

        //
        // terminal rules
        //

        z["."]          = RULE("\\s*");
        z["_"]          = RULE("\\s+");

        z["tail"]       = RULE("[^\\n]*$");
        z["symbol"]     = RULE("[[:alpha:]][[:alnum:]]*");

        z["literal1"]   = RULE("\"(\\\\\\\\|\\\\\"|[^\"])*\"");
        z["literal2"]   = RULE("\\((\\\\\\\\|\\\\\\)|[^)])*\\)");

        z["number"]     = RULE("(0|[1-9]\\d*)\\b");

        //
        // simple terminal rules
        //

        const std::list<std::string> escapes = { "\\", "/", "{", "}", "[", "]", "|" };

        const std::list<std::string> literals = { "<", ">", "<-", "->", ":", "~", "-", ",", "@" };

        for(auto escape : escapes)
                ESCAPED(z, escape);

        for(auto literal : literals)
                LITERAL(z, literal);
}

#undef RULE
#undef RULES
#undef LITERAL
#undef ESCAPED

int grammar_rule_width(const grammar& g) {

        std::string ms;

        for(const auto& x : g)
                if(x.first.length() > ms.length())
                        ms = x.first;

        return (int)ms.length();
}

std::string rule_list_string(const std::list<rule>& rs) {

        std::stringstream ss;

        const char yel[] = "\33[1;33m";
        const char brown[] = "\33[0;33m";
        const char red[] = "\33[0;31m";
        const char none[] = "\33[0m";

        for(auto iter = rs.begin(); iter != rs.end(); iter++) {

                auto& y = *iter;

                if(y.type == rule::rule_type::recursive) {

                        for(auto z : y.recursive_value)
                                ss << ' ' << z.first << qstring(z.second);

                } else if(y.type == rule::rule_type::terminal) {

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

void parse_grammar(grammar& g, FILE *fp) {
        fclose(fp);
        g.clear();
}

int main(int argc, char **argv) {

        if(argc < 1) {
                usage(*argv);
                return -1;
        }

        grammar plang;

        std::locale::global(std::locale("en_US.UTF-8"));

        define_p_grammar(plang);

        int width = grammar_rule_width(plang);

        for(auto x : plang)
                std::cout << std::setw(width) << x.first << " \33[1;30m:=\33[0m" << rule_list_string(x.second);

        parse_grammar(plang, stdin);

        return 0;
}
