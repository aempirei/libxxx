#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>
#include <cassert>
#include <cstdarg>

extern "C" {
#include <unistd.h>
}

#include <xxx.hh>
#include <xxx-xxx.hh>

namespace xxx {

    static grammar load_dynamic_grammar(const ast& a) {

        grammar g;

        if(a.name() == "document") {

            for(const auto& b : a.children) {

                const auto& name = b.children[0].match;

                if(g.find(name) == g.end())
                    g[name] = {};

                if(b.name() == "regex") {

                    g.at(name).push_back(rule(rule_type::regex, b.children[1].match));

                } else {

                    // ordered

                    for(const auto& c : b.children[1].children) {

                        // predicates

                        rule r;

                        for(const auto& d : c.children) {

                            // predicate

                            predicate p;

                            auto iter = d.children.begin();

                            if(iter->name() == "modifier") {
                                
                                /**/ if(iter->match == "^") p.modifier = predicate_modifier::lift;
                                else if(iter->match == "!") p.modifier = predicate_modifier::discard;
                                else if(iter->match == ">") p.modifier = predicate_modifier::peek_positive;
                                else if(iter->match == "~") p.modifier = predicate_modifier::peek_negative;

                                iter++;

                            } else {
                                p.modifier = predicate_modifier::push;
                            }

                            if(iter->name() == "name") {
                                p.name = iter->match;
                                iter++;
                            }

                            if(iter != d.children.end() and iter->name() == "quantifier") {

                                /**/ if(iter->match == "*") p.quantifier = q::star;
                                else if(iter->match == "+") p.quantifier = q::plus;
                                else if(iter->match == "?") p.quantifier = q::question;

                            } else {
                                p.quantifier = q::one;
                            }

                            r << p;
                        }

                        g.at(name).push_back(r);
                    }
                }
            }
        }

        return g;
    }
}

using namespace xxx;

#define FLAG '\t' << std::left << std::setw(18)

static void usage(const char *arg0) {

    auto usageline = [](char flag, const char *opt, const char *msg) {
        std::stringstream ss;
        ss << '-' << flag;
        if(opt != nullptr) ss << ' ' << opt;
        std::cerr << '\t' << std::left << std::setw(13) << ss.str();
        if(msg != nullptr) std::cerr << ' ' << msg;
        std::cerr << std::endl;
    };

    std::cerr << std::endl << "usage: " << arg0 << " [-{hpaxci}] [-g filename]" << std::endl << std::endl;

    usageline('h', nullptr   , "show this help"       );
    usageline('p', nullptr   , "display grammar"      );
    usageline('a', nullptr   , "display ast"          );
    usageline('x', nullptr   , "display xml"          );
    usageline('c', nullptr   , "display code"         );
    usageline('i', nullptr   , "parse stdin"          );
    usageline('g', "filename", "grammar specification");

    std::cerr << std::endl;
}

#undef FLAG

int main(int argc, char **argv) {

    bool do_print_code		= false;
    bool do_print_xml		= false;
    bool do_print_grammar	= false;
    bool do_load_grammar	= false;
    bool do_print_ast		= false;
    bool do_parse_input		= false;

    const char *filename = nullptr;

    int opt;

    if(argc == 1) {
        usage(*argv);
        return -1;
    }

    while ((opt = getopt(argc, argv, "achpsixg:")) != -1) {

        switch (opt) {

            case 'g': do_load_grammar	= true; filename = optarg;	break;
            case 'a': do_print_ast		= true;						break;
            case 'x': do_print_xml		= true;						break;
            case 'p': do_print_grammar	= true;						break;
            case 'c': do_print_code		= true;						break;
            case 'i': do_parse_input	= true;						break;

            case 'h':
            case '?':
            default:

                      usage(*argv);
                      return -1;
        }
    }

    grammar g = define_grammar();

    if(do_print_grammar and not do_parse_input)
        std::cout << g.to_s(grammar::string_format_type::xxx);

    if(do_load_grammar) {

        FILE *fp = fopen(filename, "r");
        if(fp == nullptr) {
            perror("fopen()");
            return -1;
        }

        ast a(g, fp);

        fclose(fp);

        if(do_print_ast and not do_parse_input)
            std::cout << a.str() << std::endl;

        grammar h = load_dynamic_grammar(a);

        if(do_print_code)
            std::cout << h.to_s(grammar::string_format_type::cc);

        if(do_parse_input) {

            ast b(h, stdin);

            if(do_print_grammar)
                std::cout << h.to_s(grammar::string_format_type::xxx);

            if(do_print_ast)
                std::cout << b.str() << std::endl;

            if(do_print_xml)
                std::cout << b.xml() << std::endl;
        }
    }

    return 0;
}
