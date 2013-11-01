#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>
#include <cassert>

#include <unistd.h>

#include <sstream>
#include <iostream>
#include <iomanip>

#include <list>
#include <stack>
#include <thread>
#include <mutex>

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

	rule::rule() : rule(rule::undefined) {
		// nothing
	}

	rule::rule(const terminal_type& x) : rule(rule::terminal) {
		terminal_value = x;
		// nothing
	}

	rule::rule(const recursive_type& x) : rule(rule::recursive) {
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

		if(type != rule::terminal)
			reset_type(rule::terminal);

		terminal_value = x;

		return *this;
	}

	rule& rule::operator<<(const recursive_type::value_type& x) {

		if(type != rule::recursive)
			reset_type(rule::recursive);

		recursive_value.push_back(x);

		return *this;
	}

	rule& rule::operator<<(const symbol& x) {

		switch(type) {
			case rule::undefined:
				throw std::runtime_error("rule type is undefined");
				break;
			case rule::terminal:
				terminal_value.assign(x.c_str());//, regex::extended);
				break;
			case rule::recursive:
				recursive_value.push_back(recursive_type::value_type(x, q::one));
				break;
			default:
				throw std::runtime_error("rule type is unknown");
				break;
		}

		return *this;
	}

	rule& rule::operator<<(const quantifier& x) {

		if(type != rule::recursive)
			throw std::runtime_error("rule type is not recursive");

		if(recursive_value.empty())
			throw std::runtime_error("cannot assign quantifier to last predicate--recursive rule is empty");

		recursive_value.back().second = x;

		return *this;
	}
}
