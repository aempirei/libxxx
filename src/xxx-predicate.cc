#include <xxx.hh>

namespace xxx {

	predicate::predicate() : quantifier(q::one), modifier(predicate_modifier::push) {
	}

	predicate::predicate(const std::string& my_name) : name(my_name), quantifier(q::one), modifier(predicate_modifier::push) {
	}

	std::string predicate::str() const {

		std::stringstream ss;

        switch(modifier) {
            case predicate_modifier::push         : ss << ""; break;
            case predicate_modifier::discard      : ss << "!"; break;
            case predicate_modifier::lift         : ss << "^"; break;
            case predicate_modifier::peek_positive: ss << ">"; break;
            case predicate_modifier::peek_negative: ss << "~"; break;
        }

		ss << name;

		/**/ if(quantifier == q::one     ) ss << "";
		else if(quantifier == q::star    ) ss << '*';
		else if(quantifier == q::plus    ) ss << '+';
		else if(quantifier == q::question) ss << '?';

		else if(quantifier.first  == quantifier.second) ss << '{' << quantifier.first                             << '}';
		else                                            ss << '{' << quantifier.first << ',' << quantifier.second << '}';

		return ss.str();
	}
}
