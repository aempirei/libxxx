#include <xxx.hh>

namespace xxx {

	rule_type rule::default_type = rule_type::recursive;

    void rule::retype(rule_type my_type) {
        operator=(rule(my_type));
    }

	std::string rule::str() const {

		if(type == rule_type::recursive) {

                std::stringstream ss;

                if(not recursive.empty()) {

                    auto iter = recursive.begin();

                    ss << iter->str();

                    while(++iter != recursive.end())
                        ss << ' ' << iter->str();
                }

                return ss.str();

		} else if(type == rule_type::terminal) {

			std::string s = terminal.str().substr(2,std::string::npos);

			return '/' + s + '/';

        } else {

            throw new std::runtime_error("unexpected type found in rule::str()");

        }
	}

	rules rule::singletons(const std::list<std::string>& xs) {

        rules y;

		for(auto x : xs)

			y.push_back(rule(rule_type::recursive) << x);

		return y;
	}


    //
    // rule::rule
    //

	rule::rule() : rule(default_type) {
	}

	rule::rule(rule_type my_type) : type(my_type) {
	}

	rule::rule(const std::string& x) : rule() {
		operator<<(x);
	}

	rule::rule(const terminal_type& x) : rule(rule_type::terminal) {
		terminal = x;
	}

	rule::rule(const recursive_type& x) : rule(rule_type::recursive) {
		recursive = x;
	}

    //
    // rule::operator<<
    //

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

            operator<<(predicate(x));

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

}
