#include <xxx.hh>

namespace xxx {

	predicate::predicate() : quantifier(Q::one), modifier(predicate_modifier::push) {
	}

	predicate::predicate(const std::string& my_name) : name(my_name), quantifier(Q::one), modifier(predicate_modifier::push) {
	}

	std::string predicate::str() const {

		std::stringstream ss;

        if(modifier != predicate_modifier::push)
            ss << (char)modifier;

		ss << name;

		/**/ if(quantifier == Q::one     ) /* NOP */;
		else if(quantifier == Q::star    ) ss << '*';
		else if(quantifier == Q::plus    ) ss << '+';
		else if(quantifier == Q::question) ss << '?';

		else if(quantifier.first  == quantifier.second) ss << '{' << quantifier.first                             << '}';
		else                                            ss << '{' << quantifier.first << ',' << quantifier.second << '}';

		return ss.str();
	}
}
