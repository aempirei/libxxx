#pragma once

#include <map>
#include <set>
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

    struct predicate;
    struct rule;
    struct ast;
    struct Q;

    class grammar;

	using predicate_quantifier = std::pair<size_t,size_t>;

    using rules = std::vector<rule>;
    using predicates = std::list<predicate>;

    using var = std::string;
    using vars = std::list<var>;

    using transform_function = void(ast *, void *);
    
    inline void empty_transform(ast *, void *) { }

    struct Q {

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

	enum struct predicate_modifier : char {
        push          = '=',
        discard       = '!',
        peek_positive = '>',
        peek_negative = '~'
    };

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

	enum struct rule_type : char {
        recursive = ':',
        regex     = '~'
    };

	struct rule {

        using regex_type = boost::regex;
        using recursive_type = predicates;

		rule_type type;

		recursive_type recursive;
        regex_type regex;

        transform_function *transform;

		rule();
        rule(rule_type);
		rule(const recursive_type&);
		rule(const regex_type&);

		rule& operator<<(const var&);
		rule& operator<<(predicate_modifier);
		rule& operator<<(const predicate_quantifier&);
		rule& operator<<(const predicate&);
        rule& operator>>(transform_function *);

		std::string str() const;

        std::string to_cc() const;

        vars to_sig() const;
	};

	//
	// grammar
	//

	using _grammar = std::map<var,rules>;
    class grammar : public _grammar {

        public:

            using _grammar::_grammar;

            enum struct string_format_type {
                xxx, cc, js
            };

            std::string to_s(string_format_type) const;

            std::set<var> appendix() const;

        private:

            std::string to_s_xxx() const;
            std::string to_s_cc() const;
            std::string to_s_js() const;
    };

	//
	// ast
	//

	struct ast {

            grammar::const_iterator match_entry;
            rules::const_iterator match_rule;

            std::string match;

            std::vector<ast> children;

            ssize_t offset;

            ast();
            ast(const grammar&, FILE *);
            ast(const grammar&, const std::string&);

            void parse(const grammar&, FILE *);
            void parse(const grammar&, const std::string&);

            std::pair<ssize_t,ssize_t> parse_recursive(const grammar&, const var&, const std::string&, ssize_t);

            void *transform();

            void transform(void *);

            const var& name() const;

            size_t node_count() const;
            size_t leaf_count() const;

            std::string str() const;
            std::string xml() const;
	};
}
