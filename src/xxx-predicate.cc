#include <xxx.hh>

namespace xxx {

    //
    // predicate_quantifier
    //

    const predicate_quantifier predicate_quantifier::star     ( 0, SIZE_MAX );
    const predicate_quantifier predicate_quantifier::plus     ( 1, SIZE_MAX );
    const predicate_quantifier predicate_quantifier::question ( 0, 1        );
    const predicate_quantifier predicate_quantifier::one      ( 1, 1        );

    predicate_quantifier predicate_quantifier::upper(size_t max) {
        return predicate_quantifier(0, max);
    }

    predicate_quantifier predicate_quantifier::lower(size_t min) {
        return predicate_quantifier(min, SIZE_MAX);
    }

    predicate_quantifier::predicate_quantifier() : predicate_quantifier(predicate_quantifier::one) {
    }

    predicate_quantifier::predicate_quantifier(const std::string& s)
        : predicate_quantifier(s == "*" ? star : s == "+" ? plus : s == "?" ? question : one)
        {
        }

    std::string predicate_quantifier::str() const {

        if(*this == star    ) return "*";
        if(*this == plus    ) return "+";
        if(*this == question) return "?";
        if(*this == one     ) return "" ;

        std::stringstream ss; 

        ss << '{' << first;

        if(first != second)
            ss << ',' << second;
            
        ss << '}';

        return ss.str();

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
}
