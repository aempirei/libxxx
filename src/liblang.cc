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
	const quantifier q::question  = quantifier(0,1);
	const quantifier q::singleton = quantifier(1,1);

	rule::rule() : type(rule::rule_type::undefined) {
		// nothing
	}

	rule::rule(const terminal_type& x) : type(rule::rule_type::terminal), terminal_value(x) {
		// nothing
	}

	rule::rule(const recursive_type& x) : type(rule::rule_type::recursive), recursive_value(x) {
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
		terminal_value.assign("");
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

		switch(type) {
			case rule_type::undefined:
				throw std::runtime_error("rule type is undefined");
				break;
			case rule_type::terminal:
				terminal_value.assign(x);
				break;
			case rule_type::recursive:
				recursive_value.push_back(recursive_type::value_type(x, q::singleton));
				break;
			default:
				throw std::runtime_error("rule type is unknown");
				break;
		}

		return *this;
	}
}
