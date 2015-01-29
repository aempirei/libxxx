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

        if(a.name == "document") {

            for(const auto& b : a.children) {

                const auto& name = b.children[0].matches[0];

                /**/ if(b.name == "literal"  ) g[name] = { rule(rule_type::literal, b.children[1].matches[0]) };
                else if(b.name == "builtin"  ) g[name] = { rule(rule_type::builtin, b.children[1].matches[0]) };
                else if(b.name == "regex"    ) g[name] = { rule(rule_type::regex  , b.children[1].matches[0]) };
                else if(b.name == "recursive") {

                    g[name] = {};

                    // ordered

                    for(const auto& c : b.children[1].children) {

                        // predicates

                        rule r;

                        for(const auto& d : c.children) {

                            // predicate

                            predicate p;

                            auto iter = d.children.begin();

                            if(iter->name == "modifier") {
                                
                                /**/ if(iter->matches[0] == "^") p.modifier = predicate_modifier::lift;
                                else if(iter->matches[0] == "!") p.modifier = predicate_modifier::discard;
                                else if(iter->matches[0] == ">") p.modifier = predicate_modifier::peek_positive;
                                else if(iter->matches[0] == "~") p.modifier = predicate_modifier::peek_negative;

                                iter++;

                            } else {
                                p.modifier = predicate_modifier::push;
                            }

                            if(iter->name == "name") {
                                p.name = iter->matches[0];
                                iter++;
                            }

                            if(iter != d.children.end() and iter->name == "quantifier") {

                                /**/ if(iter->matches[0] == "*") p.quantifier = q::star;
                                else if(iter->matches[0] == "+") p.quantifier = q::plus;
                                else if(iter->matches[0] == "?") p.quantifier = q::question;

                            } else {
                                p.quantifier = q::one;
                            }

                            r << p;
                        }

                        g[name].push_back(r);
                    }
                } else {
                    std::stringstream ss;
                    ss << "unexpected rule type " << b.name << " in load_dynamic_grammar";
                    throw std::runtime_error(ss.str());
                }
            }
        }

        return g;
    }

	static std::string codify_dynamic_grammar(const ast& a) {

		std::stringstream ss;

		ss << "namespace xxx {" << std::endl;
		ss << "\tstatic grammar define_grammar() {" << std::endl;
		ss << "\t\tgrammar g;" << std::endl;
		ss << "\t\tusing M = predicate_modifier;" << std::endl;

		if(a.name == "document") {

			for(const auto& b : a.children) {

				const auto& name = b.children[0].matches[0];

				if(b.name == "literal") {

					ss << "\t\tg[\"" << name << "\"] = { rule(rule_type::literal, \"" << b.children[1].matches[0] << "\") };" << std::endl;

                } else if(b.name == "builtin") {

					ss << "\t\tg[\"" << name << "\"] = { rule(" << b.children[1].matches[0] << ") };" << std::endl;

                } else if(b.name == "regex") {

					const auto& regexstr = b.children[1].matches[0];

					std::string escreg;

					for(size_t n = 0; n < regexstr.length(); n++) {
						if(regexstr[n] == '\\' or regexstr[n] == '"')
							escreg.push_back('\\');
						escreg.push_back(regexstr[n]);
					}

					ss << "\t\tg[\"" << name << "\"] = { rule(rule_type::regex, \"" << escreg << "\") };" << std::endl;

				} else {

					ss << "\t\tg[\"" << name << "\"] = {";

					// ordered

					bool single = (b.children[1].children.size() == 1);

					for(const auto& c : b.children[1].children) {

						// predicates

						if(single)
							ss << ' ';
						else
							ss << std::endl << "\t\t\t";

						ss << "{ rule()";

						for(const auto& d : c.children) {

							// predicate

							auto iter = d.children.begin();

							predicate p;

							if(iter->name == "modifier") {

								/**/ if(iter->matches[0] == "^") p.modifier = predicate_modifier::lift;
								else if(iter->matches[0] == "!") p.modifier = predicate_modifier::discard;
								else if(iter->matches[0] == ">") p.modifier = predicate_modifier::peek_positive;
								else if(iter->matches[0] == "~") p.modifier = predicate_modifier::peek_negative;

								iter++;

							} else {
								p.modifier = predicate_modifier::push;
							}

							if(iter->name == "name") {
								p.name = iter->matches[0];
								iter++;
							}

							if(iter != d.children.end() && iter->name == "quantifier") {

								/**/ if(iter->matches[0] == "*") p.quantifier = q::star;
								else if(iter->matches[0] == "+") p.quantifier = q::plus;
								else if(iter->matches[0] == "?") p.quantifier = q::question;

							} else {
								p.quantifier = q::one;
							}

							ss << " << \"" << p.name << '\"';

							if(p.quantifier != q::one) {
								ss << (
										(p.quantifier == q::star    ) ? " << q::star"     :
										(p.quantifier == q::plus    ) ? " << q::plus"     :
										(p.quantifier == q::question) ? " << q::question" : "");
							}

							if(p.modifier != predicate_modifier::push) {
								ss << (
										(p.modifier == predicate_modifier::lift         ) ? " << M::lift"          :
										(p.modifier == predicate_modifier::discard      ) ? " << M::discard"       :
										(p.modifier == predicate_modifier::peek_positive) ? " << M::peek_positive" :
										(p.modifier == predicate_modifier::peek_negative) ? " << M::peek_negative" : "");
							}
						}

						if(single)
							ss << " } ";
						else
							ss << " },";
					}

					if(not single)
						ss << std::endl;

					ss << "\t\t};" << std::endl;
				}
			}
		}

		ss << "\t\treturn g;" << std::endl;
		ss << "\t}" << std::endl;
		ss << "}" << std::endl;

		return ss.str();
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
        std::cout << grammar_str(g);

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

        if(do_print_code)
            std::cout << codify_dynamic_grammar(a);

        grammar h = load_dynamic_grammar(a);

        if(do_parse_input) {

            ast b(h, stdin);

            if(do_print_grammar)
                std::cout << grammar_str(h);

            if(do_print_ast)
                std::cout << b.str() << std::endl;

            if(do_print_xml)
                std::cout << b.xml() << std::endl;
        }
    }

    return 0;
}
