#include <xxx.hh>

namespace xxx {

	predicate::predicate() : quantifier(q::one), modifier(predicate_modifier::push) {
	}

	predicate::predicate(const std::string& my_name) : name(my_name), quantifier(q::one), modifier(predicate_modifier::push) {
	}

	std::string predicate::str() const {

		std::stringstream ss;

		/**/ if(modifier == predicate_modifier::push   ) ss << "";
		else if(modifier == predicate_modifier::discard) ss << '!';
		else if(modifier == predicate_modifier::lift   ) ss << '^';
		else if(modifier == predicate_modifier::peek   ) ss << '>';

		else throw std::runtime_error("unknown predicate modifier for " + name);

		ss << name;

		/**/ if(quantifier == q::one     ) ss << "";
		else if(quantifier == q::star    ) ss << '*';
		else if(quantifier == q::plus    ) ss << '+';
		else if(quantifier == q::zero    ) ss << '-';
		else if(quantifier == q::question) ss << '?';

		else if(quantifier.first  == quantifier.second) ss << '{' << quantifier.first                             << '}';
		else                                            ss << '{' << quantifier.first << ',' << quantifier.second << '}';

		return ss.str();
	}
}
