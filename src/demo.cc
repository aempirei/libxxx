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

#include <xxx.hh>

using namespace xxx;

static void usage(const char *arg0) {
        fprintf(stderr, "\nusage: %s string regex [options] < input\n\n", arg0);
}

std::string ast_string(const ast& q, int depth=0, bool basic=false) {

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

std::string q_string(const quantifier& x) {

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

void define_p_grammar(grammar& z) {

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

        z["."]          = RULE("[\\s]*");
        z["_"]          = RULE("[\\s]+");

        z["*"]          = RULE("[\\s\\n]*");

        z["eol"]        = RULE("[\\s]*($|\\z)");

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

        const std::list<std::string> literals = { "<", ">", "<-", "->", ":", ",", "@", "#", "-" };

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

struct parser {
        static ast& parse(const grammar&, FILE*, ast&);
        static ast& parse(const grammar&, std::string, ast&);
        static std::pair<ssize_t,ssize_t> parse_recursive(const grammar&, std::string, const std::string&, ast&, ssize_t offset = 0);
};

ast& parser::parse(const grammar& g, FILE *fp, ast& q) {

        std::string s;
        char buf[1024];

        while(!feof(fp)) {
                int n = fread(buf, 1, sizeof(buf), fp);
                assert(n != -1);
                s.append(buf, n);
        }

        return parse(g, s, q);
}

ast& parser::parse(const grammar& g, std::string s, ast& q) {

        std::pair<ssize_t,ssize_t> ab = parse_recursive(g, "document", s, q);

        if(ab.first == -1) {
                std::cerr << "unmatched portion of document beginning at offset " << ab.second << std::endl;
                std::cerr << "\33[1m" << s.substr(ab.second, std::string::npos) << "\33[0m";
        }
        return q;
}

std::pair<ssize_t,ssize_t> parser::parse_recursive(const grammar& g, std::string rulename, const std::string& s, ast& q, ssize_t offset) {

        const auto iter = g.find(rulename);

        if(iter == g.end()) {
                std::stringstream ss;
                ss << "rule not found -- " << rulename;
                throw std::runtime_error(ss.str());
        }

        const auto& rules = iter->second;

        ssize_t current = offset;

        std::string ms;

        q.offset = offset;
        q.rulename = rulename;

        for(const auto& rule : rules) {

                bool success = true;

                match matches;

                current = offset;

                q.children.clear();
                q.type = rule.type;

                switch(rule.type) {

                        case rule_type::undefined:

                                throw std::runtime_error("rule type is undefined");
                                break;

                        case rule_type::terminal:

                                ms = s.substr(offset, std::string::npos);

                                if(!boost::regex_search(ms, matches, rule.terminal_value)) {
                                        success = false;
                                        break;
                                }

                                q.string = matches[0].str();
                                q.terminal_matches = matches;
                                current += q.string.length();

                                break;

                        case rule_type::recursive:

                                for(const auto& predicate : rule.recursive_value) {

                                        int i;

                                        for(i = 0; i < predicate.second.second; i++) {

                                                ast qq;

                                                auto next = parse_recursive(g, predicate.first, s, qq, current);

                                                if(next.first == -1)
                                                        break;

                                                current = next.second;

                                                if(predicate.modifier == predicate_modifier::push) {

                                                        q.children.push_back(qq);

                                                } else if(predicate.modifier == predicate_modifier::lift) {

                                                        if(qq.type != rule_type::recursive)
                                                                throw new std::runtime_error("attempting to lift non-recursive ast node");

                                                        for(const auto& qqq : qq.children)
                                                                q.children.push_back(qqq);

                                                } else if(predicate.modifier == predicate_modifier::discard) {
                                                        // discard
                                                }
                                        }

                                        if(i < predicate.second.first) {
                                                success = false;
                                                break;
                                        }
                                }

                                break;

                        default:

                                throw std::runtime_error("rule type is unknown");
                                break;
                }

                if(success)
                        return std::pair<ssize_t,ssize_t>(offset,current);
        }

        return std::pair<ssize_t,ssize_t>(-1,current);
}

int main(int argc, char **argv) {

        if(argc < 1) {
                usage(*argv);
                return -1;
        }

        grammar g;
        ast past;

        // std::locale::global(std::locale("en_US.UTF-8"));

        define_p_grammar(g);

        int width = grammar_rule_width(g);

        for(auto x : g)
                std::cout << std::setw(width) << x.first << " \33[1;30m:=\33[0m" << rule_list_string(x.second);

        parser::parse(g, stdin, past);

        std::cout << ast_string(past);

        return 0;
}
