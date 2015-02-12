#include <xxx.hh>

namespace xxx {

    const predicate_quantifier predicate_quantifier::maybe (0,1);
    const predicate_quantifier predicate_quantifier::one   (1,1);

    predicate_quantifier::predicate_quantifier(const std::string& s) : predicate_quantifier(s.empty() ? '\0' : s.front()) {
    }

    std::string predicate_quantifier::str() const {
        char ch = operator char();
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

        if(quantifier == predicate_quantifier::maybe)
            ss << "\t\t\tif" << cond << std::endl << '\t';

        ss << "\t\t\t" << expr;

        return ss.str();
    }

    std::string predicate::to_cc_decl(size_t n) const {
        std::stringstream ss;
        ss << "\t\t\t" << name << " arg" << n << ';' << std::endl;
        return ss.str();
    }
}
