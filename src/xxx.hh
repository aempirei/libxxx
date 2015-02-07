#pragma once

#include <map>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

#include <climits>
#include <cctype>
#include <cstring>

#include <boost/regex.hpp>

#define plural(x) using x##s = std::vector<x>

namespace xxx {

    struct predicate_quantifier;
    struct predicate;
    struct grammar;
    struct rule;
    struct tree;

    using var = std::string;

    plural(predicate);  // predicates
    plural(rule);       // rules
    plural(tree);       // trees
    plural(var);        // vars

    using transform_function = void(tree *, void *);

    transform_function passthru_transform;

    //
    // predicate
    //

    enum class predicate_modifier : char { push = 0, peek = '>', discard = '!' };

    using predicate_name = var;

    using _predicate_quantifier = std::pair<size_t,size_t>;
    struct predicate_quantifier : _predicate_quantifier {

        static const predicate_quantifier star;
        static const predicate_quantifier plus;
        static const predicate_quantifier question;
        static const predicate_quantifier one;

        using _predicate_quantifier::_predicate_quantifier;

        predicate_quantifier(const std::string&);

        constexpr predicate_quantifier() : predicate_quantifier(1,1) {
        }

        constexpr predicate_quantifier(char ch)
            : predicate_quantifier(ch == '*' ? star : ch == '+' ? plus : ch == '?' ? question : one)
            {
            }

        constexpr bool operator!=(predicate_quantifier x) const {
            return first != x.first or second != x.second;
        }

        constexpr bool operator==(predicate_quantifier x) const {
            return first == x.first and second == x.second;
        }

        constexpr explicit operator char() const {
            return (first == 0 and second == SIZE_MAX) ? '*' : (first == 1 and second == SIZE_MAX) ? '+' : (first == 0 and second == 1) ? '?' : '\0';
        }

        std::string str() const;
    };

    struct predicate {

        predicate_modifier      modifier;
        predicate_name          name;
        predicate_quantifier    quantifier;

        predicate();
        predicate(const predicate_name&);
        predicate(predicate_modifier, const predicate_name&, const predicate_quantifier&);

        size_t upper() const;
        size_t lower() const;

        std::string str() const;

        std::string to_cc_decl(size_t) const;
        std::string to_cc_def(size_t) const;
    };

    //
    // rule
    //

    enum struct rule_type : char { composite = ':', terminal  = '~' };

    enum struct rule_kind { product, functor };

    struct rule {

        using terminal_type = boost::regex;
        using composite_type = predicates;

        rule_type type;

        composite_type composite;
        terminal_type terminal;

        transform_function *transform;

        rule();
        rule(rule_type);
        rule(const composite_type&);
        rule(const terminal_type&);

        rule& operator<<(const var&);
        rule& operator<<(predicate_modifier);
        rule& operator<<(const predicate_quantifier&);
        rule& operator<<(const predicate&);

        rule& operator>>(transform_function *);

        std::string str() const;

        std::string to_cc() const;

        vars to_sig() const;

        bool is_product() const;
    };

    //
    // grammar
    //

    using _grammar = std::map<var,rules>;

    struct grammar : _grammar {

            using _grammar::_grammar;

            std::set<var> appendix() const;

            iterator concat(const value_type&);

            std::string to_xxx() const;
            std::string to_cc() const;
            std::string to_js() const;

            std::string to_cc_transform(const key_type&, const rule&) const;
            std::string to_cc_transforms(const key_type&) const;
    };

    using entry = grammar::value_type;

    //
    // tree
    //

    struct tree {

        grammar::const_iterator match_def;
        rules::const_iterator match_rule;

        std::string match;

        trees children;

        ssize_t offset;

        tree();
        tree(const grammar&, FILE *);
        tree(const grammar&, const std::string&);

        void parse(const grammar&, FILE *);
        void parse(const grammar&, const std::string&);

        std::pair<ssize_t,ssize_t> parse_recursive(const grammar&, const var&, const std::string&, ssize_t);

        void transform(void *);

        const var& match_name() const;
        rule_type match_type() const;

        size_t node_count() const;
        size_t leaf_count() const;

        std::string str() const;
        std::string xml() const;
    };
}

#undef plural
