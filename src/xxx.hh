#pragma once

#include <map>
#include <list>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

#include <climits>
#include <cctype>
#include <cstring>

#include <boost/regex.hpp>

namespace xxx {

    struct rule;
    struct predicate;
    struct q;

	using predicate_quantifier = std::pair<size_t,size_t>;

    using rules = std::list<rule>;
    using predicates = std::list<predicate>;

    using var = std::string;

    struct q {

        static const predicate_quantifier star;
        static const predicate_quantifier plus;
        static const predicate_quantifier question;
        static const predicate_quantifier one;

        static predicate_quantifier lower(size_t);
        static predicate_quantifier upper(size_t);
    };

	//
	// predicate
	//

	enum struct predicate_modifier { push, lift, discard, peek_positive, peek_negative };

	struct predicate {

		var name;

		predicate_quantifier quantifier;
		predicate_modifier modifier;

		predicate();
		predicate(const var&);

		std::string str() const;
	};

	//
	// rule
	//

	enum struct rule_type { recursive, literal, regex, builtin };

	struct rule {

        using literal_type = std::string;
        using builtin_type = decltype(&isalpha);
        using regex_type = boost::regex;

        using recursive_type = predicates;

        using hint = std::pair<rule_type, std::string>;
        using hints = std::list<hint>;

		rule_type type;

		recursive_type recursive;

        literal_type literal;
        builtin_type builtin;
        regex_type regex;

		rule();

        rule(rule_type);

        rule(const hint&);

        rule(rule_type, std::string);

		rule(const recursive_type&);

		rule(const literal_type&);
		rule(const builtin_type&);
		rule(const regex_type&);

		rule& operator<<(const var&);

		rule& operator<<(predicate_modifier);
		rule& operator<<(const predicate_quantifier&);
		rule& operator<<(const predicate&);

		std::string str() const;

		static rules singletons(const hints&);
	};

	//
	// grammar
	//

	using grammar = std::map<var,rules>;

	std::string grammar_str(const grammar&);

	//
	// ast
	//

	struct ast {

		var name;

		rule_type type;

        grammar::const_iterator entry;
        rules::const_iterator subentry;

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
	};
}
