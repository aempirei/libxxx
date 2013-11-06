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

        rule_type rule::default_type = rule_type::undefined;

        rule::rule(rule_type t) {
                retype(t);
        }

        rule::rule() : rule(default_type) {
                // nothing
        }

        rule::rule(const terminal_type& x) : rule(rule_type::terminal) {
                terminal_value = x;
        }

        rule::rule(const recursive_type& x) : rule(rule_type::recursive) {
                recursive_value = x;
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

                terminal_value = x;

                return *this;
        }

        rule& rule::operator<<(const predicate& x) {

                if(type != rule_type::recursive)
                        retype(rule_type::recursive);

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

                                terminal_value.assign("\\A" + x, boost::regex::perl);
                                break;

                        case rule_type::recursive:

                                p.name = x;
                                p.quantifier = q::one;

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

        rule& rule::operator<<(const predicate_quantifier& x) {

                if(type != rule_type::recursive)
                        throw std::runtime_error("rule type is not recursive");

                if(recursive_value.empty())
                        throw std::runtime_error("cannot assign quantifier to last predicate--recursive rule is empty");

                recursive_value.back().quantifier = x;

                return *this;
        }

        void rule::retype(rule_type t) {
                type = t;
                recursive_value.clear();
        }

        std::string str() {
                return "<RULE>";
        }

        std::list<rule> rule::singletons(const std::list<std::string>& xs) {
                std::list<rule> y;
                for(auto x : xs)
                        y.push_back(rule(rule_type::recursive) << x);
                return y;
        }

}
