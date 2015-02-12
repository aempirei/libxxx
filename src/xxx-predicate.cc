#include <xxx.hh>

namespace xxx {

    //
    // predicate
    //

    predicate::predicate() : modifier(predicate_modifier::type::push), quantifier(predicate_quantifier::type::one) {
    }

    predicate::predicate(const std::string& my_name) : predicate(predicate_modifier::type::push, my_name, predicate_quantifier::type::one) 
    {
    }

    predicate::predicate(const predicate_modifier& my_modifier, const predicate_name& my_name, const predicate_quantifier& my_quantifier)
        : modifier(my_modifier), name(my_name), quantifier(my_quantifier)
    {
    }

    size_t predicate::lower() const {
        return quantifier == predicate_quantifier::type::one ? 1 : 0;
    }

    size_t predicate::upper() const {
        return quantifier == predicate_quantifier::type::one ? 1 : 1;
    }

    std::string predicate::str() const {

        std::stringstream ss;

        ss << modifier.str() << name << quantifier.str();

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

        if(quantifier == predicate_quantifier::type::maybe)
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
