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
#include <xxx-standalone.hh>
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
  
    std::cerr << std::endl << "usage: " << arg0 << " [-{actpsixh}] [-g filename]" << std::endl << std::endl;

    usageline('p', nullptr   , "display grammar"         );
    usageline('a', nullptr   , "display parse tree"      );
    usageline('x', nullptr   , "display xml"             );
    usageline('c', nullptr   , "display code"            );
    usageline('t', nullptr   , "generate code transforms");
	usageline('s', nullptr   , "display stand-alone code");
    usageline('i', nullptr   , "parse stdin"             );
    usageline('h', nullptr   , "show this help"          );
    usageline('g', "filename", "grammar specification"   );

    std::cerr << std::endl;
}

#undef FLAG

int main(int argc, char **argv) {

    bool do_print_code       = false;
    bool do_print_standalone = false;
    bool do_print_xml        = false;
    bool do_print_grammar    = false;
    bool do_print_tree       = false;
    bool do_load_grammar     = false;
    bool do_parse_input      = false;
    bool use_transforms      = false;

    const char *filename = nullptr;

    int opt;

    if(argc == 1) {
        usage(*argv);
        return -1;
    }

    while ((opt = getopt(argc, argv, "actpsixg:h")) != -1) {

        switch (opt) {

            case 'a': do_print_tree       = true;                     break;
            case 'c': do_print_code       = true;                     break;
            case 't': use_transforms      = true;                     break;
            case 'p': do_print_grammar    = true;                     break;
            case 's': do_print_standalone = true;                     break;
            case 'i': do_parse_input      = true;                     break;
            case 'x': do_print_xml        = true;                     break;
            case 'g': do_load_grammar     = true; filename = optarg;  break;

            case 'h':
            case '?':
            default:

                      usage(*argv);
                      return -1;
        }
    }

    grammar g;

    if(do_load_grammar) {

        FILE *fp = fopen(filename, "r");
        if(fp == nullptr) {
            perror("fopen()");
            return -1;
        }

        tree t(local::spec, fp);
        fclose(fp);

        t.transform(&g);

        if(not do_parse_input) {
            if(do_print_xml ) std::cout << t.xml();
            if(do_print_tree) std::cout << t.str();
        }

    } else {

        g = local::spec;
    }

    if(do_parse_input) {

        tree t(g, stdin);
    
        if(do_print_xml ) std::cout << t.xml();
        if(do_print_tree) std::cout << t.str();
    }

    if(do_print_standalone) std::cout << g.to_cc_standalone();
    if(do_print_code   )    std::cout << g.to_cc(use_transforms);
    if(do_print_grammar)    std::cout << g.to_xxx();

    return 0;
}
