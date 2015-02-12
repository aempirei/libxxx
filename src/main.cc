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
#include <xxx-local.hh>
#include <xxx-xxx.hh>

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
    usageline('a', nullptr   , "display parse tree"   );
    usageline('x', nullptr   , "display xml"          );
    usageline('c', nullptr   , "display code"         );
    usageline('i', nullptr   , "parse stdin"          );
    usageline('g', "filename", "grammar specification");

    std::cerr << std::endl;
}

#undef FLAG

int main(int argc, char **argv) {

    bool do_print_code      = false;
    bool do_print_xml       = false;
    bool do_print_grammar   = false;
    bool do_load_grammar    = false;
    bool do_print_tree      = false;
    bool do_parse_input     = false;

    const char *filename = nullptr;

    int opt;

    if(argc == 1) {
        usage(*argv);
        return -1;
    }

    while ((opt = getopt(argc, argv, "achpsixg:")) != -1) {

        switch (opt) {

            case 'g': do_load_grammar   = true; filename = optarg;  break;
            case 'a': do_print_tree     = true;                     break;
            case 'x': do_print_xml      = true;                     break;
            case 'p': do_print_grammar  = true;                     break;
            case 'c': do_print_code     = true;                     break;
            case 'i': do_parse_input    = true;                     break;

            case 'h':
            case '?':
            default:

                      usage(*argv);
                      return -1;
        }
    }

    if(do_print_grammar and not do_parse_input)
        std::cout << local::spec.to_xxx();

    if(do_load_grammar) {

        FILE *fp = fopen(filename, "r");
        if(fp == nullptr) {
            perror("fopen()");
            return -1;
        }

        tree a(local::spec, fp);

        fclose(fp);

        if(do_print_tree and not do_parse_input)
            std::cout << a.str() << std::endl;

        grammar h;

        a.transform(&h);

        if(do_print_code)
            std::cout << h.to_cc();

        if(do_parse_input) {

            tree b(h, stdin);

            if(do_print_grammar)
                std::cout << h.to_xxx();

            if(do_print_tree)
                std::cout << b.str() << std::endl;

            if(do_print_xml)
                std::cout << b.xml() << std::endl;
        }
    }

    return 0;
}
