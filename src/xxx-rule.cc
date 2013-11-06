#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>
#include <cassert>

#include <unistd.h>

#include <stdexcept>

#include <xxx.hh>

namespace xxx {

        rule_type rule::default_type = rule_type::recursive;

        rule::rule(rule_type t) {
                retype(t);
        }

        rule::rule() : rule(default_type) {
                // nothing
        }

        rule::rule(const terminal_type& x) : rule(rule_type::terminal) {
                terminal = x;
        }

        rule::rule(const recursive_type& x) : rule(rule_type::recursive) {
                recursive = x;
        }

        rule::rule(const rule& x) {
                *this = x;
        }

        rule::rule(const std::string& x) : rule() {
                operator<<(x);
        }

        rule& rule::operator<<(rule_type t) {

                if(type != t)
                        retype(t);

                return *this;
        }

        rule& rule::operator<<(const terminal_type& x) {

                if(type != rule_type::terminal)
                        retype(rule_type::terminal);

                terminal = x;

                return *this;
        }

        rule& rule::operator<<(const predicate& x) {

                if(type != rule_type::recursive)
                        retype(rule_type::recursive);

                recursive.push_back(x);

                return *this;
        }

        rule& rule::operator<<(const std::string& x) {

		if(type == rule_type::terminal) {

			terminal.assign("\\A" + x, boost::regex::perl);

		} else if(type == rule_type::recursive) {

			recursive.push_back(predicate(x));

		} else {

			throw std::runtime_error("unknown rule type found in rule::operator<<");
		}

		return *this;
	}

	rule& rule::operator<<(predicate_modifier m) {

		if(type != rule_type::recursive)
			throw std::runtime_error("rule type is not recursive");

		if(recursive.empty())
			throw std::runtime_error("cannot assign quantifier to last predicate--recursive rule is empty");

		recursive.back().modifier = m;

		return *this;
	}

	rule& rule::operator<<(const predicate_quantifier& x) {

		if(type != rule_type::recursive)
			throw std::runtime_error("rule type is not recursive");

		if(recursive.empty())
			throw std::runtime_error("cannot assign quantifier to last predicate--recursive rule is empty");

		recursive.back().quantifier = x;

		return *this;
	}

	void rule::retype(rule_type t) {
		type = t;
		recursive.clear();
	}

	std::string rule::str() const {

		if(type == rule_type::recursive) {

			std::string s;

			for(auto iter = recursive.begin(); iter != recursive.end(); iter++) {
				s += iter->str();
				if(next(iter) != recursive.end())
					s += ' ';
			}

			return s;

		} else if(type == rule_type::terminal) {

			return '/' + terminal.str() + '/';

		}

		throw new std::runtime_error("uknown rule type found during rule to string conversion");
        }

        std::list<rule> rule::singletons(const std::list<std::string>& xs) {
                std::list<rule> y;
                for(auto x : xs)
                        y.push_back(rule(rule_type::recursive) << x);
                return y;
        }

}
