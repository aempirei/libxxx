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

#include <liblang.hh>

namespace lang {

	const quantifier q::star      = quantifier(0,INT_MAX);
	const quantifier q::plus      = quantifier(1,INT_MAX);
	const quantifier q::question  = quantifier(0,1);
	const quantifier q::singleton = quantifier(1,1);

	/*
	   struct rule {

	   enum class rule_type { terminal, recursive };

	   typedef std::list<predicate> recursive_type;
	   typedef std::regex terminal_type;

	   rule_type type;

	   terminal_type terminal_value;
	   recursive_type recursive_value;
	   };
	 */

	rule::rule() : type(rule::rule_type::recursive) {
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

	/*
	rule& rule::operator<<(const terminal_type&);
	rule& rule::operator<<(const recursive_type::value_type&);
	rule& rule::operator<<(const recursive_type::value_type::first_type&);
	*/
}
