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

    //
    // predicate
    //

	predicate::predicate() : quantifier(predicate_quantifier::one), modifier(predicate_modifier::push) {
	}

	predicate::predicate(const std::string& my_name) : name(my_name), quantifier(predicate_quantifier::one), modifier(predicate_modifier::push) {
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

		ss << name;

		/**/ if(quantifier == predicate_quantifier::one     ) /* NOP */;
		else if(quantifier == predicate_quantifier::star    ) ss << '*';
		else if(quantifier == predicate_quantifier::plus    ) ss << '+';
		else if(quantifier == predicate_quantifier::question) ss << '?';

		else if(quantifier.first  == quantifier.second) ss << '{' << quantifier.first                             << '}';
		else                                            ss << '{' << quantifier.first << ',' << quantifier.second << '}';

		return ss.str();
	}
}
