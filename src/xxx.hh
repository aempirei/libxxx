#pragma once

#include <climits>

#include <map>
#include <list>
#include <vector>
#include <string>
#include <iomanip>

#include <boost/regex.hpp>

namespace xxx {

	//
	// predicate
	//

	template<typename T> using range = std::pair<T,T>;

	using predicate_quantifier = range<size_t>;

	enum class predicate_modifier { push, discard, lift, peek };

	struct predicate {

		std::string name;
		predicate_quantifier quantifier;
		predicate_modifier modifier;

		predicate();
		predicate(const std::string&);

		std::string str() const;
	};

	//
	// rule
	//

	enum class rule_type { terminal, recursive };

	struct rule {

		typedef std::list<predicate> recursive_type;
		typedef boost::regex terminal_type;

		static rule_type default_type;

		rule_type type;

		terminal_type terminal;
		recursive_type recursive;

		rule();
		rule(rule_type);
		rule(const rule&);
		rule(const terminal_type&);
		rule(const recursive_type&);
		rule(const std::string&);

		rule& operator<<(const rule_type);
		rule& operator<<(const predicate_modifier);

		rule& operator<<(const terminal_type&);
		rule& operator<<(const std::string&);
		rule& operator<<(const predicate_quantifier&);
		rule& operator<<(const predicate&);

		void retype(rule_type);

		std::string str() const;

		static std::list<rule> singletons(const std::list<std::string>&);
	};

	//
	// grammar
	//

	using grammar = std::map<std::string,std::list<rule>>;

	std::string grammar_str(const grammar&);

	//
	// ast
	//

	struct ast {

		std::string name;

		rule_type type;

		std::vector<std::string> matches;

		std::vector<ast> children;

		ssize_t offset;

		ast();
		ast(const grammar&, FILE *);
		ast(const grammar&, const std::string&);

		void parse(const grammar&, FILE *);
		void parse(const grammar&, const std::string&);

		std::string str() const;
		std::string xml() const;
	};

	//
	// q
	//

	namespace q {
		extern const predicate_quantifier star;
		extern const predicate_quantifier plus;
		extern const predicate_quantifier question;
		extern const predicate_quantifier one;
	}
}
