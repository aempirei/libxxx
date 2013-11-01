#pragma once

#include <set>
#include <map>
#include <string>
#include <regex>
#include <climits>

namespace lang {

	using quantifier = std::pair<int,int>;

	using symbol = std::string;

	using predicate = std::pair<symbol,quantifier>;

	struct rule {

		enum class rule_type { terminal, recursive };

		typedef std::list<predicate> recursive_type;
		typedef std::regex terminal_type;

		rule_type type;

		terminal_type terminal_value;
		recursive_type recursive_value;

		rule();
		rule(terminal_type);
		rule(recursive_type);
		rule(const rule&);
	};

	using grammar = std::map<symbol,std::list<rule>>;

	struct q {
		static const quantifier star;
		static const quantifier plus;
		static const quantifier question;
	};
}
