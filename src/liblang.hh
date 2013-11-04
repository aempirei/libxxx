#pragma once

#include <set>
#include <map>
#include <string>
#include <boost/regex.hpp>
#include <list>
#include <climits>

namespace lang {

	using regex = boost::regex;
        using match = boost::smatch;

	using quantifier = std::pair<int,int>;

	using predicate = std::pair<std::string,quantifier>;

	enum class rule_type { undefined, terminal, recursive, literal, error };

	struct rule {

		typedef std::list<predicate>    recursive_type;
		typedef regex                   terminal_type;
                typedef std::string             literal_type;

                static rule_type default_type;

		rule_type type;

		terminal_type   terminal_value;
		recursive_type  recursive_value;
                literal_type    literal_value;

		rule();
		rule(rule_type);
		rule(const terminal_type&);
		rule(const recursive_type&);
		rule(const rule&);
                rule(const std::string&);

		void reset_type(rule_type);

		rule& operator<<(rule_type);
		rule& operator<<(const terminal_type&);
		rule& operator<<(const recursive_type::value_type&);
		rule& operator<<(const std::string&);
		rule& operator<<(const quantifier&);

		static rule recursive(const std::string&);
                static rule terminal(const std::string&);

		static std::list<rule> singletons(const std::list<std::string>&);
	};

	using grammar = std::map<std::string,std::list<rule>>;

        struct ast {
                std::string rulename;
                rule_type type = rule_type::undefined;
                match terminal_matches;
                std::string string;
                std::list<ast> children;
                ssize_t offset = -1;
        };

	struct q {
		static const quantifier star;
		static const quantifier plus;
		static const quantifier zero;
		static const quantifier question;
		static const quantifier one;
	};
}
