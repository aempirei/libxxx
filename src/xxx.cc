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

        static std::pair<ssize_t,ssize_t> parse_recursive(const grammar&, std::string, const std::string&, ast&, ssize_t offset = 0);

        //
        // q
        //

	const quantifier q::star      = quantifier(0,INT_MAX);
	const quantifier q::plus      = quantifier(1,INT_MAX);

	const quantifier q::zero      = quantifier(0,0);
	const quantifier q::question  = quantifier(0,1);
	const quantifier q::one       = quantifier(1,1);

        //
        // rule
        //

        rule_type rule::default_type = rule_type::undefined;

	rule::rule(rule_type t) {
		reset_type(t);
	}

	rule::rule() : rule(default_type) {
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

        rule::rule(const std::string& x) : rule() {
                operator<<(x);
        }
	
	void rule::reset_type(rule_type t) {
		type = t;
		recursive_value.clear();
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

	rule& rule::operator<<(const std::string& x) {

                predicate p;

		switch(type) {

			case rule_type::undefined:

				throw std::runtime_error("rule type is undefined");
				break;

			case rule_type::terminal:

				terminal_value.assign("\\A" + x);
				break;

			case rule_type::recursive:

                                p.first = x;
                                p.second = q::one;

				recursive_value.push_back(p);
				break;

			default:

				throw std::runtime_error("rule type is unknown");
				break;
		}

		return *this;
	}

        rule& rule::operator<<(predicate_modifier m) {

		if(type != rule_type::recursive)
			throw std::runtime_error("rule type is not recursive");

		if(recursive_value.empty())
			throw std::runtime_error("cannot assign quantifier to last predicate--recursive rule is empty");

		recursive_value.back().modifier = m;

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

        rule rule::recursive(const std::string& x) {
                return rule(rule_type::recursive) << x;
        }

        rule rule::terminal(const std::string& x) {
                return rule(rule_type::terminal) << x;
        }

        std::list<rule> rule::singletons(const std::list<std::string>& xs) {
                std::list<rule> y;
                for(auto x : xs)
                        y.push_back(rule::recursive(x));
                return y;
        }

        //
        // parse
        //

        ast& parse(const grammar& g, FILE *fp, ast& q) {

                std::string s;
                char buf[1024];

                while(!feof(fp)) {
                        int n = fread(buf, 1, sizeof(buf), fp);
                        assert(n != -1);
                        s.append(buf, n);
                }

                return parse(g, s, q);
        }

        ast& parse(const grammar& g, std::string s, ast& q) {

                std::pair<ssize_t,ssize_t> ab = parse_recursive(g, "document", s, q);

                if(ab.first == -1) {
                        std::cerr << "unmatched portion of document beginning at offset " << ab.second << std::endl;
                        std::cerr << "\33[1m" << s.substr(ab.second, std::string::npos) << "\33[0m";
                }
                return q;
        }

        std::pair<ssize_t,ssize_t> parse_recursive(const grammar& g, std::string rulename, const std::string& s, ast& q, ssize_t offset) {

                const auto iter = g.find(rulename);

                if(iter == g.end()) {
                        std::stringstream ss;
                        ss << "rule not found -- " << rulename;
                        throw std::runtime_error(ss.str());
                }

                const auto& rules = iter->second;

                ssize_t current = offset;

                std::string ms;

                q.offset = offset;
                q.rulename = rulename;

                for(const auto& rule : rules) {

                        bool success = true;

                        match matches;

                        current = offset;

                        q.children.clear();
                        q.type = rule.type;

                        switch(rule.type) {

                                case rule_type::undefined:

                                        throw std::runtime_error("rule type is undefined");
                                        break;

                                case rule_type::terminal:

                                        ms = s.substr(offset, std::string::npos);

                                        if(!boost::regex_search(ms, matches, rule.terminal_value)) {
                                                success = false;
                                                break;
                                        }

                                        q.string = matches[0].str();
                                        q.terminal_matches = matches;
                                        current += q.string.length();

                                        break;

                                case rule_type::recursive:

                                        for(const auto& predicate : rule.recursive_value) {

                                                int i;

                                                for(i = 0; i < predicate.second.second; i++) {

                                                        ast qq;

                                                        auto next = parse_recursive(g, predicate.first, s, qq, current);

                                                        if(next.first == -1)
                                                                break;

                                                        current = next.second;

                                                        if(predicate.modifier == predicate_modifier::push) {

                                                                q.children.push_back(qq);

                                                        } else if(predicate.modifier == predicate_modifier::lift) {

                                                                if(qq.type != rule_type::recursive)
                                                                        throw new std::runtime_error("attempting to lift non-recursive ast node");

                                                                for(const auto& qqq : qq.children)
                                                                        q.children.push_back(qqq);

                                                        } else if(predicate.modifier == predicate_modifier::discard) {
                                                                // discard
                                                        }
                                                }

                                                if(i < predicate.second.first) {
                                                        success = false;
                                                        break;
                                                }
                                        }

                                        break;

                                default:

                                        throw std::runtime_error("rule type is unknown");
                                        break;
                        }

                        if(success)
                                return std::pair<ssize_t,ssize_t>(offset,current);
                }

                return std::pair<ssize_t,ssize_t>(-1,current);
        }
}
