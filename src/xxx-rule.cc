#include <xxx.hh>

namespace xxx {

    //
    // to_cstring ' escape a string as a double-quoted c-string for presentation
    //

    static std::string to_cstring(const std::string& s) {
        std::stringstream ss;
        ss << '"';
        for(int c : s) {
            /**/ if(c == '"' or c == '\\') ss << '\\' << (char)c;
            else if(isprint(c)           ) ss << (char)c;
            else                           ss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        }
        ss << '"';
        return ss.str();
    }

    //
    // transform_function passthru_transform
    //

    void passthru_transform(tree *a, void *x) {
        for(auto& b : a->children)
            b.transform(x);
    }

    //
    // rule
    //

    std::string rule::str() const {

        std::stringstream ss;

        switch(type) {

            case rule_type::composite:

                if(not composite.empty()) {

                    auto iter = composite.begin();

                    ss << iter->str();

                    while(++iter != composite.end())
                        ss << ' ' << iter->str();
                }

                break;

            case rule_type::terminal:

                ss << '/' << terminal.str().substr(2,std::string::npos) << '/';
                break;
        }

        return ss.str();
    }

    vars rule::to_sig() const {

        if(type == rule_type::terminal)
            return { "std::string" };

        vars sig;

        for(const auto& p : composite)
            if(p.modifier == predicate_modifier::push)
                sig.push_back(
                        p.quantifier == predicate_quantifier::one      ? (p.name) :
                        p.quantifier == predicate_quantifier::question ? (p.name + '?') :
                                                      ('[' + p.name + ']')
                );

        return sig;
    }

    std::string rule::to_cc() const {

        if(type == rule_type::terminal)
            return "R(" + to_cstring(terminal.str()) + ")";

        std::stringstream ss;

        ss << "rule()";

        for(const auto& p : composite) {
            ss << " << \"" << p.name << '\"';

            ss << ( (p.quantifier == predicate_quantifier::star    ) ? " << Q::star"     :
                    (p.quantifier == predicate_quantifier::plus    ) ? " << Q::plus"     :
                    (p.quantifier == predicate_quantifier::question) ? " << Q::question" : "" );

            ss << ( (p.modifier == predicate_modifier::discard) ? " << M::discard" :
                    (p.modifier == predicate_modifier::peek   ) ? " << M::peek"    : "" );
        }

        return ss.str();
    }

    //
    // rule::rule
    //

    rule::rule() : rule(rule_type::composite) {
    }

    rule::rule(rule_type my_type) : type(my_type), transform(passthru_transform) {
    }

    rule::rule(const terminal_type& my_terminal) : type(rule_type::terminal), terminal(my_terminal), transform(passthru_transform) {
    }

    rule::rule(const composite_type& my_composite) : type(rule_type::composite), composite(my_composite), transform(passthru_transform) {
    }

    //
    // rule::operator>>
    //

    rule& rule::operator>>(transform_function *x) {
        transform = x;
        return *this;
    }

    //
    // rule::operator<<
    //

    rule& rule::operator<<(const std::string& x) {
        return operator<<(predicate(x));
    }

    rule& rule::operator<<(const predicate& x) {

        if(type != rule_type::composite)
            throw std::runtime_error("rule::type is not rule_type::composite in rule::operator<<");

        composite.push_back(predicate(x));

        return *this;
    }

    rule& rule::operator<<(predicate_modifier x) {

        if(type != rule_type::composite)
            throw std::runtime_error("rule::type is not rule_type::composite in rule::operator<<");

        if(composite.empty())
            throw std::runtime_error("cannot assign quantifier to last predicate--composite rule is empty");

        composite.back().modifier = x;

        return *this;
    }

    rule& rule::operator<<(const predicate_quantifier& x) {

        if(type != rule_type::composite)
            throw std::runtime_error("rule::type is not rule_type::composite in rule::operator<<");

        if(composite.empty())
            throw std::runtime_error("cannot assign quantifier to last predicate--composite rule is empty");

        composite.back().quantifier = x;

        return *this;
    }

}
