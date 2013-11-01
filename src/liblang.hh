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

		enum class rule_type { undefined, terminal, recursive };

		typedef std::list<predicate> recursive_type;
		typedef std::regex terminal_type;

		static const rule_type terminal  = rule_type::terminal;
		static const rule_type recursive = rule_type::recursive;
		static const rule_type undefined = rule_type::undefined;

		rule_type type;

		terminal_type terminal_value;
		recursive_type recursive_value;

		rule();
		rule(const terminal_type&);
		rule(const recursive_type&);
		rule(const rule&);

		void reset_type(rule_type);

		rule& operator<<(rule_type);
		rule& operator<<(const terminal_type&);
		rule& operator<<(const recursive_type::value_type&);
		rule& operator<<(const symbol&);
		rule& operator<<(const quantifier&);
	};

	using grammar = std::map<symbol,std::list<rule>>;

	struct q {
		static const quantifier star;
		static const quantifier plus;
		static const quantifier zero;
		static const quantifier question;
		static const quantifier one;
	};
}
