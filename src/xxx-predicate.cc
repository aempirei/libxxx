#include <xxx.hh>

namespace xxx {

    const predicate_modifier predicate_modifier::push = predicate_modifier(0);
    const predicate_modifier predicate_modifier::peek = predicate_modifier('>');
    const predicate_modifier predicate_modifier::discard = predicate_modifier('!');

    predicate_modifier::predicate_modifier(const std::string& s) : ch(s.front()) {
    }

    const predicate_quantifier predicate_quantifier::star     ( 0, SIZE_MAX );
    const predicate_quantifier predicate_quantifier::plus     ( 1, SIZE_MAX );
    const predicate_quantifier predicate_quantifier::question ( 0, 1        );
    const predicate_quantifier predicate_quantifier::one      ( 1, 1        );

    predicate_quantifier::predicate_quantifier(const std::string& s) : predicate_quantifier(s.empty() ? '\0' : s.front()) {
    }

    std::string predicate_quantifier::str() const {

        char ch = operator char();

        if(ch == '\0') {

            if(operator==(one))
                return "";

            std::stringstream ss; 

            ss << '{' << first;

            if(first != second)
                ss << ',' << second;

            ss << '}';

            return ss.str();
        }

        return std::string(1, ch);
    }

    //
    // predicate
    //

    predicate::predicate() : modifier(predicate_modifier::push), quantifier(predicate_quantifier::one) {
    }

    predicate::predicate(const std::string& my_name) : predicate(predicate_modifier::push, my_name, predicate_quantifier::one) 
    {
    }

    predicate::predicate(predicate_modifier my_modifier, const predicate_name& my_name, const predicate_quantifier& my_quantifier)
        : modifier(my_modifier), name(my_name), quantifier(my_quantifier)
    {
    }

    size_t predicate::lower() const {
        return quantifier.first;
    }

    size_t predicate::upper() const {
        return quantifier.second;
    }

    std::string predicate::str() const {

        std::stringstream ss;

        if(modifier != predicate_modifier::push)
            ss << (char)modifier;

        ss << name << quantifier.str();

        return ss.str();
    }

    std::string predicate::to_cc_def(size_t n) const {

        std::stringstream ssexpr; 
        ssexpr << "(iter++)->transform(&arg" << n << ");" << std::endl;
        std::string expr = ssexpr.str();

        std::stringstream sscond; 
        sscond << "(iter != a->children.end() and iter->match_name() == \"" << name << "\")";
        std::string cond = sscond.str();

        std::stringstream ss; 

        /**/ if(quantifier == predicate_quantifier::one     ) ss << "\t\t\t" << expr;
        else if(quantifier == predicate_quantifier::question) ss <<                  "\t\t\tif"    << cond         << std::endl << '\t' << "\t\t\t" << expr;
        else if(quantifier == predicate_quantifier::star    ) ss <<                  "\t\t\twhile" << cond         << std::endl << '\t' << "\t\t\t" << expr;
        else if(quantifier == predicate_quantifier::plus    ) ss << "\t\t\tdo " << expr << "while" << cond << ";"  << std::endl                ;
        else                                                  ss <<                  "\t\t\tfor(;" << cond << ";)" << std::endl << '\t' << "\t\t\t" << expr;

        return ss.str();
    }

    std::string predicate::to_cc_decl(size_t n) const {
        std::stringstream ss;
        ss << "\t\t\t" << name << " arg" << n << ';' << std::endl;
        return ss.str();
    }
}
