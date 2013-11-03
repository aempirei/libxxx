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

        z["document"]           = RULE("." << "statement" << q::star << "eof");

        z["statement"]          = RULES("typedef", "funcdef", "funcdecl", "comment");

        z["typedef"]            = RULE("name" << "." << "~" << "." << "signature" << ".");
        z["funcdef"]            = RULE("name" << "." << "<-" << "." << "funcbody" << ".");
        z["funcdecl"]           = RULE("name" << "." << ":" << "." << "signature" << ".");

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

        rule::default_type = rule_type::terminal;

        //
        // terminal rules
        //

        z["."]          = RULE("\\s*");
        z["_"]          = RULE("\\s+");

        z["eof"]        = RULE("\\Z");

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

                if(y.type == rule_type::recursive) {

                        for(auto z : y.recursive_value)
                                ss << ' ' << z.first << qstring(z.second);

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

struct parser {
        static void parse(const grammar&, FILE*);
        static void parse(const grammar&, std::string);
        static ssize_t parse_recursive(const grammar&, std::string, const std::string&, ast&, ssize_t offset = 0);
};

void parser::parse(const grammar& g, FILE *fp) {

        std::string s;
        char buf[1024];

        while(!feof(fp)) {
                int n = fread(buf, 1, sizeof(buf), fp);
                assert(n != -1);
                s.append(buf, n);
        }

        parse(g, s);
}

void parser::parse(const grammar& g, std::string s) {

        std::cerr << "parsing via grammar with " << g.size() << " rules and input with " << s.length() << " bytes." << std::endl;

        ast q;

        parse_recursive(g, "document", s, q);
}

ssize_t parser::parse_recursive(const grammar& g, std::string rulename, const std::string& s, ast& q, ssize_t offset) {

        auto iter = g.find(rulename);

        if(iter == g.end()) {
                std::stringstream ss;
                ss << "rule not found -- " << rulename;
                throw std::runtime_error(ss.str());
        }
        

        auto& rules = iter->second;

        std::cerr << "q-rule:" << q.rulename << " @ " << offset << std::endl;

        q.offset = offset;
        q.rulename = rulename;

        for(const auto& rule : rules) {

                boost::cmatch matches;

                q.children.clear();
                q.type = rule.type;

                bool success = true;
                ssize_t current = offset;

                switch(rule.type) {

                        case rule_type::undefined:

                                throw std::runtime_error("rule type is undefined");
                                break;

                        case rule_type::terminal:

                                std::cerr << '\t' << "predicate: " << '/' << rule.terminal_value.str() << '/' << ' ';

                                if(!boost::regex_search(s.c_str() + offset, matches, rule.terminal_value)) {
                                        success = false;
                                        std::cerr << " -- match failure: quantifier constraints not met" << std::endl;
                                        break;
                                }

                                std::cerr << " -- match success: " << '"' << matches[0] << '"' << std::endl;

                                q.terminal_matches = matches;
                                current += matches[0].length();

                                break;

                        case rule_type::recursive:

                                for(const auto& predicate : rule.recursive_value) {

                                        std::cerr << '\t' << "predicate: " << predicate.first << qstring(predicate.second);

                                        int i;

                                        for(i = 0; i <= predicate.second.second; i++) {

                                                ast qq;

                                                ssize_t next = parse_recursive(g, predicate.first, s, qq, current);
                                                if(next == -1)
                                                        break;

                                                current = next;

                                                q.children.push_back(qq);
                                        }

                                        if(i < predicate.second.first) {
                                                success = false;
                                                std::cerr << " -- match failure: quantifier constraints not met" << std::endl;
                                                break;
                                        }

                                        std::cerr << " -- match success" << std::endl;
                                }

                                if(success)
                                        exit(0);

                                break;

                        default:

                                throw std::runtime_error("rule type is unknown");
                                break;
                }

                if(success)
                        return current;
        }

        return -1;
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

        parser::parse(plang, stdin);

        return 0;
}
