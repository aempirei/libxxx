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

    template <typename E, typename, E> struct enum_wrapper;

    template <typename> struct _s;

    struct predicate;
    struct grammar;
    struct rule;
    struct tree;

    using rules = _s<rule>;
    using predicates = _s<predicate>;

    using var = std::string;

    using transform_function = void(tree *, void *);

    plural(tree);   // trees
    plural(var);    // vars

    transform_function passthru_transform;

    //
    // _s<T>
    //

    template <typename T> struct _s : std::vector<T> {
        using value_type = T;
        using base_type = std::vector<value_type>;
        using base_type::base_type;
        _s();
        _s(const value_type&);
        _s(const value_type&, const _s&);
    };

    extern template struct _s<rule>;
    extern template struct _s<predicate>;

    //
    // enum_wrapper
    //

    template <typename E, typename N, E X> struct enum_wrapper {

        using enum_type = E;
        using base_type = N;

        enum_type x;

        constexpr static enum_type default_value = X;

        constexpr enum_wrapper(              ) : x((default_value)) { }
        constexpr enum_wrapper(enum_type my_x) : x((enum_type)my_x) { }
        constexpr enum_wrapper(base_type my_x) : x((enum_type)my_x) { }

        constexpr operator enum_type() const { return (enum_type)x; }
        constexpr operator base_type() const { return (base_type)x; }

        inline std::basic_string<base_type> str() const {
            return (x == default_value) ? "" : std::basic_string<base_type>(1, (base_type)x);
        }
    };

    //
    // predicate
    //

    enum struct predicate_modifier_enum : char {
        push = '=',
        peek = '>',
        drop = '!'
    };

    enum struct predicate_quantifier_enum : char {
        one   = '\0',
        maybe = '?'
    };

    using predicate_modifier   = enum_wrapper< predicate_modifier_enum  , char, predicate_modifier_enum::push  >;
    using predicate_quantifier = enum_wrapper< predicate_quantifier_enum, char, predicate_quantifier_enum::one >;

    using predicate_name = var;

    struct predicate {

        using M = predicate_modifier::enum_type;
        using Q = predicate_quantifier::enum_type;

        predicate_modifier      modifier;
        predicate_name          name;
        predicate_quantifier    quantifier;

        predicate();
        predicate(const predicate_name&);
        predicate(const predicate_modifier&, const predicate_name&, const predicate_quantifier&);

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

    struct rule {

        using terminal_type = boost::regex;
        using composite_type = predicates;

        rule_type type;

        composite_type composite;
        terminal_type terminal;

        transform_function *transform;

        var repl;

        rule();
        rule(rule_type);
        rule(const composite_type&);
        rule(const composite_type&, const var&);
        rule(const terminal_type&);
        rule(const terminal_type&, const var&);

        rule& operator<<(const var&);
        rule& operator<<(const predicate_modifier&);
        rule& operator<<(const predicate_quantifier&);
        rule& operator<<(const predicate&);

        rule& operator>>(transform_function *);
        rule& operator>>(const var&);

        std::string str() const;

        std::string to_cc() const;

        vars to_sig() const;
    };

    //
    // grammar
    //

    using _grammar = std::map<var,rules>;

    struct grammar : _grammar {

            using _grammar::_grammar;

            grammar(const value_type&, grammar&&);
            grammar(const value_type&, const grammar&);
            grammar(const value_type&);
            grammar();

            iterator concat(const value_type&);

            std::string to_xxx() const;
            std::string to_js() const;

            std::string to_cc(bool) const;

            std::set<var> appendix() const;

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
