#pragma once

#include <map>
#include <list>
#include <vector>
#include <string>
#include <iomanip>

#include <complex>

#include <boost/regex.hpp>

#include <climits>

#include <ctype.h>

namespace xxx {

	//
	// q
	//

    struct rule;
    struct predicate;

	template<typename T> using range = std::pair<T,T>;

	using predicate_quantifier = range<size_t>;

    using rules = std::list<rule>;
    using predicates = std::list<predicate>;

	namespace q {
		extern const predicate_quantifier star;
		extern const predicate_quantifier plus;
		extern const predicate_quantifier question;
		extern const predicate_quantifier one;
	}

	//
	// predicate
	//

	enum struct predicate_modifier { push, discard, lift, peek };

	struct predicate {

		std::string name;

		predicate_quantifier quantifier = q::one;
		predicate_modifier modifier = predicate_modifier::push;

		predicate();
		predicate(const std::string&);

		std::string str() const;
	};

	//
	// rule
	//

	enum struct rule_type { recursive, terminal };

	enum struct rule_transform { none, le, be, dec, hex, oct, bin, uint };

	struct rule {

        using recursive_type = predicates;
        using terminal_type = boost::regex;

		static rule_type default_type;

		rule_type type;

		terminal_type terminal;
		recursive_type recursive;

		rule_transform transform = rule_transform::none;

		rule();

		rule(rule_type);

		rule(const std::string&);

		rule(const terminal_type&);
		rule(const recursive_type&);

		rule& operator<<(const rule_type);

		rule& operator<<(const std::string&);

		rule& operator<<(const predicate_modifier);
		rule& operator<<(const predicate_quantifier&);

		rule& operator<<(const terminal_type&);
		rule& operator<<(const predicate&);

		void retype(rule_type);

		std::string str() const;

		static rules singletons(const std::list<std::string>&);
	};

	//
	// grammar
	//

	using grammar = std::map<std::string,rules>;

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

		size_t node_count() const;
		size_t leaf_count() const;

		std::string str() const;
		std::string xml() const;
		std::string code() const;
	};
}
