#include <xxx.hh>

namespace xxx {

	std::string rule::str() const {

        std::stringstream ss;

        switch(type) {

            case rule_type::recursive:

                if(not recursive.empty()) {

                    auto iter = recursive.begin();

                    ss << iter->str();

                    while(++iter != recursive.end())
                        ss << ' ' << iter->str();
                }

                break;

            case rule_type::regex:

                ss << '/' << regex.str().substr(2,std::string::npos) << '/';
                break;
        }

        return ss.str();
	}

    static std::string to_cstring(const std::string& s) {
        std::stringstream ss;
        ss << '"';
        for(int c : s) {
            /**/ if(c == '"' or c == '\\') ss << '\\' << (char)c;
            else if(isprint(c)           ) ss << (char)c;
            else                           ss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        }
        ss << '"';
        return ss.str();
    }

    std::string rule::to_cc() const {

        std::stringstream ss;

        if(type == rule_type::regex) {
            ss << "rule::regex_type(" << to_cstring(regex.str()) << ')';
        } else {

            ss << "rule()";

            for(const auto& p : recursive) {
                ss << " << \"" << p.name << '\"';

                if(p.quantifier != q::one) {
                    ss << (
                            (p.quantifier == q::star    ) ? " << q::star"     :
                            (p.quantifier == q::plus    ) ? " << q::plus"     :
                            (p.quantifier == q::question) ? " << q::question" : "");
                }

                if(p.modifier != predicate_modifier::push) {
                    ss << (
                            (p.modifier == predicate_modifier::lift         ) ? " << M::lift"          :
                            (p.modifier == predicate_modifier::discard      ) ? " << M::discard"       :
                            (p.modifier == predicate_modifier::peek_positive) ? " << M::peek_positive" :
                            (p.modifier == predicate_modifier::peek_negative) ? " << M::peek_negative" : "");
                }
            }
        }

        return ss.str();
    }

	rules rule::singletons(const hints& xs) {

        rules y;

		for(const auto& x : xs)

			y.push_back(rule(x));

		return y;
	}


    //
    // rule::rule
    //

	rule::rule() : rule(rule_type::recursive) {
	}

	rule::rule(rule_type my_type) : type(my_type) {
    }

    rule::rule(rule_type my_type, const std::string s) : rule(my_type) {

        switch(type) {

            case rule_type::regex:

                regex.assign("\\A" + s, boost::regex::perl);
                break;

            case rule_type::recursive:

                recursive = { predicate(s) };
                break;
        }
    }

    rule::rule(const hint& h) : rule(h.first, h.second) {
    }

	rule::rule(const     regex_type& x) : rule(rule_type::regex    ) { regex     = x; }
	rule::rule(const recursive_type& x) : rule(rule_type::recursive) { recursive = x; }

    //
    // rule::operator<<
    //

    rule& rule::operator<<(const var& x) {
        return operator<<(predicate(x));
    }

    rule& rule::operator<<(const predicate& x) {

        if(type != rule_type::recursive)
            throw std::runtime_error("rule::type is not rule_type::recursive in rule::operator<<");

        recursive.push_back(predicate(x));

        return *this;
    }

    rule& rule::operator<<(predicate_modifier x) {

        if(type != rule_type::recursive)
            throw std::runtime_error("rule::type is not rule_type::recursive in rule::operator<<");

        if(recursive.empty())
            throw std::runtime_error("cannot assign quantifier to last predicate--recursive rule is empty");

		recursive.back().modifier = x;

		return *this;
	}

	rule& rule::operator<<(const predicate_quantifier& x) {

		if(type != rule_type::recursive)
			throw std::runtime_error("rule::type is not rule_type::recursive in rule::operator<<");

		if(recursive.empty())
			throw std::runtime_error("cannot assign quantifier to last predicate--recursive rule is empty");

		recursive.back().quantifier = x;

		return *this;
	}

}
