#pragma once

#include <set>
#include <map>
#include <string>
#include <climits>

namespace lang {
	using quantifier = std::pair<int,int>;
	using symbol = std::string;
	using predicate = std::pair<symbol,quantifier>;
	using rule = std::list<predicate>;
	using grammar = std::map<symbol,std::list<rule>>;

	extern const quantifier QSTAR;
	extern const quantifier QPLUS;
	extern const quantifier QQUESTION;
}
