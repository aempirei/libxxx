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

void define_p_grammar(grammar& z) {

#define RULE(X)         { rule() << X }
#define RULES(...)      rule::singletons({ __VA_ARGS__ })
#define LITERAL(X,Y)    X[Y] = RULE(Y)
#define ESCAPED(X,Y)    X[Y] = RULE("\\" Y)

        //
        // recursive rules
        //

        rule::default_type = rule_type::recursive;

        z["s"]                  = RULE("statement" << q::star);

        z["statement"]          = RULES("typedef", "funcdef", "funcdecl", "comment");

        z["typedef"]            = RULE("." << "name" << "." << "tilde" << "." << "signature" << ".");
        z["funcdef"]            = RULE("." << "name" << "." << "larrow" << "." << "funcbody" << ".");
        z["funcdecl"]           = RULE("." << "name" << "." << "colon" << "." << "signature" << ".");

        z["comment"]            = RULE("." << "hash" << "tail");

        z["name"]               = RULE("token" << "dash.token" << q::star);
        z["dash.token"]             = RULE("dash" << "token");

        z["token"]              = RULES("symbol", "abstraction");
        z["abstraction"]        = RULES("abstraction1", "abstraction2", "abstraction3", "abstraction4", "abstraction5");

        z["abstraction1"]       = RULE("[" << "name" << "]");
        z["abstraction2"]       = RULE("{" << "name" << "}");
        z["abstraction3"]       = RULE("<" << "name" << ">");
        z["abstraction4"]       = RULE("/" << "name" << "/");
        z["abstraction5"]       = RULE("\\" << "name" << "\\");

        z["signature"]          = RULE("name._" << q::star << "name" << "." << "->" << "." << "name");
        z["name._"]             = RULE("name" << "_");

        z["reference"]          = RULE("@" << "number" << q::question);

        z["funcbody"]           = RULE("expr.," << q::star << "expr");
        z["expr.,"]             = RULE("expr" << "." << "," << ".");
        z["expr"]               = RULE("call.|" << q::star << "call");

        z["call.|"]             = RULE("call" << "." << "|" << ".");

        // expr := ( call . "|" . ) * call
        // 
        // call := name ( _ parameter ) *
        // 
        // parameter := name / literal / reference
        // 
        // literal := literal1 / literal2
 
        //
        // terminal rules
        //

        rule::default_type = rule_type::terminal;

        //

        z["."]          = RULE("\\s*");
        z["_"]          = RULE("\\s+");
        
        //

        z["tail"]       = RULE("[^\\n]*$");
        z["symbol"]     = RULE("[[:alpha:]][[:alnum:]]*");

        z["literal1"]   = RULE("\"(\\\\\\\\|\\\\\"|[^\"])*\"");
        z["literal2"]   = RULE("\\((\\\\\\\\|\\\\\\)|[^)])*\\)");

        z["number"]     = RULE("(0|[1-9]\\d*)\\b");

        //

        ESCAPED(z,"\\");
        ESCAPED(z,"/");

        ESCAPED(z,"{");
        ESCAPED(z,"}");

        ESCAPED(z,"[");
        ESCAPED(z,"]");

        ESCAPED(z,"|");

        //

        LITERAL(z,"<");
        LITERAL(z,">");

        LITERAL(z,"<-");
        LITERAL(z,":");
        LITERAL(z,"~");
        LITERAL(z,"-");
        LITERAL(z,",");
        LITERAL(z,"@");
}

int grammar_rule_width(const grammar& g) {

        std::string ms;

        for(const auto& x : g)
                if(x.first.length() > ms.length())
                        ms = x.first;

        return (int)ms.length();
}

std::string rule_list_string(const std::list<rule>& rs) {

        std::stringstream ss;

        for(auto iter = rs.begin(); iter != rs.end(); iter++) {

                auto& y = *iter;

                if(y.type == rule::rule_type::recursive) {

                        for(auto z : y.recursive_value)
                                ss << ' ' << z.first << qstring(z.second);

                } else if(y.type == rule::rule_type::terminal) {

                        ss << " /" << y.terminal_value.str() << '/';

                } else {

                        ss << " UNDEFINED" << std::endl;
                }

                if(next(iter) == rs.end())
                        ss << std::endl;
                else
                        ss << " /";
        }

        return ss.str();
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
                std::cout << std::left << std::setw(width) << x.first << " :=" << rule_list_string(x.second);

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
