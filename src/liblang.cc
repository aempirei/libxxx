#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>
#include <cassert>

#include <unistd.h>

#include <stdexcept>

#include <liblang.hh>

namespace lang {

	const quantifier q::star      = quantifier(0,INT_MAX);
	const quantifier q::plus      = quantifier(1,INT_MAX);

	const quantifier q::zero      = quantifier(0,0);
	const quantifier q::question  = quantifier(0,1);
	const quantifier q::one       = quantifier(1,1);

	rule::rule(rule_type t) {
		reset_type(t);
	}

	rule::rule() : rule(rule_type::undefined) {
		// nothing
	}

	rule::rule(const terminal_type& x) : rule(rule_type::terminal) {
		terminal_value = x;
		// nothing
	}

	rule::rule(const recursive_type& x) : rule(rule_type::recursive) {
		recursive_value = x;
		// nothing
	}

	rule::rule(const rule& x) {
		type = x.type;
		terminal_value = x.terminal_value;
		recursive_value = x.recursive_value;
	}
	
	void rule::reset_type(rule_type t) {
		type = t;
		recursive_value.clear();
		// terminal_value.assign("");
	}

	rule& rule::operator<<(rule_type t) {

		if(type != t)
			reset_type(t);

		return *this;
	}

	rule& rule::operator<<(const terminal_type& x) {

		if(type != rule_type::terminal)
			reset_type(rule_type::terminal);

		terminal_value = x;

		return *this;
	}

	rule& rule::operator<<(const recursive_type::value_type& x) {

		if(type != rule_type::recursive)
			reset_type(rule_type::recursive);

		recursive_value.push_back(x);

		return *this;
	}

	rule& rule::operator<<(const symbol& x) {

				std::string r = x;
		switch(type) {
			case rule_type::undefined:
				throw std::runtime_error("rule type is undefined");
				break;
			case rule_type::terminal:

				terminal_value = regex(r.c_str());
				break;
			case rule_type::recursive:
				recursive_value.push_back(recursive_type::value_type(x, q::one));
				break;
			default:
				throw std::runtime_error("rule type is unknown");
				break;
		}

		return *this;
	}

	rule& rule::operator<<(const quantifier& x) {

		if(type != rule_type::recursive)
			throw std::runtime_error("rule type is not recursive");

		if(recursive_value.empty())
			throw std::runtime_error("cannot assign quantifier to last predicate--recursive rule is empty");

		recursive_value.back().second = x;

		return *this;
	}

	rule rule::singleton(const symbol& x) {
		return rule() << rule_type::recursive << x;
	}

	std::list<rule> rule::singletons(const std::list<symbol>& xs) {
		std::list<rule> y;
		for(auto x : xs)
			y.push_back(rule::singleton(x));
		return y;
	}
}
